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

#include "cafViewer.h"

#include "cafCadNavigation.h"
#include "cafFrameAnimationControl.h"
#include "cafNavigationPolicy.h"
#include "cafPointOfInterestVisualizer.h"

#include "cvfCamera.h"
#include "cvfDebugTimer.h"
#include "cvfDrawable.h"
#include "cvfDrawableGeo.h"
#include "cvfDynamicUniformSet.h"
#include "cvfFramebufferObject.h"
#include "cvfHitItemCollection.h"
#include "cvfManipulatorTrackball.h"
#include "cvfModel.h"
#include "cvfOpenGLCapabilities.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOverlayImage.h"
#include "cvfPart.h"
#include "cvfRay.h"
#include "cvfRayIntersectSpec.h"
#include "cvfRenderQueueSorter.h"
#include "cvfRenderSequence.h"
#include "cvfRenderbufferObject.h"
#include "cvfRendering.h"
#include "cvfScene.h"
#include "cvfShaderSourceProvider.h"
#include "cvfSingleQuadRenderingGenerator.h"
#include "cvfTextureImage.h"
#include "cvfTransform.h"
#include "cvfUniform.h"
#include "cvfUniformSet.h"

#include "cvfqtOpenGLContext.h"
#include "cvfqtPerformanceInfoHud.h"
#include "cvfqtUtils.h"

#include <cmath>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputEvent>
#include "cvfRenderingScissor.h"

namespace caf
{

class GlobalViewerDynUniformSet: public cvf::DynamicUniformSet
{
public:
    GlobalViewerDynUniformSet()
    {
        m_headlightPosition = new cvf::UniformFloat("u_ecLightPosition", cvf::Vec3f(0.5, 5.0, 7.0));
        m_uniformSet = new cvf::UniformSet();
        m_uniformSet->setUniform(m_headlightPosition.p());
    }

    ~GlobalViewerDynUniformSet() override {}

    void setHeadLightPosition(const cvf::Vec3f posRelativeToCamera) { m_headlightPosition->set(posRelativeToCamera);}


    cvf::UniformSet* uniformSet() override { return m_uniformSet.p(); }
    void        update(cvf::Rendering* rendering) override{};      

private:
    cvf::ref<cvf::UniformSet>   m_uniformSet;
    cvf::ref<cvf::UniformFloat> m_headlightPosition;
};


}

