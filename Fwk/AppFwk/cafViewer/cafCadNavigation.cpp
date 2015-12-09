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


#include "cafCadNavigation.h"
#include "cafViewer.h"
#include "cvfCamera.h"
#include "cvfViewport.h"
#include "cvfHitItemCollection.h"
#include "cvfRay.h"
#include "cvfManipulatorTrackball.h"

#include <QInputEvent>
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::CadNavigation::CadNavigation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::CadNavigation::~CadNavigation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::CadNavigation::init()
{
    caf::TrackBallBasedNavigation::init();
    m_navigationUpdated = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::CadNavigation::handleInputEvent(QInputEvent* inputEvent)
{
    if (! inputEvent) return false;
    bool isEventHandled = false;

    switch (inputEvent->type())
    {
    case QEvent::MouseButtonPress:
        {
            QMouseEvent * me = static_cast<QMouseEvent*>( inputEvent);
            int translatedMousePosX = me->x();
            int translatedMousePosY = m_viewer->height() - me->y();

            if (me->button() == Qt::MidButton)
            {
                if (me->modifiers() & Qt::ShiftModifier)
                {
                     m_trackball->startNavigation(cvf::ManipulatorTrackball::PAN, translatedMousePosX, translatedMousePosY);
                     m_isNavigating = true;
                     isEventHandled = true;
                }
                else if (me->modifiers() == Qt::NoModifier)
                {
                    cvf::HitItemCollection hic;
                    bool hitSomething = m_viewer->rayPick( me->x(),  me->y(), &hic);

                    if (hitSomething)
                    { 
                        cvf::Vec3d pointOfInterest = hic.firstItem()->intersectionPoint();
                        this->setPointOfInterest(pointOfInterest);
                    }
                    else
                    {
                        initializeRotationCenter();
                    }

                    m_trackball->startNavigation(cvf::ManipulatorTrackball::ROTATE, translatedMousePosX, translatedMousePosY);
                    //m_viewer->setCursor(RiuCursors::get(RiuCursors::ROTATE));
                    m_isNavigating = true;
                    isEventHandled = true;
                }
            }

            if (isEventHandled)
            {
                m_navigationUpdated = false;
            }
        }
        break;
    case QEvent::MouseButtonRelease: 
        {
            if (m_isNavigating)
            {
                QMouseEvent * me = static_cast<QMouseEvent*>( inputEvent);
                if (me->button() == Qt::MidButton)
                {
                    m_trackball->endNavigation();
                    //m_viewer->setCursor(RiuCursors::get(RiuCursors::PICK));
                    m_isNavigating = false;
                    
                    isEventHandled = m_navigationUpdated;
                    m_navigationUpdated = false;
                }
            }
        }
        break;
    case QEvent::MouseMove:
        {
            initializeRotationCenter();
            if (m_isRotCenterInitialized)
            {
                QMouseEvent * me = static_cast<QMouseEvent*>( inputEvent);
                int translatedMousePosX = me->x();
                int translatedMousePosY = m_viewer->height() - me->y();

                if (m_isNavigating)
                {
                    bool needRedraw = m_trackball->updateNavigation(translatedMousePosX, translatedMousePosY);
                    if (needRedraw)
                    {
                        m_viewer->navigationPolicyUpdate();
                        m_navigationUpdated = true;
                    }
                    isEventHandled = true;
                }
            }
        }
        break;
    case QEvent::Wheel:
        {
            if (inputEvent->modifiers() == Qt::NoModifier)
            {
                initializeRotationCenter();
                if (m_isRotCenterInitialized)
                {
                    QWheelEvent* we = static_cast<QWheelEvent*>(inputEvent);
                    int translatedMousePosX = we->x();
                    int translatedMousePosY = m_viewer->height() - we->y();

                    cvf::ref<cvf::Ray> ray;
                    if (we->delta() > 0)
                        ray = m_viewer->mainCamera()->rayFromWindowCoordinates(translatedMousePosX, translatedMousePosY);
                    else
                        ray = m_viewer->mainCamera()->rayFromWindowCoordinates((int)(1.0*translatedMousePosX), (int)(1.0*translatedMousePosY));

                    if (ray.notNull() && abs(we->delta()) > 0)
                    {
                        cvf::Vec3d pos, vrp, up;
                        m_viewer->mainCamera()->toLookAt(&pos, &vrp, &up);

                        double scale = -we->delta()/8.0 * 1.0/150 * (pos - m_pointOfInterest).length();
                        cvf::Vec3d trans = scale * ray->direction();
                        cvf::Vec3d newPos = pos + trans;
                        cvf::Vec3d newVrp = vrp + trans;

                        m_viewer->mainCamera()->setFromLookAt(newPos,newVrp, up );

                        m_viewer->updateParallelProjectionHeightFromMoveZoom(m_pointOfInterest);
                        m_viewer->navigationPolicyUpdate();
                    }
                }
                isEventHandled = true;
            }
        }
        break;
    }

    return isEventHandled;
}
