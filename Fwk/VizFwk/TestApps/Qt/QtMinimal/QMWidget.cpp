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

#include "QMWidget.h"
#include "QMMainWindow.h"

#include "cvfRendering.h"
#include "cvfRenderSequence.h"
#include "cvfScene.h"
#include "cvfCamera.h"
#include "cvfManipulatorTrackball.h"
#include "cvfOverlayAxisCross.h"
#include "cvfFixedAtlasFont.h"
#include "cvfRayIntersectSpec.h"
#include "cvfHitItemCollection.h"
#include "cvfHitItem.h"
#include "cvfTrace.h"
#include "cvfPart.h"

#include <QMouseEvent>



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMWidget::QMWidget(cvf::OpenGLContextGroup* contextGroup, QWidget* parent, QMMainWindow* mainWindow)
:   cvfqt::OpenGLWidget(contextGroup, parent),
    m_mainWindow(mainWindow)
{
    CVF_ASSERT(contextGroup);

    m_camera = new cvf::Camera;

    cvf::ref<cvf::Rendering> rendering = new cvf::Rendering;
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

    update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMWidget::resizeGL(int w, int h)
{
    if (m_camera.notNull())
    {
        m_camera->viewport()->set(0, 0, w, h);
        m_camera->setProjectionAsPerspective(m_camera->fieldOfViewYDeg(), m_camera->nearPlane(), m_camera->farPlane());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMWidget::paintGL()
{
    cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    CVF_ASSERT(currentOglContext);

    if (m_renderSequence.notNull())
    {
        m_renderSequence->render(currentOglContext);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMWidget::mouseMoveEvent(QMouseEvent* event)
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
void QMWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_renderSequence.isNull()) return;

    const int posX = event->position().toPoint().x();
    const int posY = height() - event->position().toPoint().y();

    if (event->buttons() == Qt::LeftButton && 
        event->modifiers() == Qt::ControlModifier)
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
void QMWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    m_trackball->endNavigation();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMWidget::onWidgetOpenGLReady()
{
    if (m_mainWindow)
    {
        m_mainWindow->handleVizWidgetIsOpenGLReady();
    }
}
