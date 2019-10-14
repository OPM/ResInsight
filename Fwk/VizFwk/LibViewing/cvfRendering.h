//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
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
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfRenderEngine.h"
#include "cvfViewport.h"
#include "cvfCullSettings.h"
#include "cvfPerformanceInfo.h"
#include "cvfBoundingBox.h"
#include "cvfCollection.h"
#include "cvfOverlayItem.h"
#include "cvfRect.h"

#include <map>

namespace cvf {

class Scene;
class Camera;
class PartRenderHintCollection;
class RenderQueueSorter;
class Effect;
class FramebufferObject;
class DynamicUniformSet;
class UniformSet;
class RayIntersectSpec;
class HitItemCollection;
class OpenGLContext;
class RenderingScissor;


//==================================================================================================
//
// Rendering
//
//==================================================================================================
class Rendering : public Object
{
public:
    Rendering(const String& renderingName = String());
    ~Rendering();

    void                    setRenderingName(const String& renderingName);
    String                  renderingName() const;

    void                    setScene(Scene* scene);
    Scene*                  scene();
    const Scene*            scene() const;

    void                    setCamera(Camera* camera);
    Camera*                 camera();
    const Camera*           camera() const;

    void                    render(OpenGLContext* oglContext);

    void                    setRenderQueueSorter(RenderQueueSorter* sorter);
    RenderQueueSorter*      renderQueueSorter();
    RenderEngine*           renderEngine();
    const RenderEngine*     renderEngine() const;

    FramebufferObject*      targetFramebuffer();
    void                    setTargetFramebuffer(FramebufferObject* framebuffer);

    ref<RayIntersectSpec>   rayIntersectSpecFromWindowCoordinates(int x, int y) const;
    bool                    rayIntersect(RayIntersectSpec& rayIntersectSpec, HitItemCollection* hitItemCollection);
    
    BoundingBox             boundingBox() const;

    void                    setEnableMask(uint mask);
    uint                    enableMask() const;

    void                    setClearMode(Viewport::ClearMode clearMode);
    Viewport::ClearMode     clearMode() const;
    void                    setRenderingScissor(RenderingScissor* scissor);

    void                    setEffectOverride(Effect* effect);
    Effect*                 effectOverride();

    void                    addDynamicUniformSet(DynamicUniformSet* dynUniformSet);
    void                    removeDynamicUniformSet(DynamicUniformSet* dynUniformSet);
    void                    removeAllDynamicUniformSets();
    void                    addGlobalDynamicUniformSet(DynamicUniformSet* dynUniformSet);
    void                    removeAllGlobalDynamicUniformSets();

    const PerformanceInfo&  performanceInfo() const;
    void                    enablePerformanceTiming(bool enable);
    bool                    isPerformanceTimingEnabled() const;

    CullSettings*           cullSettings();
    const CullSettings*     cullSettings() const;
    void                    setMaxNumPartsToDraw(size_t maxNumPartsToDraw);
    void                    clearMaxNumPartsToDraw();

    size_t			        overlayItemCount() const;
    void                    addOverlayItem(OverlayItem* overlayItem);
    OverlayItem*            overlayItem(size_t index);
    const OverlayItem*      overlayItem(size_t index) const;
    OverlayItem*            overlayItemFromWindowCoordinates(int x, int y);
    void                    removeOverlayItem(const OverlayItem* overlayItem);
    void                    removeAllOverlayItems();

    String                  debugString() const;

private:
    void                    renderOverlayItems(OpenGLContext* oglContext, bool useSoftwareRendering);

    typedef std::map<const cvf::OverlayItem*, cvf::Recti>  OverlayItemRectMap;
    void                    calculateOverlayItemLayout(OverlayItemRectMap* itemRectMap);
    void                    calculateOverlayItemLayoutForSchemeAndCorner(OverlayItemRectMap* itemRectMap, OverlayItem::LayoutScheme layoutScheme, OverlayItem::AnchorCorner anchorCorner);

    void                    updateDynamicUniformSets();
    void                    updateAndCombineGlobalDynamicUniformSets();

private:
    String                          m_renderingName;
    ref<Scene>                      m_scene;
    ref<Camera>                     m_camera;
    ref<FramebufferObject>          m_targetFramebuffer;        // The target framebuffer for the rendering. NULL means the default window framebuffer.

    Collection<OverlayItem>         m_overlayItems;

    ref<PartRenderHintCollection>   m_visibleParts;             // The collection of visible parts for one pass. The collection class is reused between passes
    ref<RenderQueueSorter>          m_renderQueueSorter;        // Render queue sorter, initialized to a basic sorter with minimal sorting strategy
    RenderEngine                    m_renderEngine;             // 

    uint                            m_enableMask;               // Mask will be compared against the contained scene's models and the model's parts when determining visible parts
    Viewport::ClearMode             m_clearMode;
    ref<Effect>                     m_effectOverride;           // Can hold an overriding effect. All parts drawn by this rendering will use this effect
    ref<RenderingScissor>           m_renderingScissor;         // Can hold a scissor used on the rendered scene. Will not affect overly items

    Collection<DynamicUniformSet>   m_dynamicUniformSets;       // Collection of user added dynamic uniform sets
    Collection<DynamicUniformSet>   m_globalDynamicUniformSets; // Collection of global user added dynamic uniform sets
    ref<UniformSet>                 m_combinedGlobalUniformSet; // Global uniform set, this is the combination of the uniform sets from all the dynamic uniform sets

    ref<CullSettings>               m_cullSettings;             // Cull settings used when extracting visible parts from the scene
    size_t                          m_maxNumPartsToDraw;        // The maximum number of parts to draw during render pass

    PerformanceInfo                 m_performanceInfo;
    bool                            m_enablePerformanceTiming;

    ref<Logger>                     m_logger;
};

}
