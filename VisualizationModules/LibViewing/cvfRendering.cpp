//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#include "cvfBase.h"
#include "cvfRendering.h"
#include "cvfScene.h"
#include "cvfModel.h"
#include "cvfCamera.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfRenderQueue.h"
#include "cvfRenderQueueBuilder.h"
#include "cvfRenderQueueSorter.h"
#include "cvfEffect.h"
#include "cvfTimer.h"
#include "cvfOpenGL.h"
#include "cvfFramebufferObject.h"
#include "cvfOverlayItem.h"
#include "cvfDynamicUniformSet.h"
#include "cvfUniformSet.h"
#include "cvfRay.h"
#include "cvfTrace.h"
#include "cvfShaderProgram.h"
#include "cvfRayIntersectSpec.h"
#include "cvfHitItemCollection.h"

namespace cvf {

// Internal
struct OverlayItemLayout
{
    ref<OverlayItem>                overlayItem;
    OverlayItem::LayoutCorner       corner;
    OverlayItem::LayoutDirection    direction;
};


//==================================================================================================
///
/// \class cvf::Rendering
/// \ingroup Viewing
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
Rendering::Rendering(const String& renderingName)
:   m_renderingName(renderingName),
    m_enableMask(0xffffffff),
    m_clearMode(Viewport::CLEAR_COLOR_DEPTH),    
    m_maxNumPartsToDraw(std::numeric_limits<size_t>::max()),
    m_enablePerformanceTiming(true)
{
    m_camera = new Camera;
    m_visibleParts = new PartRenderHintCollection;

    // Initialize with minimal sorting (just priorities)
    m_renderQueueSorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::MINIMAL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rendering::~Rendering()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::setRenderingName(const String& renderingName)
{
    m_renderingName = renderingName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Rendering::renderingName() const
{
    return m_renderingName;
}


//--------------------------------------------------------------------------------------------------
/// Set the scene to be rendered
///
/// The specified scene can be shared among many renderings.
//--------------------------------------------------------------------------------------------------
void Rendering::setScene(Scene* scene)
{
    m_scene = scene;

    m_visibleParts->removeAll();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Scene* Rendering::scene()
{
    return m_scene.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Scene* Rendering::scene() const
{
    return m_scene.p();
}


//--------------------------------------------------------------------------------------------------
/// Render the configured scene using the current Camera
//--------------------------------------------------------------------------------------------------
void Rendering::render(OpenGLContext* oglContext)
{
    CVF_ASSERT(m_camera.notNull() && m_camera->viewport());

    bool debugLogging = CVF_SHOULD_LOG_RENDER_DEBUG(oglContext);
    if (debugLogging)
    {
        CVF_LOG_RENDER_DEBUG(oglContext, String("Entering Rendering::render(), renderingName='%1'").arg(m_renderingName));
    }

    m_performanceInfo.clear();

    ref<Timer> renderTimer;
    if (m_enablePerformanceTiming)
    {
        renderTimer = new Timer;
    }

    // Compute visible parts
    // -------------------------------------------------------------------------
    m_visibleParts->setCountZero();
    if (m_scene.notNull())
    {
        m_scene->findVisibleParts(m_visibleParts.p(), *m_camera.p(), m_cullSettings, m_enableMask);
    }

    if (renderTimer.notNull())
    {
        m_performanceInfo.computeVisiblePartsTime = renderTimer->lapTime();
    }

    m_performanceInfo.visiblePartsCount = m_visibleParts->count();

    // Setup FBO
    // -------------------------------------------------------------------------
    if (m_targetFrameBuffer.notNull())
    {
        // Option to sync FBO size to viewport size
        m_targetFrameBuffer->applyOpenGL(oglContext);

        String failReason;
        if (!m_targetFrameBuffer->isFramebufferComplete(oglContext, &failReason))
        {
            Trace::show(failReason);
            return;
        }
    }
    else
    {
#ifndef CVF_OPENGL_ES
        if (FramebufferObject::supportedOpenGL(oglContext))
        {
            FramebufferObject::useDefaultWindowFramebuffer(oglContext);
        }
#endif
    }

    // Setup camera and view
    // -------------------------------------------------------------------------
    m_camera->viewport()->applyOpenGL(oglContext, m_clearMode);
    m_camera->applyOpenGL();

    // Update dynamic uniforms and dynamic uniform sets
    // -------------------------------------------------------------------------
    updateDynamicUniformSets();
    updateAndCombineGlobalDynamicUniformSets();


    // Build the render queue
    // -------------------------------------------------------------------------
    RenderQueueBuilder builder(m_visibleParts.p(), oglContext, m_camera.p());
    builder.setFixedEffect(m_effectOverride.p());
    if (m_renderQueueSorter.notNull())
    {
        builder.setRequireDistance(m_renderQueueSorter->requireDistance());
        builder.setRequirePixelArea(m_renderQueueSorter->requirePixelArea());
    }

    RenderQueue renderQueue;
    builder.populateRenderQueue(&renderQueue);

    if (renderTimer.notNull())
    {
        m_performanceInfo.buildRenderQueueTime = renderTimer->lapTime();
    }


    // Sort the render queue
    // -------------------------------------------------------------------------
    if (m_renderQueueSorter.notNull())
    {
        m_renderQueueSorter->sort(&renderQueue);

        if (renderTimer.notNull())
        {
            m_performanceInfo.sortRenderQueueTime = renderTimer->lapTime();
        }
    }


    // Render the scene
    // -------------------------------------------------------------------------
    const UniformSet* globalUniformSet = m_combinedGlobalUniformSet.p();
    size_t numPartsToDraw = std::min(m_maxNumPartsToDraw, renderQueue.count());
    m_renderEngine.render(oglContext, &renderQueue, numPartsToDraw, *m_camera, globalUniformSet);

    if (renderTimer.notNull())
    {
        m_performanceInfo.renderEngineTime = renderTimer->lapTime();
    }

    // Render the overlay items
    // Use software rendering if forced or if no support for Shader Programs
    bool useSoftwareRendering = m_renderEngine.isForcedImmediateModeEnabled() || !ShaderProgram::supportedOpenGL(oglContext);
    renderOverlayItems(oglContext, useSoftwareRendering);

    // Update counters
    m_performanceInfo.renderedPartsCount        = m_renderEngine.renderedPartCount();
    m_performanceInfo.vertexCount               = m_renderEngine.renderedVertexCount();
    m_performanceInfo.triangleCount             = m_renderEngine.renderedTriangleCount();
    m_performanceInfo.openGLPrimitiveCount      = m_renderEngine.renderedOpenGLPrimitiveCount();
    m_performanceInfo.applyRenderStateCount     = m_renderEngine.applyRenderStateCount();
    m_performanceInfo.shaderProgramChangesCount = m_renderEngine.shaderProgramChangesCount();

    // Last update before we exit, total render time for this rendering
    if (renderTimer.notNull())
    {
        m_performanceInfo.totalDrawTime = renderTimer->time();
    }

    if (debugLogging)
    {
        CVF_LOG_RENDER_DEBUG(oglContext, String("Exiting Rendering::render(), renderingName='%1'").arg(m_renderingName));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::renderOverlayItems(OpenGLContext* oglContext, bool useSoftwareRendering)
{
    OverlayItemRectMap itemRectMap;
    calculateOverlayItemLayout(&itemRectMap);
    
    OverlayItemRectMap::iterator it;
    for (it = itemRectMap.begin(); it != itemRectMap.end(); ++it)
    {
        OverlayItem* item = it->first;
        Rectui rect = it->second;

        if (useSoftwareRendering)
        {
            item->renderSoftware(oglContext, rect.min(), Vec2ui(rect.width(), rect.height()));
        }
        else
        {
            item->render(oglContext, rect.min(), Vec2ui(rect.width(), rect.height()));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::calculateOverlayItemLayout(OverlayItemRectMap* itemRectMap)
{
    calculateOverlayItemLayout(itemRectMap, OverlayItem::TOP_LEFT,     OverlayItem::HORIZONTAL);
    calculateOverlayItemLayout(itemRectMap, OverlayItem::TOP_LEFT,     OverlayItem::VERTICAL);
    calculateOverlayItemLayout(itemRectMap, OverlayItem::TOP_RIGHT,    OverlayItem::HORIZONTAL);
    calculateOverlayItemLayout(itemRectMap, OverlayItem::TOP_RIGHT,    OverlayItem::VERTICAL);
    calculateOverlayItemLayout(itemRectMap, OverlayItem::BOTTOM_LEFT,  OverlayItem::HORIZONTAL);
    calculateOverlayItemLayout(itemRectMap, OverlayItem::BOTTOM_LEFT,  OverlayItem::VERTICAL);
    calculateOverlayItemLayout(itemRectMap, OverlayItem::BOTTOM_RIGHT, OverlayItem::HORIZONTAL);
    calculateOverlayItemLayout(itemRectMap, OverlayItem::BOTTOM_RIGHT, OverlayItem::VERTICAL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::calculateOverlayItemLayout(OverlayItemRectMap* itemRectMap, OverlayItem::LayoutCorner corner, OverlayItem::LayoutDirection direction)
{
    const uint border = 3;
    const Vec2ui vpSize = Vec2ui(m_camera->viewport()->width(), m_camera->viewport()->height());

    Vec2ui cursor(0,0);
    switch (corner)
    {
        case OverlayItem::TOP_LEFT:     cursor.set(border, vpSize.y() - border); break;
        case OverlayItem::TOP_RIGHT:    cursor.set(vpSize.x() - border, vpSize.y() - border); break;
        case OverlayItem::BOTTOM_LEFT:  cursor.set(border, border); break;
        case OverlayItem::BOTTOM_RIGHT: cursor.set(vpSize.x() - border, border); break;
        default:                        cursor.set(border,border);
    }

    size_t numOverlayItems = m_overlayItems.size();
    size_t i;
    for (i = 0; i < numOverlayItems; i++)
    {
        OverlayItemLayout item = m_overlayItems.at(i);
        if ((item.corner == corner) && (item.direction == direction))
        {
            CVF_ASSERT(item.overlayItem.notNull());

            // Find this position and size
            Vec2ui position = cursor;
            Vec2ui size = item.overlayItem->sizeHint();
            if ((corner == OverlayItem::TOP_RIGHT) || (corner == OverlayItem::BOTTOM_RIGHT))
            {
                position.x() -= size.x();
            }

            if ((corner == OverlayItem::TOP_LEFT) || (corner == OverlayItem::TOP_RIGHT))
            {
                position.y() -= size.y();
            }

            // Store the position in the map
            Rectui rect(position.x(), position.y(), size.x(), size.y());
            (*itemRectMap)[item.overlayItem.p()] = rect;

            // Find next position, moving the cursor
            if (direction == OverlayItem::HORIZONTAL)
            {
                if ((corner == OverlayItem::TOP_LEFT) || (corner == OverlayItem::BOTTOM_LEFT))
                {
                    cursor.x() += (size.x() + border);
                }
                else
                {
                    cursor.x() -= (size.x() + border);
                }
            }
            else if (direction == OverlayItem::VERTICAL)
            {
                if ((corner == OverlayItem::BOTTOM_LEFT) || (corner == OverlayItem::BOTTOM_RIGHT))
                {
                    cursor.y() += (size.y() + border);
                }
                else
                {
                    cursor.y() -= (size.y() + border);
                }
            }
            else
            {
                CVF_FAIL_MSG("Unhandled OverlayItem::LayoutDirection");
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Set the camera used in this rendering. Can be shared with other renderings etc.
//--------------------------------------------------------------------------------------------------
void Rendering::setCamera(Camera* camera)
{
    m_camera = camera;
}


//--------------------------------------------------------------------------------------------------
/// Get the camera used in this rendering
//--------------------------------------------------------------------------------------------------
Camera* Rendering::camera()
{
    return m_camera.p();
}


//--------------------------------------------------------------------------------------------------
/// Get the camera used in this rendering
//--------------------------------------------------------------------------------------------------
const Camera* Rendering::camera() const
{
    return m_camera.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderEngine* Rendering::renderEngine() 
{
    return &m_renderEngine;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RenderEngine* Rendering::renderEngine() const
{
    return &m_renderEngine;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::setTargetFrameBuffer(FramebufferObject* frameBuffer)
{
    m_targetFrameBuffer = frameBuffer;
}


//--------------------------------------------------------------------------------------------------
/// Set the sorter to use when sorting the render queue. 
/// 
/// Can specify NULL for no sorting.
//--------------------------------------------------------------------------------------------------
void Rendering::setRenderQueueSorter(RenderQueueSorter* sorter)
{
    m_renderQueueSorter = sorter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderQueueSorter* Rendering::renderQueueSorter()
{
    return m_renderQueueSorter.p();
}


//--------------------------------------------------------------------------------------------------
/// Create a new RayIntersectSpec object based on this rendering and the specified coordinates
/// 
/// The coordinates (\a winCoordX & \a winCoordY) are assumed to be in the  window coordinate system
/// where <0,0> if in the top left corner.
//--------------------------------------------------------------------------------------------------
ref<RayIntersectSpec> Rendering::createRayIntersectSpec(int winCoordX, int winCoordY) const
{
    if (m_scene.isNull() || m_camera.isNull()) 
    {
        return NULL;
    }

    ref<Ray> ray = m_camera->rayFromWinCoord(winCoordX, winCoordY);
    if (ray.isNull()) 
    {
        return NULL;
    }

    ref<RayIntersectSpec> rayIntersectSpec = new RayIntersectSpec(ray.p());
    return rayIntersectSpec;
}


//--------------------------------------------------------------------------------------------------
/// Intersect all parts in this rendering and return resulting hits
/// 
/// The HitItemCollection will contain a list of hits in all parts intersected by the ray.
/// The list will be sorted based on distance, so the closest item to the eye will be first in the list.
//--------------------------------------------------------------------------------------------------
bool Rendering::rayIntersect(RayIntersectSpec& rayIntersectSpec, HitItemCollection* hitItemCollection) 
{
    if (m_scene.isNull()) return false;

    bool anyHits = false;

    uint numModels = m_scene->modelCount();
    uint i;
    for (i = 0; i < numModels; i++)
    {
        Model* model = m_scene->model(i);
        CVF_ASSERT(model);

        anyHits |= model->rayIntersect(rayIntersectSpec, hitItemCollection);
    }

    // Finally, sort the item collection so the first item is the closest one to the eye
    if (hitItemCollection)
    {
        hitItemCollection->sort();
    }

    return anyHits;
}


//--------------------------------------------------------------------------------------------------
/// Get the bounding box of the Rendering
//--------------------------------------------------------------------------------------------------
BoundingBox Rendering::boundingBox() const
{
    if (m_scene.notNull())
    {
        return m_scene->boundingBox();
    }
    else
    {
        return BoundingBox();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::setEnableMask(unsigned int mask)
{
    m_enableMask = mask;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
unsigned int Rendering::enableMask() const
{
    return m_enableMask;
}


//--------------------------------------------------------------------------------------------------
/// Specify what (if any) should be cleared when the viewport is applied to OpenGL
//--------------------------------------------------------------------------------------------------
void Rendering::setClearMode(Viewport::ClearMode clearMode)
{
    m_clearMode = clearMode;
}


//--------------------------------------------------------------------------------------------------
/// Returns the current clear mode specifying what to clear when the viewport is applied to OpenGL
//--------------------------------------------------------------------------------------------------
Viewport::ClearMode Rendering::clearMode() const
{
    return m_clearMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::setEffectOverride(Effect* effect)
{
    m_effectOverride = effect;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Effect* Rendering::effectOverride()
{
    return m_effectOverride.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::addDynamicUniformSet(DynamicUniformSet* dynUniformSet)
{
    CVF_ASSERT(dynUniformSet);
    m_dynamicUniformSets.push_back(dynUniformSet);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::removeDynamicUniformSet(DynamicUniformSet* dynUniformSet)
{
    CVF_ASSERT(dynUniformSet);
    m_dynamicUniformSets.erase(dynUniformSet);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::removeAllDynamicUniformSets()
{
    m_dynamicUniformSets.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::addGlobalDynamicUniformSet(DynamicUniformSet* dynUniformSet)
{
    CVF_ASSERT(dynUniformSet);
    m_globalDynamicUniformSets.push_back(dynUniformSet);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::removeAllGlobalUniformSets()
{
    m_globalDynamicUniformSets.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::updateDynamicUniformSets()
{
    size_t numSets = m_dynamicUniformSets.size();
    if (numSets == 0)
    {
        return;
    }

    size_t i;
    for (i = 0; i < numSets; i++)
    {
        ref<DynamicUniformSet> dynUniformSet = m_dynamicUniformSets[i];
        dynUniformSet->update(this);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::updateAndCombineGlobalDynamicUniformSets()
{
    m_combinedGlobalUniformSet = NULL;

    size_t numSets = m_globalDynamicUniformSets.size();
    if (numSets == 0)
    {
        return;
    }

    m_combinedGlobalUniformSet = new UniformSet;

    size_t idus;
    for (idus = 0; idus < numSets; idus++)
    {
        ref<DynamicUniformSet> dynUniformSet = m_globalDynamicUniformSets[idus];
        dynUniformSet->update(this);

        ref<UniformSet> uniformSet = dynUniformSet->uniformSet();
        if (uniformSet.notNull())
        {
            size_t numUniforms = uniformSet->count();
            size_t iu;
            for (iu = 0; iu < numUniforms; iu++)
            {
                Uniform* u = uniformSet->uniform(iu);
                m_combinedGlobalUniformSet->setUniform(u);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Get the performance info of the last render() call
//--------------------------------------------------------------------------------------------------
const PerformanceInfo& Rendering::performanceInfo() const
{
    return m_performanceInfo;
}


//--------------------------------------------------------------------------------------------------
/// Turn performance timing on or off (to possibly increase performance)
//--------------------------------------------------------------------------------------------------
void Rendering::enablePerformanceTiming(bool enable)
{
    m_enablePerformanceTiming = enable;
}


//--------------------------------------------------------------------------------------------------
/// Check if performance timing is enabled or not
//--------------------------------------------------------------------------------------------------
bool Rendering::isPerformanceTimingEnabled() const
{
    return m_enablePerformanceTiming;
}


//--------------------------------------------------------------------------------------------------
/// Returns a ptr to the cull settings. 
///
/// These settings are used when building the visible part queue.
//--------------------------------------------------------------------------------------------------
CullSettings* Rendering::cullSettings()
{
    return &m_cullSettings;
}


//--------------------------------------------------------------------------------------------------
/// Returns a const ref to the cull settings. 
///
/// These settings are used when building the visible part queue.
//--------------------------------------------------------------------------------------------------
const CullSettings* Rendering::cullSettings() const
{
    return &m_cullSettings;
}


//--------------------------------------------------------------------------------------------------
/// Limit the maximum number of parts that will be drawn
/// 
/// Typically used to achieve a target frame rate. \sa clearMaxNumPartsToDraw()
//--------------------------------------------------------------------------------------------------
void Rendering::setMaxNumPartsToDraw(size_t maxNumPartsToDraw)
{
    m_maxNumPartsToDraw = maxNumPartsToDraw;
}


//--------------------------------------------------------------------------------------------------
/// Clear current limit on the maximum number of parts to draw
/// 
/// \sa setMaxNumPartsToDraw()
//--------------------------------------------------------------------------------------------------
void Rendering::clearMaxNumPartsToDraw()
{
    m_maxNumPartsToDraw = std::numeric_limits<size_t>::max();
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of overlay items in this rendering
//--------------------------------------------------------------------------------------------------
size_t Rendering::overlayItemCount() const
{
    return m_overlayItems.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::addOverlayItem(OverlayItem* overlayItem, OverlayItem::LayoutCorner corner, OverlayItem::LayoutDirection direction)
{
    CVF_ASSERT(overlayItem);

    OverlayItemLayout item;
    item.corner = corner;
    item.direction = direction;
    item.overlayItem = overlayItem;

    m_overlayItems.push_back(item);
}


//--------------------------------------------------------------------------------------------------
/// Returns the overlay item at the given index.
/// 
/// corner and direction are optional parameters to receive the layout attachment of the overlay item
//--------------------------------------------------------------------------------------------------
OverlayItem* Rendering::overlayItem(size_t index, OverlayItem::LayoutCorner* corner, OverlayItem::LayoutDirection* direction)
{
    CVF_ASSERT(index < overlayItemCount());

    if (corner)
    {
        *corner = m_overlayItems[index].corner;
    }

    if (direction)
    {
        *direction = m_overlayItems[index].direction;
    }

    return m_overlayItems[index].overlayItem.p();
}


//--------------------------------------------------------------------------------------------------
/// Returns the overlay item at the given index.
/// 
/// corner and direction are optional parameters to receive the layout attachment of the overlay item
//--------------------------------------------------------------------------------------------------
const OverlayItem* Rendering::overlayItem(size_t index, OverlayItem::LayoutCorner* corner, OverlayItem::LayoutDirection* direction) const
{
    CVF_ASSERT(index < overlayItemCount());

    if (corner)
    {
        *corner = m_overlayItems[index].corner;
    }

    if (direction)
    {
        *direction = m_overlayItems[index].direction;
    }

    return m_overlayItems[index].overlayItem.p();
}


//--------------------------------------------------------------------------------------------------
/// Get the the overlay item (if any) at the given cursor position
//--------------------------------------------------------------------------------------------------
OverlayItem* Rendering::overlayItemFromWinCoord(uint winCoordX, uint winCoordY)
{
    // Overlay items are stored OpenGL coord system with (0,0) in lower left corner, so flip the y-coordinate
    winCoordY = static_cast<int>(m_camera->viewport()->height()) - winCoordY;

    OverlayItemRectMap itemRectMap;
    calculateOverlayItemLayout(&itemRectMap);

    OverlayItemRectMap::iterator it;
    for (it = itemRectMap.begin(); it != itemRectMap.end(); ++it)
    {
        OverlayItem* item = it->first;
        Rectui rect = it->second;

        if ((winCoordX > rect.min().x()) && (winCoordX < rect.max().x()) &&
            (winCoordY > rect.min().y()) && (winCoordY < rect.max().y()))
        {
            return item;
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::removeOverlayItem(const OverlayItem* overlayItem)
{
    CVF_UNUSED(overlayItem);

    std::vector<OverlayItemLayout>::iterator it;
    for (it = m_overlayItems.begin(); it != m_overlayItems.end(); it++)
    {
        if (it->overlayItem == overlayItem)
        {
            m_overlayItems.erase(it);
            break;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rendering::removeAllOverlayItems()
{
    m_overlayItems.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Rendering::toString() const
{
    String str = "Rendering: ";
    str += m_renderingName;
    str += "\n Clearmode:                       ";

    switch (m_clearMode)
    {
        case Viewport::DO_NOT_CLEAR:              str += "DO_NOT_CLEAR"; break; 
        case Viewport::CLEAR_COLOR:               str += "CLEAR_COLOR"; break; 
        case Viewport::CLEAR_DEPTH:               str += "CLEAR_DEPTH"; break; 
        case Viewport::CLEAR_STENCIL:             str += "CLEAR_STENCIL"; break; 
        case Viewport::CLEAR_COLOR_DEPTH:         str += "CLEAR_COLOR_DEPTH"; break; 
        case Viewport::CLEAR_COLOR_STENCIL:       str += "CLEAR_COLOR_STENCIL"; break; 
        case Viewport::CLEAR_DEPTH_STENCIL:       str += "CLEAR_DEPTH_STENCIL"; break; 
        case Viewport::CLEAR_COLOR_DEPTH_STENCIL: str += "CLEAR_COLOR_DEPTH_STENCIL"; break; 
    }

    str += "\n enableMask:                      " + String(m_enableMask);
    str += "\n m_maxNumPartsToDraw:             " + String(m_enableMask);

    str += "\n m_viewFrustumCulling:            " + String(m_cullSettings.isViewFrustumCullingEnabled());
    str += "\n m_pixelSizeCulling:              " + String(m_cullSettings.isPixelSizeCullingEnabled());
    str += "\n m_pixelSizeCullingAreaThreshold: " + String(m_cullSettings.pixelSizeCullingAreaThreshold());

    return str;
}


} // namespace cvf

