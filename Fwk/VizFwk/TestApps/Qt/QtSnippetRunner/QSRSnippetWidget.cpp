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


#include "QSRStdInclude.h"
#include "QSRSnippetWidget.h"
#include "QSRTranslateEvent.h"

#include "cvfqtPerformanceInfoHud.h"
#include "cvfqtOpenGLContext.h"

#include <math.h>

#include <QMouseEvent>

using cvfu::TestSnippet;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRSnippetWidget::QSRSnippetWidget(TestSnippet* snippet, cvf::OpenGLContextGroup* contextGroup, const QGLFormat& format, QWidget* parent)
:   cvfqt::OpenGLWidget(contextGroup, format, parent),
    m_drawHUD(false),
    m_lastSetRenderMode(DrawableGeo::VERTEX_ARRAY),
    m_enableMultisampleWhenDrawing(false)
{
    CVF_ASSERT(snippet);

    // Needed to get keystrokes
    setFocusPolicy(Qt::ClickFocus);

    // To avoid  QPainter::begin() painting background
    setAutoFillBackground(false);

    // Create and wire, but don't start the timer
    m_animationUpdateTimer = new QTimer(this);
    connect(m_animationUpdateTimer, SIGNAL(timeout()), SLOT(slotAnimationUpdateTimer()));

    m_snippet = snippet;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRSnippetWidget::~QSRSnippetWidget()
{
    makeCurrent();

    if (m_snippet.notNull())
    {
        m_snippet->destroySnippet();
        m_snippet = NULL;
    }

    cvfShutdownOpenGLContext();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::TestSnippet* QSRSnippetWidget::snippet()
{
    return m_snippet.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::setRenderMode(DrawableGeo::RenderMode mode)
{
    m_lastSetRenderMode = mode;

    Collection<Part> allParts;

    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();

    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

        Scene* scene = rendering->scene();
        CVF_ASSERT(scene);

        scene->allParts(&allParts);
    }

    size_t numParts = allParts.size();
    size_t partIdx;
    for (partIdx = 0; partIdx < numParts; partIdx++)
    {
        Part* part = allParts.at(partIdx);

        cvf::uint lod;
		for (lod = 0; lod < cvf::Part::MAX_NUM_LOD_LEVELS; lod++)
        {
            DrawableGeo* drawableGeo = dynamic_cast<DrawableGeo*>(part->drawable(lod));
            if (drawableGeo)
            {
                drawableGeo->setRenderMode(m_lastSetRenderMode);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::enableForcedImmediateMode(bool enable)
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numRenderings = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numRenderings; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

        cvf::RenderEngine* engine = rendering->renderEngine();
        CVF_ASSERT(engine);

        engine->enableForcedImmediateMode(enable);
    }    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DrawableGeo::RenderMode QSRSnippetWidget::renderMode() const
{
    return m_lastSetRenderMode;    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRSnippetWidget::isForcedImmediateModeEnabled() const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    if (numPasses < 1) return false;

    return renderSeq->firstRendering()->renderEngine()->isForcedImmediateModeEnabled();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::enablePerfInfoHUD(bool drawHUD)
{
    m_drawHUD = drawHUD;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRSnippetWidget::isPerfInfoHudEnabled() const
{
    return m_drawHUD;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::enableViewFrustumCulling(bool enable)
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

        rendering->cullSettings()->enableViewFrustumCulling(enable);
    }     
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRSnippetWidget::isViewFrustumCullingEnabled() const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    if (numPasses < 1) return false;

    return renderSeq->firstRendering()->cullSettings()->isViewFrustumCullingEnabled();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::enablePixelSizeCulling(bool enable)
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

        rendering->cullSettings()->enablePixelSizeCulling(enable);
    }     
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRSnippetWidget::isPixelSizeCullingEnabled() const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    if (numPasses < 1) return false;

    return renderSeq->rendering(0)->cullSettings()->isPixelSizeCullingEnabled();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::setPixelSizeCullingThreshold(double threshold)
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

        rendering->cullSettings()->setPixelSizeCullingAreaThreshold(threshold);
    }     
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double QSRSnippetWidget::pixelSizeCullingThreshold() const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    if (numPasses < 1) return 0.0;

    return renderSeq->firstRendering()->cullSettings()->pixelSizeCullingAreaThreshold();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::enableItemCountUpdates(bool enable)
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

		cvf::RenderEngine* engine = rendering->renderEngine();
        CVF_ASSERT(engine);

        engine->enableItemCountUpdate(enable);
    }    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRSnippetWidget::isItemCountUpdatesEnabled() const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    if (numPasses < 1) return false;

    return renderSeq->firstRendering()->renderEngine()->isItemCountUpdateEnabled();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::setRenderDrawablesDisabled(bool disable)
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

		cvf::RenderEngine* engine = rendering->renderEngine();
        CVF_ASSERT(engine);

        engine->disableRenderDrawables(disable);
    }    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRSnippetWidget::isRenderDrawablesDisabled() const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    if (numPasses < 1) return false;

    return renderSeq->firstRendering()->renderEngine()->isRenderDrawableDisabled();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::setApplyEffectsDisabled(bool disable)
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

		cvf::RenderEngine* engine = rendering->renderEngine();
        CVF_ASSERT(engine);

        engine->disableApplyEffects(disable);
    }    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRSnippetWidget::isApplyEffectsDisabled() const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    if (numPasses < 1) return false;

    return renderSeq->firstRendering()->renderEngine()->isApplyEffectsDisabled();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::convertDrawablesToShort()
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    int numPasses = renderSeq->renderingCount();
    int i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = renderSeq->rendering(i);
        CVF_ASSERT(rendering);

        Scene* scene = rendering->scene();
        CVF_ASSERT(scene);

        int numModels = scene->modelCount();
        int j;
        for (j = 0; j < numModels; j++)
        {
            Model* model = scene->model(j);

            Collection<Part> partCollection;
            model->allParts(&partCollection);

            size_t numParts = partCollection.size();
            size_t i;
            for (i = 0; i < numParts; i++)
            {
                DrawableGeo* drawable = dynamic_cast<DrawableGeo*>(partCollection.at(i)->drawable());
                if (drawable)
                {
                    drawable->convertFromUIntToUShort();
                }
            }
        }
    }    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::showModelStatistics()
{
    RenderSequence* renderSeq = m_snippet->renderSequence();
    CVF_ASSERT(renderSeq);

    size_t totPartCount = 0;
    size_t totFaceCount = 0;

    int numRenderings = renderSeq->renderingCount();
    int ir;
    for (ir = 0; ir < numRenderings; ir++)
    {
        Rendering* rendering = renderSeq->rendering(ir);
        CVF_ASSERT(rendering);

        Scene* scene = rendering->scene();
        CVF_ASSERT(scene);

        int numModels = scene->modelCount();
        int im;
        for (im = 0; im < numModels; im++)
        {
            Model* model = scene->model(im);

            Collection<Part> allParts;
            model->allParts(&allParts);

            size_t numParts = allParts.size();
            size_t ip;
            for (ip = 0; ip < numParts; ip++)
            {
                Part* part = allParts[ip].p();
                Drawable* drawable = part->drawable();

                totPartCount++;
                totFaceCount += drawable->faceCount();
            }
        }
    }    

    cvf::Trace::show("Total number of parts: %ld", totPartCount);
    cvf::Trace::show("Total number of faces: %ld", totFaceCount);
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::initializeGL()
{
    CVF_ASSERT(m_snippet.notNull());

    cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    CVF_ASSERT(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

    bool bInitOK = m_snippet->initializeSnippet(currentOglContext);
    CVF_ASSERT(bInitOK);

    CVF_CHECK_OGL(currentOglContext);

    CVF_ASSERT(m_animationUpdateTimer);
    m_animationUpdateTimer->start(1);
    m_currAnimationTime.restart();
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::resizeGL(int width, int height)
{
    CVF_ASSERT(m_snippet.notNull());

    cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    CVF_ASSERT(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

    m_snippet->onResizeEvent(width, height);

    CVF_CHECK_OGL(currentOglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::slotAnimationUpdateTimer()
{
    if (m_snippet.isNull()) return;

    double animTime = m_currAnimationTime.time();
    cvfu::PostEventAction postEventAction = cvfu::NONE;
    m_snippet->onUpdateAnimation(animTime, &postEventAction);
    if (postEventAction == cvfu::REDRAW)
    {
        update();
    }
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::paintEvent(QPaintEvent* /*event*/)
{
    CVF_ASSERT(m_snippet.notNull());

    makeCurrent();

    cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    CVF_ASSERT(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

    QPainter painter(this);

#if QT_VERSION >= 0x040600
    // Qt 4.6
    painter.beginNativePainting();
    CVF_CHECK_OGL(currentOglContext);
#endif

    
	cvfqt::OpenGLContext::saveOpenGLState(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

    if (m_enableMultisampleWhenDrawing)
    {
        glEnable(GL_MULTISAMPLE);
    }

    cvfu::PostEventAction postEventAction = cvfu::NONE;
    m_snippet->onPaintEvent(&postEventAction);
    CVF_CHECK_OGL(currentOglContext);

    if (m_enableMultisampleWhenDrawing)
    {
        glDisable(GL_MULTISAMPLE);
    }

    cvfqt::OpenGLContext::restoreOpenGLState(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

    if (postEventAction == cvfu::REDRAW)
    {
        update();
    }


#if QT_VERSION >= 0x040600
    // Qt 4.6
    painter.endNativePainting();
    CVF_CHECK_OGL(currentOglContext);
#endif

    if (m_drawHUD)
    {
        drawHud(&painter, currentOglContext);
    }

    CVF_CHECK_OGL(currentOglContext);
}


//------------------------------------------------------------------------------------------------
/// Draw the performance info in the view using Qt
//------------------------------------------------------------------------------------------------
void QSRSnippetWidget::drawHud(QPainter* painter, cvf::OpenGLContext* oglContext) const
{
    const RenderSequence* renderSeq = m_snippet->renderSequence();
    const Camera* camera = m_snippet->camera();

    const cvf::OpenGLResourceManager* resourceManager = oglContext ? oglContext->resourceManager() : NULL;

	cvfqt::PerformanceInfoHud hud;

    // For some obscure reason text drawing via Qt seems to go hay-wire in software OpenGL mode quite often
    // For the moment it seems that stuffing a long string (longer than any other strings) at the top of the HUD
    // fixes this. Go figure!
    hud.addString("Ceetron Qt Snippet Runner Performance Info:");

    if (renderSeq) 
    {
        hud.addStrings(renderSeq->performanceInfo());
        hud.addString(QString("Num renderings: %1").arg(renderSeq->renderingCount()));
    }

    if (resourceManager)
    {
        hud.addStrings(*resourceManager);
    }

    if (camera)
	{
		hud.addString("");
		hud.addStrings(*camera);
	}

    hud.draw(painter, width(), height());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::enableMultisampleWhenDrawing(bool enable)
{
    m_enableMultisampleWhenDrawing = enable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_snippet.isNull()) return;

	cvfu::MouseEvent me = QSRTranslateEvent::translateMouseEvent(height(), *event);
    m_snippet->onMouseMoveEvent(&me);
	if (me.requestedAction() == cvfu::REDRAW)
    {
        update();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_snippet.isNull()) return;

	cvfu::MouseEvent me = QSRTranslateEvent::translateMouseEvent(height(), *event);
	cvfu::MouseButton buttonPressed = QSRTranslateEvent::translateMouseButton(event->button());
    m_snippet->onMousePressEvent(buttonPressed, &me);
	if (me.requestedAction() == cvfu::REDRAW)
    {
        update();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::mouseReleaseEvent( QMouseEvent *event )
{
    if (m_snippet.isNull()) return;

	cvfu::MouseEvent me = QSRTranslateEvent::translateMouseEvent(height(), *event);
    cvfu::MouseButton buttonReleased = QSRTranslateEvent::translateMouseButton(event->button());
    m_snippet->onMouseReleaseEvent(buttonReleased, &me);
    if (me.requestedAction() == cvfu::REDRAW)
    {
        update();
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QSRSnippetWidget::keyPressEvent(QKeyEvent* event)
{
    cvfu::KeyEvent keyEvent = QSRTranslateEvent::translateKeyEvent(*event);
    m_snippet->onKeyPressEvent(&keyEvent);
    if (keyEvent.requestedAction() == cvfu::REDRAW)
    {
        // Do a redraw of this snippet widget
        update();

        // Repaint the parent widget to make changes visible
        parentWidget()->repaint();
    }
}


//########################################################
#ifndef CVF_USING_CMAKE
#include "qt-generated/moc_QSRSnippetWidget.cpp"
#endif
//########################################################

