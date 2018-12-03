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

#include "QMVWidget.h"

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
QMVWidget::QMVWidget(cvf::OpenGLContextGroup* contextGroup, const QGLFormat& format, QWidget* parent)
:   cvfqt::OpenGLWidget(contextGroup, format, parent)
{
    m_trackball = new cvf::ManipulatorTrackball;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMVWidget::QMVWidget(QMVWidget* shareWidget, QWidget* parent)
:   cvfqt::OpenGLWidget(shareWidget, parent)
{
    m_trackball = new cvf::ManipulatorTrackball;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMVWidget::~QMVWidget()
{
    cvfShutdownOpenGLContext();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVWidget::setRenderSequence(cvf::RenderSequence* renderSequence)
{
    m_trackball->setCamera(NULL);
    m_renderSequence = renderSequence;

    if (m_renderSequence.notNull())
    {
        // Camera extracted from first rendering of the view
        cvf::Camera* camera = currentCamera();
        camera->viewport()->set(0, 0, width(), height());
        camera->setProjectionAsPerspective(camera->fieldOfViewYDeg(), camera->nearPlane(), camera->farPlane());

        m_trackball->setCamera(camera);

        cvf::BoundingBox bb = m_renderSequence->boundingBox();
        if (bb.isValid())
        {
            m_trackball->setRotationPoint(bb.center());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::RenderSequence* QMVWidget::renderSequence()
{
    return m_renderSequence.p();
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void QMVWidget::resizeGL(int width, int height)
{
    cvf::Camera* camera = currentCamera();
    if (camera)
    {
        camera->viewport()->set(0, 0, width, height);
        camera->setProjectionAsPerspective(camera->fieldOfViewYDeg(), camera->nearPlane(), camera->farPlane());
    }
    else
    {
        glViewport(0, 0, width, height);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVWidget::paintGL()
{
    cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    CVF_ASSERT(currentOglContext);
    CVF_CHECK_OGL(currentOglContext);

    cvfqt::OpenGLContext::saveOpenGLState(currentOglContext);

    if (m_renderSequence.notNull())
    {
        m_renderSequence->render(currentOglContext);
    }
    else
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    cvfqt::OpenGLContext::restoreOpenGLState(currentOglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Camera* QMVWidget::currentCamera()
{
    if (m_renderSequence.notNull())
    {
        cvf::Rendering* rendering = m_renderSequence->firstRendering();
        if (rendering)
        {
            return rendering->camera();
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVWidget::mouseMoveEvent(QMouseEvent* event)
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
void QMVWidget::mousePressEvent(QMouseEvent* event)
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
void QMVWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    m_trackball->endNavigation();
}


//########################################################
#ifndef CVF_USING_CMAKE
#include "qt-generated/moc_QMVWidget.cpp"
#endif
//########################################################

