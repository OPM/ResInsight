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

#include "QMWidget.h"

#include "cvfqtOpenGLContext.h"

#if QT_VERSION >= 0x050000
#include <QMouseEvent>
#else
#include <QtGui/QMouseEvent>
#endif

using cvf::ref;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMWidget::QMWidget(cvf::OpenGLContextGroup* contextGroup, QWidget* parent)
:   cvfqt::OpenGLWidget(contextGroup, QGLFormat(), parent)
{
    m_camera = new cvf::Camera;

    ref<cvf::Rendering> rendering = new cvf::Rendering;
    rendering->setCamera(m_camera.p());
    rendering->addOverlayItem(new cvf::OverlayAxisCross(m_camera.p(), new cvf::FixedAtlasFont(cvf::FixedAtlasFont::STANDARD)));

    m_renderSequence = new cvf::RenderSequence;
    m_renderSequence->addRendering(rendering.p());

    m_trackball = new cvf::ManipulatorTrackball;
    m_trackball->setCamera(m_camera.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMWidget::~QMWidget()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMWidget::setScene(cvf::Scene* scene)
{
    ref<cvf::Rendering> rendering = m_renderSequence->firstRendering();
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

    update();
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QMWidget::resizeGL(int width, int height)
{
    if (m_camera.notNull())
    {
        m_camera->viewport()->set(0, 0, width, height);
        m_camera->setProjectionAsPerspective(m_camera->fieldOfViewYDeg(), m_camera->nearPlane(), m_camera->farPlane());
    }
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QMWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);

    makeCurrent();

    cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    CVF_ASSERT(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

#if QT_VERSION >= 0x040600
    painter.beginNativePainting();
#endif

	cvfqt::OpenGLContext::saveOpenGLState(currentOglContext);

    if (m_renderSequence.notNull())
    {
        m_renderSequence->render(currentOglContext);
    }

	cvfqt::OpenGLContext::restoreOpenGLState(currentOglContext);

#if QT_VERSION >= 0x040600
    painter.endNativePainting();
#endif
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_renderSequence.isNull()) return;

    Qt::MouseButtons mouseBn = event->buttons();
    int posX = event->x();
    int posY = height() - event->y();

    cvf::ManipulatorTrackball::NavigationType navType = cvf::ManipulatorTrackball::NONE;
    if (mouseBn == Qt::LeftButton)
    {
        navType = cvf::ManipulatorTrackball::PAN;
    }
    else if (mouseBn == Qt::RightButton) 
    {
        navType = cvf::ManipulatorTrackball::ROTATE;
    }
    else if (mouseBn == (Qt::LeftButton | Qt::RightButton) || mouseBn == Qt::MidButton)
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
void QMWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_renderSequence.isNull()) return;

    if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier)
    {
        cvf::Rendering* r = m_renderSequence->firstRendering();
        ref<cvf::RayIntersectSpec> ris = r->rayIntersectSpecFromWindowCoordinates(event->x(), height() - event->y());

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
void QMWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    m_trackball->endNavigation();
}




#ifndef CVF_USING_CMAKE
//########################################################
#include "qt-generated/moc_QMWidget.cpp"
//########################################################
#endif