std::list<caf::Viewer*> caf::Viewer::sm_viewers;
cvf::ref<cvf::OpenGLContextGroup> caf::Viewer::sm_openGLContextGroup;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::Viewer::Viewer(const QGLFormat& format, QWidget* parent)
    #if QT_VERSION >= 0x050000
    :   caf::OpenGLWidget(contextGroup(), format, nullptr, sharedWidget()),
    #else
    :   caf::OpenGLWidget(contextGroup(), format, new QWidget(parent), sharedWidget()),
    #endif
    m_navigationPolicy(nullptr),
    m_navigationPolicyEnabled(true),
    m_defaultPerspectiveNearPlaneDistance(0.05),
    m_maxClipPlaneDistance(cvf::UNDEFINED_DOUBLE),
    m_cameraFieldOfViewYDeg(40.0),
    m_paintCounter(0),
    m_releaseOGLResourcesEachFrame(false),
    m_isOverlayPaintingEnabled(true),
    m_offscreenViewportWidth(0),
    m_offscreenViewportHeight(0),
    m_parallelProjectionLightDirection(0, 0, -1), // Light directly from behind
    m_comparisonViewOffset(0, 0, 0),
    m_comparisonWindowNormalizedRect(0.5f, 0.0f, 0.5f, 1.0f)
{
    #if QT_VERSION >= 0x050000
    m_layoutWidget = new QWidget(parent);
    #else
    m_layoutWidget = parentWidget();
    #endif
    
    QHBoxLayout* layout = new QHBoxLayout(m_layoutWidget);

    layout->addWidget(this);
    layout->setContentsMargins(0, 0, 0, 0);

    setAutoFillBackground(false);
    setMouseTracking(true);

    // Needed to get keystrokes
    setFocusPolicy(Qt::ClickFocus);

    m_globalUniformSet = new GlobalViewerDynUniformSet();

    m_mainCamera = new cvf::Camera;
    m_mainCamera->setFromLookAt(cvf::Vec3d(0,0,-1), cvf::Vec3d(0,0,0), cvf::Vec3d(0,1,0));
    m_comparisonMainCamera = new cvf::Camera;
    m_comparisonMainCamera->setFromLookAt(cvf::Vec3d(0,0,-1), cvf::Vec3d(0,0,0), cvf::Vec3d(0,1,0));

    m_renderingSequence = new cvf::RenderSequence();
    m_renderingSequence->setDefaultFFLightPositional(cvf::Vec3f(0.5, 5.0, 7.0));

    m_mainRendering = new cvf::Rendering("Main Rendering");
    m_comparisonMainRendering = new cvf::Rendering("Comparison Rendering");
    m_overlayItemsRendering  = new cvf::Rendering("Overlay Rendering");
    m_overlayItemsRendering->setClearMode(cvf::Viewport::DO_NOT_CLEAR);

    m_comparisonRenderingScissor = new cvf::RenderingScissor;
    m_comparisonMainRendering->setRenderingScissor(m_comparisonRenderingScissor.p());

    m_animationControl = new caf::FrameAnimationControl(this);
    connect(m_animationControl, SIGNAL(changeFrame(int)), SLOT(slotSetCurrentFrame(int)));
    connect(m_animationControl, SIGNAL(endAnimation()), SLOT(slotEndAnimation()));


    this->setNavigationPolicy(new caf::CadNavigation);

    m_overlayTextureImage = new cvf::TextureImage;
    m_overlayImage = new cvf::OverlayImage(m_overlayTextureImage.p());
    m_overlayImage->setBlending(cvf::OverlayImage::TEXTURE_ALPHA);
    m_overlayImage->setLayoutFixedPosition(cvf::Vec2i(0,0));

    setupMainRendering();
    setupRenderingSequence();

    m_showPerfInfoHud = false;

    sm_viewers.push_back(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::Viewer::~Viewer()
{
    this->cvfShutdownOpenGLContext();
    sm_viewers.remove(this);

    // To delete the layout widget
    if (m_layoutWidget) m_layoutWidget->deleteLater();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setupMainRendering()
{
    m_mainRendering->setCamera(m_mainCamera.p());
    m_comparisonMainRendering->setCamera(m_comparisonMainCamera.p());
    m_overlayItemsRendering->setCamera(m_mainCamera.p());

    m_mainRendering->setRenderQueueSorter(new cvf::RenderQueueSorterBasic(cvf::RenderQueueSorterBasic::EFFECT_ONLY));
    m_comparisonMainRendering->setRenderQueueSorter(new cvf::RenderQueueSorterBasic(cvf::RenderQueueSorterBasic::EFFECT_ONLY));
    m_overlayItemsRendering->setRenderQueueSorter(new cvf::RenderQueueSorterBasic(cvf::RenderQueueSorterBasic::EFFECT_ONLY));

    m_mainRendering->addGlobalDynamicUniformSet(m_globalUniformSet.p());
    m_comparisonMainRendering->addGlobalDynamicUniformSet(m_globalUniformSet.p());

    // Set fixed function rendering if QGLFormat does not support directRendering
    if (!this->format().directRendering())
    {
        m_mainRendering->renderEngine()->enableForcedImmediateMode(true);
        m_comparisonMainRendering->renderEngine()->enableForcedImmediateMode(true);
        m_overlayItemsRendering->renderEngine()->enableForcedImmediateMode(true);
    }

    if (contextGroup()->capabilities() &&
        contextGroup()->capabilities()->hasCapability(cvf::OpenGLCapabilities::FRAMEBUFFER_OBJECT))
    {
        m_offscreenFbo = new cvf::FramebufferObject;

        m_mainRendering->setTargetFramebuffer(m_offscreenFbo.p());
        m_comparisonMainRendering->setTargetFramebuffer(m_offscreenFbo.p());
        m_overlayItemsRendering->setTargetFramebuffer(m_offscreenFbo.p());

        cvf::ref<cvf::RenderbufferObject> rbo = new cvf::RenderbufferObject(cvf::RenderbufferObject::DEPTH_COMPONENT24, 1, 1);
        m_offscreenFbo->attachDepthRenderbuffer(rbo.p());

        m_offscreenTexture = new cvf::Texture(cvf::Texture::TEXTURE_2D, cvf::Texture::RGBA);
        m_offscreenTexture->setSize(1, 1);
        m_offscreenFbo->attachColorTexture2d(0, m_offscreenTexture.p());
    }

    updateOverlayImagePresence();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setupRenderingSequence()
{
    m_renderingSequence->addRendering(m_mainRendering.p());
    m_renderingSequence->addRendering(m_comparisonMainRendering.p());
    m_renderingSequence->addRendering(m_overlayItemsRendering.p());

    if (m_offscreenFbo.notNull())
    {
        // Setup second rendering drawing the texture on the screen
        // -------------------------------------------------------------------------
        cvf::SingleQuadRenderingGenerator quadRenderGen;
        cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
        sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
        sampler->setMinFilter(cvf::Sampler::NEAREST);
        sampler->setMagFilter(cvf::Sampler::NEAREST);

        quadRenderGen.addTexture(m_offscreenTexture.p(), sampler.p(), "u_texture2D");
        quadRenderGen.addFragmentShaderCode(cvf::ShaderSourceProvider::instance()->getSourceFromRepository(cvf::ShaderSourceRepository::fs_Unlit));
        quadRenderGen.addFragmentShaderCode(cvf::ShaderSourceProvider::instance()->getSourceFromRepository(cvf::ShaderSourceRepository::src_Texture));

        m_quadRendering = quadRenderGen.generate();
        m_renderingSequence->addRendering(m_quadRendering.p());
    }

    updateCamera(width(), height());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::Viewer* caf::Viewer::sharedWidget()
{
    if (sm_viewers.size() > 0)
    {
        return *(sm_viewers.begin());
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OpenGLContextGroup* caf::Viewer::contextGroup()
{
    if (sm_openGLContextGroup.isNull())
    {
        sm_openGLContextGroup = new cvf::OpenGLContextGroup();
    }

    return sm_openGLContextGroup.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Camera* caf::Viewer::mainCamera()
{
    return m_mainCamera.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Camera* caf::Viewer::comparisonMainCamera()
{
    return m_comparisonMainCamera.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setComparisonViewEyePointOffset(const cvf::Vec3d& offset)
{
    m_comparisonViewOffset = offset;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d caf::Viewer::comparisonViewEyePointOffset()
{
    return m_comparisonViewOffset;
}

//--------------------------------------------------------------------------------------------------
/// setNormalizedComparisonViewRect
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setComparisonViewVisibleNormalizedRect( const cvf::Rectf& visibleRect )
{
    m_comparisonWindowNormalizedRect = visibleRect;

    updateCamera(width(), height());
    update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Rectf caf::Viewer::comparisonViewVisibleNormalizedRect() const 
{
    return m_comparisonWindowNormalizedRect;
}

//--------------------------------------------------------------------------------------------------
/// Set the scene to be rendered when the animation is inactive (Stopped)
//--------------------------------------------------------------------------------------------------
void  caf::Viewer::setMainScene(cvf::Scene* scene, bool isForComparisonView )
{
    appendAllStaticModelsToFrame(scene, isForComparisonView);
    if ( !isForComparisonView )
    {
        m_mainScene = scene;
        m_mainRendering->setScene(scene);
    }
    else
    {
        m_comparisonMainScene = scene;
        m_comparisonMainRendering->setScene(scene);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Scene* caf::Viewer::mainScene( bool isForComparisonView)
{
    if (!isForComparisonView)
    {
        return m_mainScene.p();
    }
    else
    {
        return m_comparisonMainScene.p();
    }
}

//--------------------------------------------------------------------------------------------------
/// Return the currently rendered scene
//--------------------------------------------------------------------------------------------------
cvf::Scene* caf::Viewer::currentScene(bool isForComparisonView)
{
    if (!isForComparisonView)
    {
        return m_mainRendering->scene();
    }
    else
    {
        return m_comparisonMainRendering->scene();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::updateCamera(int width, int height)
{
    if (width < 1 || height < 1) return;

    m_mainCamera->viewport()->set(0, 0, width, height);
    m_comparisonMainCamera->viewport()->set(0, 0, width, height);
    m_comparisonRenderingScissor->setScissorRectangle(static_cast<int>(width  * m_comparisonWindowNormalizedRect.min().x()),
                                                      static_cast<int>(height * m_comparisonWindowNormalizedRect.min().y()),
                                                      static_cast<int>(width  * m_comparisonWindowNormalizedRect.width()),
                                                      static_cast<int>(height * m_comparisonWindowNormalizedRect.height()));

    if (m_mainCamera->projection() == cvf::Camera::PERSPECTIVE)
    {
        m_mainCamera->setProjectionAsPerspective(m_cameraFieldOfViewYDeg, m_mainCamera->nearPlane(), m_mainCamera->farPlane());
    }
    else
    {
        m_mainCamera->setProjectionAsOrtho(m_mainCamera->frontPlaneFrustumHeight(), m_mainCamera->nearPlane(), m_mainCamera->farPlane());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::canRender() const
{
    if (m_renderingSequence->renderingCount() < 1) return false;

    if (m_mainCamera.isNull()) return false;
    if (m_mainCamera->viewport()->width() < 1) return false;
    if (m_mainCamera->viewport()->height() < 1) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::optimizeClippingPlanes()
{
    double nearPlaneDist = HUGE_VAL;
    double farPlaneDist = HUGE_VAL;

    cvf::Vec3d navPointOfinterest = m_navigationPolicy->pointOfInterest();

    if ( calculateNearFarPlanes(m_mainRendering.p(), navPointOfinterest, &farPlaneDist, &nearPlaneDist) )
    {
        if ( m_mainCamera->projection() == cvf::Camera::PERSPECTIVE )
        {
            m_mainCamera->setProjectionAsPerspective(m_cameraFieldOfViewYDeg, nearPlaneDist, farPlaneDist);
        }
        else
        {
            m_mainCamera->setProjectionAsOrtho(m_mainCamera->frontPlaneFrustumHeight(), nearPlaneDist, farPlaneDist);
        }
    }

    copyCameraView(m_mainCamera.p(), m_comparisonMainCamera.p() );



    if ( m_comparisonMainRendering->scene() )
    {
        cvf::Vec3d camUp;
        cvf::Vec3d camEye;
        cvf::Vec3d camViewRefPoint;

        m_comparisonMainCamera->toLookAt( &camEye, &camViewRefPoint, &camUp );
        camEye += m_comparisonViewOffset;
        camViewRefPoint += m_comparisonViewOffset;
        m_comparisonMainCamera->setFromLookAt(camEye, camViewRefPoint, camUp);

        if ( calculateNearFarPlanes(m_comparisonMainRendering.p(), navPointOfinterest, &farPlaneDist, &nearPlaneDist) )
        {
            if ( m_comparisonMainCamera->projection() == cvf::Camera::PERSPECTIVE )
            {
                m_comparisonMainCamera->setProjectionAsPerspective(m_cameraFieldOfViewYDeg, nearPlaneDist, farPlaneDist);
            }
            else
            {
                m_comparisonMainCamera->setProjectionAsOrtho(m_comparisonMainCamera->frontPlaneFrustumHeight(), nearPlaneDist, farPlaneDist);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::calculateNearFarPlanes(const cvf::Rendering* rendering, 
                                         const cvf::Vec3d& navPointOfinterest, 
                                         double *farPlaneDist, 
                                         double *nearPlaneDist)
{
    cvf::BoundingBox bb = rendering->boundingBox();

    if ( !bb.isValid() ) return false;

    cvf::Vec3d eye     = rendering->camera()->position();
    cvf::Vec3d viewdir = rendering->camera()->direction();

    cvf::Vec3d bboxCorners[8];
    bb.cornerVertices(bboxCorners);

    // Find the distance to the bbox corners most behind and most in front of camera

    double maxDistEyeToCornerAlongViewDir = -HUGE_VAL;
    double minDistEyeToCornerAlongViewDir = HUGE_VAL;
    for ( int bcIdx = 0; bcIdx < 8; ++bcIdx )
    {
        double distEyeBoxCornerAlongViewDir = (bboxCorners[bcIdx] - eye)*viewdir;

        if ( distEyeBoxCornerAlongViewDir > maxDistEyeToCornerAlongViewDir )
        {
            maxDistEyeToCornerAlongViewDir = distEyeBoxCornerAlongViewDir;
        }

        if ( distEyeBoxCornerAlongViewDir < minDistEyeToCornerAlongViewDir )
        {
            minDistEyeToCornerAlongViewDir = distEyeBoxCornerAlongViewDir; // Sometimes negative-> behind camera
        }
    }

    (*farPlaneDist) = CVF_MIN(maxDistEyeToCornerAlongViewDir * 1.2, m_maxClipPlaneDistance);

    // Near-plane:

    bool isOrthoNearPlaneFollowingCamera = false;

    // If we have perspective projection, set the near plane just in front of camera, and not behind

    if ( rendering->camera()->projection() == cvf::Camera::PERSPECTIVE || isOrthoNearPlaneFollowingCamera )
    {
        // Choose the one furthest from the camera of: 0.8*bbox distance, m_minPerspectiveNearPlaneDistance.
        (*nearPlaneDist) = CVF_MAX(m_defaultPerspectiveNearPlaneDistance, 0.8*minDistEyeToCornerAlongViewDir);

        // If we are zooming into a detail, allow the near-plane to move towards camera beyond the m_minPerspectiveNearPlaneDistance
        if ( (*nearPlaneDist) == m_defaultPerspectiveNearPlaneDistance // We are inside the bounding box
            && m_navigationPolicy.notNull() && m_navigationPolicyEnabled )
        {
            double pointOfInterestDist = (eye - navPointOfinterest).length();
            (*nearPlaneDist) = CVF_MIN((*nearPlaneDist), pointOfInterestDist*0.2);
        }

        // Guard against the zero nearplane possibility
        if ( nearPlaneDist <= 0 ) (*nearPlaneDist) = m_defaultPerspectiveNearPlaneDistance;
    }
    else // Orthographic projection. Set to encapsulate the complete boundingbox, possibly setting a negative nearplane
    {
        if ( minDistEyeToCornerAlongViewDir >= 0 )
        {
            (*nearPlaneDist) = CVF_MIN(0.8 * minDistEyeToCornerAlongViewDir, m_maxClipPlaneDistance);
        }
        else
        {
            (*nearPlaneDist) = CVF_MAX(1.2 * minDistEyeToCornerAlongViewDir, -m_maxClipPlaneDistance);
        }
    }

    if ( (*farPlaneDist) <= (*nearPlaneDist) ) (*farPlaneDist) = (*nearPlaneDist) + 1.0;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Forward all events classified as QInputEvent to the navigation policy
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::event(QEvent* e)
{
    #if QT_VERSION >= 0x050000
    // The most reliable way we have found of detecting when an OpenGL context is about to be destroyed is
    // hooking into the QEvent::PlatformSurface event and checking for the SurfaceAboutToBeDestroyed event type.
    // From the Qt doc: 
    //   The underlying native surface will be destroyed immediately after this event.
    //   The SurfaceAboutToBeDestroyed event type is useful as a means of stopping rendering to a platform window before it is destroyed.
    if ( e->type() == QEvent::PlatformSurface )
    {
        QPlatformSurfaceEvent* platformSurfaceEvent = static_cast<QPlatformSurfaceEvent*>(e);
        if ( platformSurfaceEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed )
        {
            cvfShutdownOpenGLContext();
        }
    }
    #endif

    if (e && m_navigationPolicy.notNull() && m_navigationPolicyEnabled)
    {
        switch (e->type())
        {
        case QEvent::ContextMenu:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::ShortcutOverride:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::TabletMove:     
        case QEvent::TabletPress:     
        case QEvent::TabletRelease:
        case QEvent::TabletEnterProximity:
        case QEvent::TabletLeaveProximity:
        case QEvent::Wheel:
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:    
        case QEvent::TouchEnd:
            if (m_navigationPolicy->handleInputEvent(static_cast<QInputEvent*>(e)))
                return true;
            else return QGLWidget::event(e);
            break;
        default:
            return QGLWidget::event(e); 
            break;
        }
    }
    else return QGLWidget::event(e); 
}

//--------------------------------------------------------------------------------------------------
/// Set the pointer to the navigation policy to be used. Stored as a cvf::ref internally
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setNavigationPolicy(caf::NavigationPolicy* navigationPolicy)
{
    m_navigationPolicy = navigationPolicy;

    if (m_navigationPolicy.notNull())  m_navigationPolicy->setViewer(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const caf::NavigationPolicy* caf::Viewer::getNavigationPolicy() const
{
    return m_navigationPolicy.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::rayPick(int winPosX, int winPosY, cvf::HitItemCollection* pickedPoints, cvf::Vec3d* globalRayOrigin/*=nullptr*/)
{
    CVF_ASSERT(m_mainRendering.notNull());

    int translatedMousePosX = winPosX;
    int translatedMousePosY = height() - winPosY;

    cvf::ref<cvf::RayIntersectSpec> ris = m_mainRendering->rayIntersectSpecFromWindowCoordinates(translatedMousePosX, translatedMousePosY);
    if (ris.notNull())
    {
        bool retVal = m_mainRendering->rayIntersect(*ris, pickedPoints);
        if (retVal && globalRayOrigin)
        {
            CVF_ASSERT(ris->ray() != nullptr);
            *globalRayOrigin = ris->ray()->origin();
        }
        return retVal;
    }
    else
    {
        return false;
    }


}

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void caf::Viewer::resizeGL(int width, int height)
{
    if (width < 1 || height < 1) return;

    if (m_offscreenFbo.notNull())
    {
        m_offscreenFbo->resizeAttachedBuffers(width, height);
    

        m_offscreenViewportWidth = width;
        m_offscreenViewportHeight = height;
    }

    if (m_quadRendering.notNull())
    {
        m_quadRendering->camera()->viewport()->set(0, 0, width, height);
    }

    updateCamera(width, height);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::enablePerfInfoHud(bool enable)
{
    m_showPerfInfoHud = enable;
    updateOverlayImagePresence();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::isPerfInfoHudEnabled()
{
    return m_showPerfInfoHud;
}

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void caf::Viewer::paintEvent(QPaintEvent* event)
{
    makeCurrent();

    cvf::ref<cvf::OpenGLContext> myOglContext = cvfOpenGLContext();
    CVF_CHECK_OGL(myOglContext.p());
    CVF_ASSERT(myOglContext->isContextValid());

    QPainter painter(this);

    if (m_renderingSequence.isNull() || !canRender())
    {
        QColor bgClr(128, 128, 128);
        painter.fillRect(rect(), bgClr);
        return;
    }

    // If Qt overlay painting is enabled, paint to an QImage, and set it to the cvf::OverlayImage

    if (m_isOverlayPaintingEnabled || m_showPerfInfoHud)
    {
        // Set up image to draw to, and painter 
        if (m_overlayPaintingQImage.size() != this->size())
        {
            m_overlayPaintingQImage = QImage(this->size(), QImage::Format_ARGB32);
        }

        m_overlayPaintingQImage.fill(Qt::transparent);
        QPainter overlayPainter(&m_overlayPaintingQImage);

        // Call virtual method to allow subclasses to paint on the OpenGlCanvas

        if (m_isOverlayPaintingEnabled)
        {
            this->paintOverlayItems(&overlayPainter); 
        }

        // Draw performance overlay

        if (m_showPerfInfoHud )
        {
            cvfqt::PerformanceInfoHud hud;
            hud.addStrings(m_renderingSequence->performanceInfo());
            hud.addStrings(*m_mainCamera);
            hud.addString(QString("PaintCount: %1").arg(m_paintCounter++));
            hud.draw(&overlayPainter, width(), height());
        }

        // Convert the QImage into the cvf::TextureImage, 
        // handling vertical mirroring and (possible) byteswapping

        if (((int)m_overlayTextureImage->height()) != this->height() || ((int)m_overlayTextureImage->width() != this->width()))
        {
            m_overlayTextureImage->allocate(this->width(), this->height());
        }

        cvfqt::Utils::toTextureImage(m_overlayPaintingQImage, m_overlayTextureImage.p());
        
        m_overlayImage->setImage(m_overlayTextureImage.p());
        m_overlayImage->setPixelSize(cvf::Vec2ui(this->width(), this->height()));
    }
   
#if QT_VERSION >= 0x040600
    // Qt 4.6
    painter.beginNativePainting();
#endif

    if (isShadersSupported())
    {
        cvfqt::OpenGLContext::saveOpenGLState(myOglContext.p());
    }

    optimizeClippingPlanes();

    m_renderingSequence->removeRendering(m_comparisonMainRendering.p());
    if ( m_comparisonMainRendering->scene() )
    {
        m_renderingSequence->insertRendering( m_overlayItemsRendering.p(), m_comparisonMainRendering.p());
    }

    if ( m_poiVisualizationManager.notNull() )
    {
        m_poiVisualizationManager->update(m_navigationPolicy->pointOfInterest()); // Todo: Must be inserted in comparison scene as well, using the display offset
        m_mainRendering->scene()->addModel(m_poiVisualizationManager->model());
    }

    // Do normal drawing
    m_renderingSequence->render(myOglContext.p());
    CVF_CHECK_OGL(cvfOpenGLContext());

    if (  m_poiVisualizationManager.notNull() )
    {
        m_mainRendering->scene()->removeModel(m_poiVisualizationManager->model());
    }

    if (isShadersSupported())
    {
        cvfqt::OpenGLContext::restoreOpenGLState(myOglContext.p());
    }

#if QT_VERSION >= 0x040600
    // Qt 4.6
    painter.endNativePainting();
#endif

 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setDefaultPerspectiveNearPlaneDistance(double dist)
{
    m_defaultPerspectiveNearPlaneDistance = dist;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setMaxClipPlaneDistance(double dist)
{
    m_maxClipPlaneDistance = dist;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setView(const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection)
{
    if (m_navigationPolicy.notNull() && m_navigationPolicyEnabled)
    {
        m_navigationPolicy->setView(alongDirection, upDirection); 

        navigationPolicyUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::zoomAll()
{
    cvf::BoundingBox bb = m_mainRendering->boundingBox();
    if (!bb.isValid())
    {
      return;
    }

    cvf::Vec3d eye, vrp, up;
    m_mainCamera->toLookAt(&eye, &vrp, &up);

    cvf::Vec3d newEye = m_mainCamera->computeFitViewEyePosition(bb, vrp-eye, up, 0.9, m_cameraFieldOfViewYDeg, m_mainCamera->viewport()->aspectRatio());
    m_mainCamera->setFromLookAt(newEye, bb.center(), up);
    
    updateParallelProjectionHeightFromMoveZoom(bb.center());

    if (m_navigationPolicy.notNull()) m_navigationPolicy->setPointOfInterest(bb.center());

    navigationPolicyUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::addFrame(cvf::Scene* scene, bool isForComparisonView)
{
    appendAllStaticModelsToFrame(scene, isForComparisonView);

    if ( !isForComparisonView )
    {
        m_frameScenes.push_back(scene);
    }
    else
    {
        m_comparisonFrameScenes.push_back(scene);
    }

    m_animationControl->setNumFrames( static_cast<int>( frameCount() ) );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::removeAllFrames(bool isForComparisonView)
{
    if ( !isForComparisonView )
    {
        m_frameScenes.clear();
        m_mainRendering->setScene(m_mainScene.p());
    }
    else
    {
        m_comparisonFrameScenes.clear();
        m_comparisonMainRendering->setScene(m_comparisonMainScene.p());
    }

    m_animationControl->setNumFrames(static_cast<int>(frameCount()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::isAnimationActive()
{
    cvf::Scene* currentScene = m_mainRendering->scene();

    if (!currentScene)
    {
        return false;
    }

    if (m_mainScene.notNull() && m_mainScene.p() == currentScene)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::slotSetCurrentFrame(int frameIndex)
{
    if (m_frameScenes.size() == 0) return;

    int clampedFrameIndex = clampFrameIndex(frameIndex);

    //if (m_frameScenes.at(clampedFrameIndex) == nullptr) return;

    if (m_releaseOGLResourcesEachFrame)
    {
        releaseOGlResourcesForCurrentFrame();
    }

    if (m_frameScenes.size() > clampedFrameIndex &&  m_frameScenes.at(clampedFrameIndex) != nullptr )
    {
        m_mainRendering->setScene(m_frameScenes.at(clampedFrameIndex));
    }
    else 
    {
        m_mainRendering->setScene(nullptr);
    }
    if (m_comparisonFrameScenes.size() > clampedFrameIndex &&  m_comparisonFrameScenes.at(clampedFrameIndex) != nullptr )
    {
        m_comparisonMainRendering->setScene(m_comparisonFrameScenes.at(clampedFrameIndex));
    }
    else 
    {
        m_comparisonMainRendering->setScene(nullptr);
    }
    update();
}

void caf::Viewer::releaseOGlResourcesForCurrentFrame()
{
    if (isAnimationActive())
    {
        cvf::Scene* currentScene = m_mainRendering->scene();
        makeCurrent();
        cvf::uint modelCount = currentScene->modelCount();
        for (cvf::uint i = 0; i < modelCount; ++i)
        {
            cvf::Collection<cvf::Part> partCollection; 
            currentScene->model(i)->allParts(&partCollection);
            for (size_t pIdx = 0; pIdx < partCollection.size(); ++pIdx)
            {
                if (partCollection[pIdx].notNull() && partCollection[pIdx]->drawable())
                {
                    partCollection[pIdx]->drawable()->releaseBufferObjectsGPU();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::slotEndAnimation()
{
    if(m_releaseOGLResourcesEachFrame)
    {
        releaseOGlResourcesForCurrentFrame();
    }

    m_mainRendering->setScene(m_mainScene.p());

    update();
}

//--------------------------------------------------------------------------------------------------
/// This only updates the boundingboxes yet. Might want to do other things as well
//--------------------------------------------------------------------------------------------------
void caf::Viewer::updateCachedValuesInScene()
{
    if (m_mainScene.notNull())
    {
        cvf::uint midx;
        for (midx = 0; midx <  m_mainScene->modelCount() ; ++midx)
        {
            m_mainScene->model(midx)->updateBoundingBoxesRecursive();
        }
    }
    size_t sIdx;
    for (sIdx = 0; sIdx < m_frameScenes.size(); ++sIdx)
    {
        cvf::uint midx;
        for (midx = 0; midx <  m_frameScenes[sIdx]->modelCount() ; ++midx)
        {
             m_frameScenes[sIdx]->model(midx)->updateBoundingBoxesRecursive();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::isShadersSupported()
{
    QGLFormat::OpenGLVersionFlags flags = QGLFormat::openGLVersionFlags();
    bool hasOpenGL_2_0 = QGLFormat::OpenGL_Version_2_0 & flags;

    if (hasOpenGL_2_0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage caf::Viewer::snapshotImage()
{
    // Qt5 : Call paintEvent() manually to make sure invisible widgets are rendered properly
    // If this call is skipped, we get an assert in cvf::FramebufferObject::bind()
    paintEvent(nullptr);

    QImage image;
    if (m_offscreenFbo.notNull() && m_offscreenViewportWidth > 0 && m_offscreenViewportHeight > 0)
    {
        cvf::ref<cvf::OpenGLContext> myOglContext = cvfOpenGLContext();

        m_offscreenFbo->bind(myOglContext.p());

        GLint iOldPackAlignment = 0;
        glGetIntegerv(GL_PACK_ALIGNMENT, &iOldPackAlignment);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        CVF_CHECK_OGL(myOglContext.p());

        cvf::UByteArray arr(3 * m_offscreenViewportWidth * m_offscreenViewportHeight);

        glReadPixels(0, 0, static_cast<GLsizei>(m_offscreenViewportWidth), static_cast<GLsizei>(m_offscreenViewportHeight), GL_RGB, GL_UNSIGNED_BYTE, arr.ptr());
        CVF_CHECK_OGL(myOglContext.p());

        glPixelStorei(GL_PACK_ALIGNMENT, iOldPackAlignment);
        CVF_CHECK_OGL(myOglContext.p());

        cvf::FramebufferObject::useDefaultWindowFramebuffer(myOglContext.p());

        cvf::TextureImage texImage;
        texImage.setFromRgb(arr.ptr(), m_offscreenViewportWidth, m_offscreenViewportHeight);

        image = cvfqt::Utils::toQImage(texImage);
    }
    else
    {
        // Code moved from RimView::snapshotWindowContent()

        GLint currentReadBuffer;
        glGetIntegerv(GL_READ_BUFFER, &currentReadBuffer);

        glReadBuffer(GL_FRONT);
        image = this->grabFrameBuffer();

        glReadBuffer(currentReadBuffer);
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Scene* caf::Viewer::frame(size_t frameIndex, bool isForComparisonView)
{
    if ( !isForComparisonView )
    {
        if ( frameIndex < m_frameScenes.size() )
            return m_frameScenes[frameIndex].p();
        else
            return nullptr;
    }
    else
    {
        if ( frameIndex < m_comparisonFrameScenes.size() )
            return m_comparisonFrameScenes[frameIndex].p();
        else
            return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function used to write out the name of all parts in a rendering sequence
//--------------------------------------------------------------------------------------------------
void caf::Viewer::debugShowRenderingSequencePartNames()
{
    qDebug() << "\n\n";
    size_t globalPartCount = 0;

    cvf::uint rIdx = m_renderingSequence->renderingCount();
    for (rIdx = 0; rIdx < m_renderingSequence->renderingCount(); rIdx++)
    {
        cvf::Rendering* rendering = m_renderingSequence->rendering(rIdx);
        if (rendering && rendering->scene())
        {
            cvf::uint mIdx;
            for (mIdx = 0; mIdx < rendering->scene()->modelCount(); mIdx++)
            {
                cvf::Model* model = rendering->scene()->model(mIdx);
                if (model)
                {
                    cvf::Collection<cvf::Part> parts;
                    model->allParts(&parts);

                    size_t pIdx;
                    for (pIdx = 0; pIdx < parts.size(); pIdx++)
                    {
                        cvf::Part* part = parts.at(pIdx);

                        qDebug() << QString("%1").arg(globalPartCount++) << cvfqt::Utils::toQString(part->name());
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::enableNavigationPolicy(bool enable)
{
    m_navigationPolicyEnabled = enable;
    if (enable && m_navigationPolicy.notNull()) m_navigationPolicy->init(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize caf::Viewer::sizeHint() const
{
    return QSize(500, 400);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::enableForcedImmediateMode(bool enable)
{
    cvf::uint rIdx = m_renderingSequence->renderingCount();
    for (rIdx = 0; rIdx < m_renderingSequence->renderingCount(); rIdx++)
    {
        cvf::Rendering* rendering = m_renderingSequence->rendering(rIdx);
        if (rendering && rendering->scene())
        {
            rendering->renderEngine()->enableForcedImmediateMode(enable);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::Viewer::currentFrameIndex() const
{
    if (m_animationControl)
    {
        int clampedFrameIndex = clampFrameIndex(m_animationControl->currentFrame());
        return clampedFrameIndex;
    }
    else return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::isOverlayPaintingEnabled() const
{
    return m_isOverlayPaintingEnabled;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::enableOverlayPainting(bool val)
{
    m_isOverlayPaintingEnabled = val;
    updateOverlayImagePresence();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Rendering* caf::Viewer::overlayItemsRendering()
{
    return m_overlayItemsRendering.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::updateOverlayImagePresence()
{
    if (m_isOverlayPaintingEnabled || m_showPerfInfoHud)
    {
         m_overlayItemsRendering->addOverlayItem(m_overlayImage.p());
    }
    else
    {
        m_overlayItemsRendering->removeOverlayItem(m_overlayImage.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// Create a virtual method so it is possible to override this function in derived classes
//--------------------------------------------------------------------------------------------------
void caf::Viewer::navigationPolicyUpdate()
{
    update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::addStaticModelOnce(cvf::Model* model, bool isForComparisonView)
{
    if ( !isForComparisonView )
    {
        if ( m_staticModels.contains(model) ) return;

        m_staticModels.push_back(model);
    }
    else
    {
        if ( m_comparisonStaticModels.contains(model) ) return;

        m_comparisonStaticModels.push_back(model);
    }

    appendModelToAllFrames(model, isForComparisonView);

    updateCachedValuesInScene();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::removeStaticModel(cvf::Model* model)
{
    removeModelFromAllFrames(model);
    
    m_staticModels.erase(model);
    m_comparisonStaticModels.erase(model);

    updateCachedValuesInScene();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::removeAllStaticModels()
{
    for (size_t i = 0; i < m_staticModels.size(); i++)
    {
        removeModelFromAllFrames(m_staticModels.at(i));
    }
    
    for (size_t i = 0; i < m_comparisonStaticModels.size(); i++)
    {
        removeModelFromAllFrames(m_comparisonStaticModels.at(i));
    }

    m_staticModels.clear();
    m_comparisonStaticModels.clear();

    updateCachedValuesInScene();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setEnableMask(unsigned int mask, bool isForComparisonView /*= false */)
{
    if (!isForComparisonView)
    {
        m_mainRendering->setEnableMask(mask);
    }
    else
    {
        m_comparisonMainRendering->setEnableMask(mask);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::removeModelFromAllFrames(cvf::Model* model)
{
    for (size_t i = 0; i < m_frameScenes.size(); i++)
    {
        cvf::Scene* scene = m_frameScenes.at(i);

        scene->removeModel(model);
    }

    if (m_mainScene.notNull())
    {
        m_mainScene->removeModel(model);
    }

    for (size_t i = 0; i < m_comparisonFrameScenes.size(); i++)
    {
        cvf::Scene* scene = m_comparisonFrameScenes.at(i);

        scene->removeModel(model);
    }

    if (m_comparisonMainScene.notNull())
    {
        m_comparisonMainScene->removeModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::appendModelToAllFrames(cvf::Model* model, bool isForComparisonView )
{
    if ( !isForComparisonView )
    {
        for ( size_t i = 0; i < m_frameScenes.size(); i++ )
        {
            cvf::Scene* scene = m_frameScenes.at(i);

            scene->addModel(model);
        }

        if ( m_mainScene.notNull() )
        {
            m_mainScene->addModel(model);
        }
    }
    else
    {
        for ( size_t i = 0; i < m_comparisonFrameScenes.size(); i++ )
        {
            cvf::Scene* scene = m_comparisonFrameScenes.at(i);

            scene->addModel(model);
        }

        if ( m_comparisonMainScene.notNull() )
        {
            m_comparisonMainScene->addModel(model);
        }
    }
}   

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::appendAllStaticModelsToFrame(cvf::Scene* scene, bool isForComparisonView )
{
    if (!scene) return;

    if ( !isForComparisonView )
    {
        for ( size_t i = 0; i < m_staticModels.size(); i++ )
        {
            scene->addModel(m_staticModels.at(i));
        }
    }
    else
    {
        for ( size_t i = 0; i < m_comparisonStaticModels.size(); i++ )
        {
            scene->addModel(m_comparisonStaticModels.at(i));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OverlayItem* caf::Viewer::overlayItem(int winPosX, int winPosY)
{
    if (m_overlayItemsRendering.isNull()) return nullptr;

    int translatedMousePosX = winPosX;
    int translatedMousePosY = height() - winPosY;

    return m_overlayItemsRendering->overlayItemFromWindowCoordinates(translatedMousePosX, translatedMousePosY);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::enableParallelProjection(bool enableOrtho)
{
    if (enableOrtho && m_mainCamera->projection() == cvf::Camera::PERSPECTIVE)
    {
        cvf::Vec3d pointOfInterest;

        if (m_navigationPolicy.isNull() || !m_navigationPolicyEnabled)
        {
            using namespace cvf;

            Vec3d eye, vrp, up;
            m_mainCamera->toLookAt(&eye, &vrp, &up);

            Vec3d eyeToFocus = pointOfInterest - eye;
            Vec3d camDir = vrp - eye;
            camDir.normalize();

            double distToFocusPlane =  0.5*(m_mainCamera->farPlane() - m_mainCamera->nearPlane());
            pointOfInterest = camDir *distToFocusPlane;
        }
        else
        {
            pointOfInterest = m_navigationPolicy->pointOfInterest();
        }
        m_mainCamera->setProjectionAsOrtho(1.0, m_mainCamera->nearPlane(), m_mainCamera->farPlane());
        this->updateParallelProjectionHeightFromMoveZoom(pointOfInterest);

        // Set a fake directional light by putting the point far away from the scene
        float sceneDepth = m_mainCamera->farPlane() -  m_mainCamera->nearPlane();
        this->m_renderingSequence->setDefaultFFLightPositional(-m_parallelProjectionLightDirection* 10*sceneDepth);
        m_globalUniformSet->setHeadLightPosition(-m_parallelProjectionLightDirection* 10*sceneDepth);

        this->update();
    }
    else if (!enableOrtho && m_mainCamera->projection() == cvf::Camera::ORTHO)
    {
        // We currently expect all the navigation policies to do walk-based navigation and not fiddle with the field of view
        // so we do not need to update the camera position based on orthoHeight and fieldOfView. 
        // We assume the camera is in a sensible position.

        // Set dummy near and far plane. These will be updated by the optimize clipping planes
        m_mainCamera->setProjectionAsPerspective(m_cameraFieldOfViewYDeg, 0.1, 1.0);

        this->m_renderingSequence->setDefaultFFLightPositional(cvf::Vec3f(0.5, 5.0, 7.0));
        m_globalUniformSet->setHeadLightPosition(cvf::Vec3f(0.5, 5.0, 7.0));

        this->update();
    }
}

//--------------------------------------------------------------------------------------------------
/// Direction in camera coordinates default is (0, 0, -1) directly from behind
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setParallelProjectionHeadLightDirection(const cvf::Vec3f& direction)
{
    m_parallelProjectionLightDirection = direction;

    float sceneDepth = m_mainCamera->farPlane() -  m_mainCamera->nearPlane();
    this->m_renderingSequence->setDefaultFFLightPositional(-m_parallelProjectionLightDirection* 10*sceneDepth);
    m_globalUniformSet->setHeadLightPosition(-m_parallelProjectionLightDirection* 10*sceneDepth);

    this->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setPointOfInterestVisualizer(PointOfInterestVisualizer* poiVisualizer)
{
    m_poiVisualizationManager = poiVisualizer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

double calculateOrthoHeight(double perspectiveViewAngleYDeg, double focusPlaneDist)
{
   return 2 * (cvf::Math::tan( cvf::Math::toRadians(0.5 * perspectiveViewAngleYDeg) ) * focusPlaneDist);
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

double calculateDistToPlaneOfOrthoHeight(double perspectiveViewAngleYDeg, double orthoHeight)
{
   return orthoHeight / (2 * (cvf::Math::tan( cvf::Math::toRadians(0.5 * perspectiveViewAngleYDeg) )));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

double distToPlaneOfInterest(const cvf::Camera* camera, const cvf::Vec3d& pointOfInterest)
{
    using namespace cvf;
    CVF_ASSERT(camera);

    Vec3d eye, vrp, up;
    camera->toLookAt(&eye, &vrp, &up);

    Vec3d camDir = vrp - eye;
    camDir.normalize();

    Vec3d eyeToFocus = pointOfInterest - eye;
    double distToFocusPlane = eyeToFocus*camDir;

    return distToFocusPlane;
}

//--------------------------------------------------------------------------------------------------
/// Update the ortho projection view height from a walk based camera manipulation.
/// Using pointOfInterest, the perspective Y-field Of View along with the camera position
//--------------------------------------------------------------------------------------------------
void caf::Viewer::updateParallelProjectionHeightFromMoveZoom(const cvf::Vec3d& pointOfInterest)
{
    using namespace cvf;
    cvf::Camera* camera = m_mainCamera.p();

    if (!camera || camera->projection() != Camera::ORTHO) return;

    // Negative distance can occur. If so, do not set a negative ortho.

    double distToFocusPlane = cvf::Math::abs( distToPlaneOfInterest(camera, pointOfInterest));    

    double orthoHeight = calculateOrthoHeight(m_cameraFieldOfViewYDeg, distToFocusPlane);

    camera->setProjectionAsOrtho(orthoHeight, camera->nearPlane(), camera->farPlane());
}

//--------------------------------------------------------------------------------------------------
/// Update the camera eye position from point of interest, keeping the ortho height fixed and in sync 
/// with distToPlaneOfInterest  from a walk based camera manipulation in ortho projection.
//--------------------------------------------------------------------------------------------------
void caf::Viewer::updateParallelProjectionCameraPosFromPointOfInterestMove(const cvf::Vec3d& pointOfInterest)
{
    using namespace cvf;
    cvf::Camera* camera = m_mainCamera.p();

    if (!camera || camera->projection() != Camera::ORTHO) return;

    
    double orthoHeight = camera->frontPlaneFrustumHeight();
    //Trace::show(String::number(orthoHeight));

    double neededDistToFocusPlane = calculateDistToPlaneOfOrthoHeight(m_cameraFieldOfViewYDeg, orthoHeight);

    Vec3d eye, vrp, up;
    camera->toLookAt(&eye, &vrp, &up);
    Vec3d camDir = vrp - eye;
    camDir.normalize();

    double existingDistToFocusPlane = distToPlaneOfInterest(camera, pointOfInterest);

    Vec3d newEye = eye + (existingDistToFocusPlane - neededDistToFocusPlane) * camDir;

    //Trace::show(String::number(newEye.x()) + ", " + String::number(newEye.y()) + ", " +String::number(newEye.z()));
    camera->setFromLookAt(newEye, newEye + 10.0*camDir, up);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::Viewer::clampFrameIndex(int frameIndex) const
{
    int clampedFrameIndex = frameIndex;
    int frameCountInt = static_cast<int>(frameCount());

    if (clampedFrameIndex >= frameCountInt)
    {
        clampedFrameIndex = frameCountInt - 1;
    }
    else if (clampedFrameIndex < 0)
    {
        clampedFrameIndex = 0;
    }

    return clampedFrameIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::copyCameraView(cvf::Camera* srcCamera, cvf::Camera* dstCamera)
{
    if (srcCamera->projection() == cvf::Camera::PERSPECTIVE)
    {
        dstCamera->setProjectionAsPerspective(srcCamera->fieldOfViewYDeg(), srcCamera->nearPlane(), srcCamera->farPlane());
    }
    else
    {
        dstCamera->setProjectionAsOrtho(srcCamera->frontPlaneFrustumHeight(), srcCamera->nearPlane(), srcCamera->farPlane());
    }

    dstCamera->setViewMatrix(srcCamera->viewMatrix());
}
