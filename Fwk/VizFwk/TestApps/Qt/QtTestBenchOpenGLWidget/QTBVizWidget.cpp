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

#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"

#include "QTBVizWidget.h"
#include "QTBMainWindow.h"

#include <QMouseEvent>



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTBVizWidget::QTBVizWidget(cvf::OpenGLContextGroup* contextGroup, QWidget* parent, QTBMainWindow* mainWindow)
:   cvfqt::OpenGLWidget(contextGroup, parent),
    m_mainWindow(mainWindow)
{
    cvf::Trace::show("QTBVizWidget[%d]::QTBVizWidget()", instanceNumber());

    initializeMembers();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::initializeMembers()
{
    m_camera = new cvf::Camera;

    cvf::ref<cvf::Rendering> rendering = new cvf::Rendering;
    rendering->setCamera(m_camera.p());

    cvf::ref<cvf::FixedAtlasFont> font = new cvf::FixedAtlasFont(cvf::FixedAtlasFont::STANDARD);

    rendering->addOverlayItem(new cvf::OverlayAxisCross(m_camera.p(), font.p()));

    m_titleOverlayTextBox = new cvf::OverlayTextBox(font.p());
    m_titleOverlayTextBox->setText("N/A");
    m_titleOverlayTextBox->setSizeToFitText();
    m_titleOverlayTextBox->setLayout(cvf::OverlayItem::VERTICAL, cvf::OverlayItem::TOP_LEFT);
    rendering->addOverlayItem(m_titleOverlayTextBox.p());

    m_renderSequence = new cvf::RenderSequence;
    m_renderSequence->addRendering(rendering.p());

    m_trackball = new cvf::ManipulatorTrackball;
    m_trackball->setCamera(m_camera.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::onWidgetOpenGLReady()
{
    cvf::Trace::show("QTBVizWidget[%d]::onWidgetOpenGLReady()", instanceNumber());
    m_mainWindow->handleVizWidgetIsOpenGLReady(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTBVizWidget::~QTBVizWidget()
{
    cvf::Trace::show("QTBVizWidget[%d]::~QTBVizWidget()", instanceNumber());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::setViewTitle(cvf::String viewTitle)
{
    if (m_titleOverlayTextBox.isNull())
    {
        return;
    }

    m_titleOverlayTextBox->setText(viewTitle);
    m_titleOverlayTextBox->setSizeToFitText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::String QTBVizWidget::viewTitle()
{
    if (m_titleOverlayTextBox.isNull())
    {
        return cvf::String();
    }

    return m_titleOverlayTextBox->text();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::setViewTitleHighlighted(bool highlighted)
{
    if (m_titleOverlayTextBox.isNull())
    {
        return;
    }

    const cvf::Color3f clr = highlighted ? cvf::Color3f(1.0f, 0.2f, 0.2f) : cvf::Color3f(0.2f, 0.2f, 1.0f);
    m_titleOverlayTextBox->setBackgroundColor(clr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::setScene(cvf::Scene* scene)
{
    cvf::ref<cvf::Rendering> rendering = m_renderSequence->firstRendering();
    CVF_ASSERT(rendering.notNull());

    rendering->setScene(scene);

    if (scene)
    {
        cvf::BoundingBox bb = scene->boundingBox();
        if (bb.isValid())
        {
            m_camera->fitView(bb, -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS);
            m_trackball->setRotationPoint(bb.center());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
cvf::Scene* QTBVizWidget::scene()
{
    cvf::ref<cvf::Rendering> rendering = m_renderSequence->firstRendering();
    CVF_ASSERT(rendering.notNull());
    
    return rendering->scene();
}

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
cvf::OpenGLContext* QTBVizWidget::cvfOpenGLContext()
{
    return cvfqt::OpenGLWidget::cvfOpenGLContext();
}

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::initializeGL()
{
    cvf::Trace::show("QTBVizWidget[%d]::initializeGL()", instanceNumber());

    cvfqt::OpenGLWidget::initializeGL();
}

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::resizeGL(int width, int height)
{
    cvf::Trace::show("QTBVizWidget[%d]::resizeGL() w=%d h=%d", instanceNumber(), width, height);

    if (m_camera.notNull())
    {
        m_camera->viewport()->set(0, 0, width, height);
        m_camera->setProjectionAsPerspective(m_camera->fieldOfViewYDeg(), m_camera->nearPlane(), m_camera->farPlane());
    }
}

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::paintGL()
{
    const int w = width();
    const int h = height();
    cvf::Trace::show("QTBVizWidget[%d]::paintGL(), w=%d h=%d", instanceNumber(), w, h);

    // According to Qt docs, context should already be current
    cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    CVF_ASSERT(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

    cvf::OpenGLUtils::pushOpenGLState(currentOglContext);

    if (m_renderSequence.notNull())
    {
        m_renderSequence->render(currentOglContext);
    }

    cvf::OpenGLUtils::popOpenGLState(currentOglContext);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_renderSequence.isNull()) return;

    Qt::MouseButtons mouseBn = event->buttons();
    const int posX = event->position().toPoint().x();
    const int posY = height() - event->position().toPoint().y();

    cvf::ManipulatorTrackball::NavigationType navType = cvf::ManipulatorTrackball::NONE;
    if (mouseBn == Qt::LeftButton)
    {
        navType = cvf::ManipulatorTrackball::PAN;
    }
    else if (mouseBn == Qt::RightButton) 
    {
        navType = cvf::ManipulatorTrackball::ROTATE;
    }
    else if (mouseBn == (Qt::LeftButton | Qt::RightButton) || mouseBn == Qt::MiddleButton)
    {
        navType = cvf::ManipulatorTrackball::WALK;
    }

    if (navType != m_trackball->activeNavigation())
    {
        m_trackball->startNavigation(navType, posX, posY);
    }

    bool needRedraw = m_trackball->updateNavigation(posX, posY);
    if (needRedraw)
    {
        update();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_renderSequence.isNull()) return;

    const int posX = event->position().toPoint().x();
    const int posY = height() - event->position().toPoint().y();

    if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier)
    {
        cvf::Rendering* r = m_renderSequence->firstRendering();
        cvf::ref<cvf::RayIntersectSpec> ris = r->rayIntersectSpecFromWindowCoordinates(posX, posY);

        cvf::HitItemCollection hic;
        if (r->rayIntersect(*ris, &hic))
        {
            cvf::HitItem* item = hic.firstItem();
            CVF_ASSERT(item && item->part());

            cvf::Vec3d isect = item->intersectionPoint();
            m_trackball->setRotationPoint(isect);

            cvf::Trace::show("hitting part: '%s'   coords: %.3f %.3f %.3f", item->part()->name().toAscii().ptr(), isect.x(), isect.y(), isect.z());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    m_trackball->endNavigation();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBVizWidget::closeEvent(QCloseEvent* event)
{
    cvf::Trace::show("QTBVizWidget[%d]::closeEvent()", instanceNumber());

    cvfqt::OpenGLWidget::closeEvent(event);
}

