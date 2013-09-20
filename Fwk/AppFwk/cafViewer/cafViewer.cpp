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

#include "cvfCamera.h"
#include "cvfRendering.h"
#include "cvfRenderSequence.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfRenderQueueSorter.h"
#include "cvfScene.h"
#include "cvfModel.h"
#include "cvfTextureImage.h"
#include "cvfOverlayImage.h"

#include "cvfqtOpenGLContext.h"

#include "cvfRay.h"
#include "cvfPart.h"
#include "cvfDrawable.h"
#include "cvfDrawableGeo.h"
#include "cvfTransform.h"
#include "cvfRayIntersectSpec.h"
#include "cvfHitItemCollection.h"

#include "cvfDebugTimer.h"
#include "cvfqtPerformanceInfoHud.h"
#include "cvfqtUtils.h"

#include "cafNavigationPolicy.h"
#include "cafCadNavigation.h"
#include "cafFrameAnimationControl.h"

#include <QInputEvent>
#include <QHBoxLayout>
#include <QDebug>

std::list<caf::Viewer*> caf::Viewer::sm_viewers;
cvf::ref<cvf::OpenGLContextGroup> caf::Viewer::sm_openGLContextGroup;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::Viewer::Viewer(const QGLFormat& format, QWidget* parent)
    :   caf::OpenGLWidget(contextGroup(), format, new QWidget(parent), sharedWidget()),
    m_navigationPolicy(NULL),
    m_minNearPlaneDistance(0.05),
    m_maxFarPlaneDistance(cvf::UNDEFINED_DOUBLE),
    m_releaseOGLResourcesEachFrame(false),
    m_paintCounter(0),
    m_navigationPolicyEnabled(true),
    m_isOverlyPaintingEnabled(true)
{
    m_layoutWidget = parentWidget();

    QHBoxLayout* layout = new QHBoxLayout(m_layoutWidget);

    layout->addWidget(this);
    layout->setContentsMargins(0, 0, 0, 0);

    setAutoFillBackground(false);
    setMouseTracking(true);

    // Needed to get keystrokes
    setFocusPolicy(Qt::ClickFocus);

    m_mainCamera = new cvf::Camera;
    m_mainCamera->setFromLookAt(cvf::Vec3d(0,0,-1), cvf::Vec3d(0,0,0), cvf::Vec3d(0,1,0));
    m_renderingSequence = new cvf::RenderSequence();
    m_mainRendering = new cvf::Rendering();

    m_animationControl = new caf::FrameAnimationControl(this);
    connect(m_animationControl, SIGNAL(changeFrame(int)), SLOT(slotSetCurrentFrame(int)));
    connect(m_animationControl, SIGNAL(endAnimation()), SLOT(slotEndAnimation()));


    this->setNavigationPolicy(new caf::CadNavigation);

    m_overlayTextureImage = new cvf::TextureImage;
    m_overlayImage = new cvf::OverlayImage(m_overlayTextureImage.p());
    m_overlayImage->setBlending(cvf::OverlayImage::TEXTURE_ALPHA);
    m_overlayImage->setUnmanagedPosition(cvf::Vec2i(0,0));

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
/// This method is supposed to setup the contents of the main rendering
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setupMainRendering()
{
    m_mainRendering->setCamera(m_mainCamera.p());
    m_mainRendering->setRenderQueueSorter(new cvf::RenderQueueSorterBasic(cvf::RenderQueueSorterBasic::EFFECT_ONLY));

    // Set fixed function rendering if QGLFormat does not support directRendering
    if (!this->format().directRendering())
    {
        m_mainRendering->renderEngine()->enableForcedImmediateMode(true);
    }

    updateOverlayImagePresence();
}

//--------------------------------------------------------------------------------------------------
/// This method is supposed to assemble the rendering sequence (cvf::RenderSequence) based on
/// the Viewers renderings. THe ViewerBase has only one rendering. The m_mainRendering
/// Reimplement to build more advanced rendering sequences
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setupRenderingSequence()
{
    m_renderingSequence->addRendering(m_mainRendering.p());  

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

    return NULL;
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
/// Set the scene to be rendered when the animation is inactive (Stopped)
//--------------------------------------------------------------------------------------------------
void  caf::Viewer::setMainScene(cvf::Scene* scene)
{
    m_mainScene = scene;
    m_mainRendering->setScene(scene);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Scene* caf::Viewer::mainScene()
{
    return m_mainScene.p();
}

//--------------------------------------------------------------------------------------------------
/// Return the currently rendered scene
//--------------------------------------------------------------------------------------------------
cvf::Scene* caf::Viewer::currentScene()
{
    return m_renderingSequence->firstRendering()->scene();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::updateCamera(int width, int height)
{
    if (width < 1 || height < 1) return;

    m_mainCamera->viewport()->set(0, 0, width, height);

    if (m_mainCamera->projection() == cvf::Camera::PERSPECTIVE)
    {
        m_mainCamera->setProjectionAsPerspective(40, m_mainCamera->nearPlane(), m_mainCamera->farPlane());
    }
    else
    {
        cvf::BoundingBox bb = m_renderingSequence->boundingBox();
        if (bb.isValid())
        {
            m_mainCamera->setProjectionAsOrtho(bb.extent().length(), m_mainCamera->nearPlane(), m_mainCamera->farPlane());
        }
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
    if (m_mainCamera->projection() == cvf::Camera::PERSPECTIVE)
    {
        cvf::BoundingBox bb = m_renderingSequence->boundingBox();
        if (!bb.isValid()) return;

        cvf::Vec3d eye, vrp, up;
        m_mainCamera->toLookAt(&eye, &vrp, &up);

        cvf::Vec3d viewdir = (vrp - eye).getNormalized();

        double distEyeBoxCenterAlongViewDir = (bb.center() - eye)*viewdir;

        double farPlaneDist = distEyeBoxCenterAlongViewDir + bb.radius();
        farPlaneDist = CVF_MIN(farPlaneDist, m_maxFarPlaneDistance);

        double nearPlaneDist = distEyeBoxCenterAlongViewDir - bb.radius();
        if (nearPlaneDist < m_minNearPlaneDistance) nearPlaneDist = m_minNearPlaneDistance;
        if ( m_navigationPolicy.notNull())
        {
            double pointOfInterestDist = (eye - m_navigationPolicy->pointOfInterest()).length();
            nearPlaneDist = CVF_MIN( nearPlaneDist, pointOfInterestDist*0.2);
        }

        if (farPlaneDist <= nearPlaneDist) farPlaneDist = nearPlaneDist + 1.0;

        m_mainCamera->setProjectionAsPerspective(m_mainCamera->fieldOfViewYDeg(), nearPlaneDist, farPlaneDist);
    }
}

//--------------------------------------------------------------------------------------------------
/// Forward all events classified as QInputEvent to the navigation policy
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::event(QEvent* e)
{
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
bool caf::Viewer::rayPick(int winPosX, int winPosY, cvf::HitItemCollection* pickedPoints)
{
    CVF_ASSERT(m_mainRendering.notNull());

    int translatedMousePosX = winPosX;
    int translatedMousePosY = height() - winPosY;

    cvf::ref<cvf::RayIntersectSpec> ris = m_mainRendering->rayIntersectSpecFromWindowCoordinates(translatedMousePosX, translatedMousePosY);
    if (ris.notNull())
    {
        bool retVal = m_mainRendering->rayIntersect(*ris, pickedPoints);

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

    if (m_isOverlyPaintingEnabled || m_showPerfInfoHud)
    {
        // Set up image to draw to, and painter 
        if (m_overlayPaintingQImage.size() != this->size())
        {
            m_overlayPaintingQImage = QImage(this->size(), QImage::Format_ARGB32);
        }

        m_overlayPaintingQImage.fill(Qt::transparent);
        QPainter overlyPainter(&m_overlayPaintingQImage);

        // Call virtual method to allow subclasses to paint on the OpenGlCanvas

        if (m_isOverlyPaintingEnabled)
        {
            this->paintOverlayItems(&overlyPainter); 
        }

        // Draw performance overlay

        if (m_showPerfInfoHud )
        {
            cvfqt::PerformanceInfoHud hud;
            hud.addStrings(m_renderingSequence->performanceInfo());
            hud.addStrings(*m_mainCamera);
            hud.addString(QString("PaintCount: %1").arg(m_paintCounter++));
            hud.draw(&overlyPainter, width(), height());
        }

        // Convert the QImage into the cvf::TextureImage, 
        // handling vertical mirroring and (possible) byteswapping

        if (((int)m_overlayTextureImage->height()) !=  this->height() || ((int)m_overlayTextureImage->width() !=  this->width()))
        {
            m_overlayTextureImage->allocate(this->width(), this->height());
        }

        int height =  m_overlayTextureImage->height();
        int width =  m_overlayTextureImage->width();

#pragma omp parallel for
        for (int y = 0 ; y < height; ++y)
        {
            int negy =  height - 1 - y;
            for (int x = 0 ; x < width; ++x)
            {
                // Should probably do direct conversion on byte level. Would be faster
                // QImage.bits() and cvf::TextureImage.ptr() (casting away const)
                QRgb qtRgbaVal = m_overlayPaintingQImage.pixel(x, negy);
                cvf::Color4ub cvfRgbVal(qRed(qtRgbaVal), qGreen(qtRgbaVal), qBlue(qtRgbaVal), qAlpha(qtRgbaVal)); 
                m_overlayTextureImage->setPixel(x, y, cvfRgbVal);
            }
        }

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

    // Do normal drawing
    m_renderingSequence->render(myOglContext.p());
    CVF_CHECK_OGL(cvfOpenGLContext());

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
void caf::Viewer::setMinNearPlaneDistance(double dist)
{
    m_minNearPlaneDistance = dist;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setMaxFarPlaneDistance(double dist)
{
    m_maxFarPlaneDistance = dist;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::setView(const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection)
{
    if (m_navigationPolicy.notNull())
    {
        m_navigationPolicy->setView(alongDirection, upDirection); 
        update();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::zoomAll()
{
    cvf::BoundingBox bb;

    cvf::Scene* scene = m_renderingSequence->firstRendering()->scene();
    if (scene)
    {
        bb = scene->boundingBox();
    }

    if (!bb.isValid())
    {
      return;
    }

    cvf::Vec3d eye, vrp, up;
    m_mainCamera->toLookAt(&eye, &vrp, &up);

    m_mainCamera->fitView(bb, vrp-eye, up);
    update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::addFrame(cvf::Scene* scene)
{
    m_frameScenes.push_back(scene);
    m_animationControl->setNumFrames(static_cast<int>(m_frameScenes.size()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::removeAllFrames()
{
    m_frameScenes.clear();
    m_animationControl->setNumFrames(0);
    m_renderingSequence->firstRendering()->setScene(m_mainScene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::isAnimationActive()
{
    cvf::Scene* currentScene = m_renderingSequence->firstRendering()->scene();

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
    if (frameIndex < 0 || static_cast<size_t>(frameIndex) >= m_frameScenes.size() || m_frameScenes.at(frameIndex) == NULL) return;


    if(m_releaseOGLResourcesEachFrame)
    {
        releaseOGlResourcesForCurrentFrame();
    }

    m_renderingSequence->firstRendering()->setScene(m_frameScenes.at(frameIndex));


    update();
}

void caf::Viewer::releaseOGlResourcesForCurrentFrame()
{
    if (isAnimationActive())
    {
        cvf::Scene* currentScene = m_renderingSequence->firstRendering()->scene();
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

    m_renderingSequence->firstRendering()->setScene(m_mainScene.p());
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
cvf::Scene* caf::Viewer::frame(size_t frameIndex)
{
    if (frameIndex < m_frameScenes.size())
        return m_frameScenes[frameIndex].p();
    else
        return NULL;
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

void caf::Viewer::enableForcedImmediateMode(bool enable)
{
    m_mainRendering->renderEngine()->enableForcedImmediateMode(enable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::Viewer::currentFrameIndex()
{
    if (m_animationControl) return m_animationControl->currentFrame();
    else return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::Viewer::isOverlyPaintingEnabled() const
{
    return m_isOverlyPaintingEnabled;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::enableOverlyPainting(bool val)
{
    m_isOverlyPaintingEnabled = val;
    updateOverlayImagePresence();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::Viewer::updateOverlayImagePresence()
{
    if (m_isOverlyPaintingEnabled || m_showPerfInfoHud)
    {
         m_mainRendering->addOverlayItem(m_overlayImage.p(), cvf::OverlayItem::UNMANAGED, cvf::OverlayItem::VERTICAL);
    }
    else
    {
         m_mainRendering->removeOverlayItem(m_overlayImage.p());
    }
}

