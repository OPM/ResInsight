//##################################################################################################
//
//  Copyright (C) 2011, Ceetron AS
//  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Ceetron AS. The contents of this file may 
//  not be disclosed to third parties, copied or duplicated in any form, in whole or in part,
//  without the prior written permission of Ceetron AS.
//##################################################################################################
#include "cafCeetronPlusNavigation.h"
#include "cafViewer.h"
#include "cvfCamera.h"
#include "cvfScene.h"
#include "cvfModel.h"
#include "cvfViewport.h"
#include "cvfHitItemCollection.h"
#include "cvfRay.h"

#include <QInputEvent>
#include <QHBoxLayout>

using cvf::ManipulatorTrackball;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::CeetronPlusNavigation::init()
{
    m_trackball = new cvf::ManipulatorTrackball;
    m_trackball->setCamera(m_viewer->mainCamera());
    m_isRotCenterInitialized = false;
    m_hasMovedMouseDuringNavigation = false;
    m_isNavigating = false;
    m_isZooming = false;
    m_lastPosX = 0;
    m_lastPosY = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::CeetronPlusNavigation::handleInputEvent(QInputEvent* inputEvent)
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

            if (me->button() == Qt::RightButton)
            {
                cvf::HitItemCollection hic;
                bool hitSomething = m_viewer->rayPick(me->x(), me->y(), &hic);

                if (hitSomething)
                { 
                    cvf::Vec3d pointOfInterest = hic.firstItem()->intersectionPoint();
                    m_trackball->setRotationPoint(pointOfInterest);
                    m_pointOfInterest = pointOfInterest;
                }
                else
                {
                    initializeRotationCenter();
                }

                m_trackball->startNavigation(cvf::ManipulatorTrackball::ROTATE, translatedMousePosX, translatedMousePosY);
                //m_viewer->setCursor(RICursors::get(RICursors::ROTATE));
                m_isNavigating = true;
                m_hasMovedMouseDuringNavigation = false;
                isEventHandled = true;
            }
            else if (me->button() == Qt::LeftButton)
            {
                if (me->modifiers() == Qt::NoModifier)
                {
                    m_trackball->startNavigation(cvf::ManipulatorTrackball::PAN, translatedMousePosX, translatedMousePosY);
                    m_isNavigating = true;
                    m_hasMovedMouseDuringNavigation = false;
                    isEventHandled = true;
                }
            }
            else if (me->button() == Qt::MidButton)
            {
                if (me->modifiers() == Qt::NoModifier)
                {
                    QMouseEvent* we = static_cast<QMouseEvent*> ( inputEvent);
                    m_lastPosX = we->x();
                    m_lastPosY = we->y();

                    m_zoomRay = m_viewer->mainCamera()->rayFromWindowCoordinates(translatedMousePosX, translatedMousePosY);

                    m_isNavigating = true;
                    m_hasMovedMouseDuringNavigation = false;
                    isEventHandled = true;
                    m_isZooming = true;
                }
            }
        }
        break;
    case QEvent::MouseButtonRelease: 
        {
            if (m_isNavigating)
            {
                QMouseEvent * me = static_cast<QMouseEvent*>( inputEvent);
                if (me->button() == Qt::RightButton || me->button() == Qt::LeftButton )
                {
                    m_trackball->endNavigation();

                    m_isNavigating = false;
                    if (m_hasMovedMouseDuringNavigation) isEventHandled = true;
                    m_hasMovedMouseDuringNavigation = false;
                }
                else if ( me->button() == Qt::MidButton )
                {
                    m_isZooming = false;

                    m_isNavigating = false;
                    if (m_hasMovedMouseDuringNavigation) isEventHandled = true;
                    m_hasMovedMouseDuringNavigation = false;
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
                    if (m_isZooming)
                    {
                        int delta = 3*(m_lastPosY - me->y());
                        this->zoomAlongRay(m_zoomRay.p(), delta);
                        m_lastPosX = me->x();
                        m_lastPosY = me->y();
                    }
                    else
                    {
                        bool needRedraw = m_trackball->updateNavigation(translatedMousePosX, translatedMousePosY);
                        if (needRedraw)
                        {
                            m_viewer->navigationPolicyUpdate();
                        }
                    }
                    isEventHandled = true;
                    m_hasMovedMouseDuringNavigation = true;
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
                    QWheelEvent* we = static_cast<QWheelEvent*> ( inputEvent);
                    int translatedMousePosX = we->x();
                    int translatedMousePosY = m_viewer->height() - we->y();
                    int delta = we->delta();

                    cvf::ref<cvf::Ray> ray;
                    if (delta < 0)
                        ray = m_viewer->mainCamera()->rayFromWindowCoordinates(translatedMousePosX, translatedMousePosY);
                    else
                        ray = m_viewer->mainCamera()->rayFromWindowCoordinates((int)(1.0*translatedMousePosX), (int)(1.0*translatedMousePosY));

                    zoomAlongRay(ray.p(), delta);

                }
                isEventHandled = true;
            }
        }
        break;
    }

    return false;//isEventHandled;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::CeetronPlusNavigation::initializeRotationCenter()
{
    if(m_isRotCenterInitialized || m_trackball.isNull() || !m_viewer->mainScene() || !m_viewer->currentScene()->boundingBox().isValid()) return;
    m_pointOfInterest = m_viewer->currentScene()->boundingBox().center();

    m_trackball->setRotationPoint(m_pointOfInterest);
    m_isRotCenterInitialized = true;
}

//--------------------------------------------------------------------------------------------------
/// Repositions and orients the camera to view the rotation point along the 
/// direction "alongDirection". The distance to the rotation point is maintained.
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronPlusNavigation::setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection )
{
    m_trackball->setView(alongDirection, upDirection);
    /*
    if (m_camera.isNull()) return;

    Vec3d dir = alongDirection;
    if (!dir.normalize()) return;
    Vec3d up = upDirection;
    if(!up.normalize()) up = Vec3d::Z_AXIS;

    if((up * dir) < 1e-2) up = dir.perpendicularVector();

    Vec3d cToE = m_camera->position() - m_rotationPoint;
    Vec3d newEye = m_rotationPoint - cToE.length() * dir;

    m_camera->setFromLookAt(newEye, m_rotationPoint, upDirection);
    */
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::CeetronPlusNavigation::pointOfInterest()
{
   initializeRotationCenter();     
   return m_pointOfInterest;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::CeetronPlusNavigation::setPointOfInterest(cvf::Vec3d poi)
{
    m_pointOfInterest = poi;
    m_trackball->setRotationPoint(poi);
    m_isRotCenterInitialized = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::CeetronPlusNavigation::zoomAlongRay(cvf::Ray* ray, int delta)
{
    if (ray && abs(delta) > 0)
    {
        cvf::Vec3d pos, vrp, up;
        m_viewer->mainCamera()->toLookAt(&pos, &vrp, &up);

        double scale = delta/8.0 * 1.0/150 * (pos - m_pointOfInterest).length();
        cvf::Vec3d trans = scale * ray->direction();
        cvf::Vec3d newPos = pos + trans;
        cvf::Vec3d newVrp = vrp + trans;

        m_viewer->mainCamera()->setFromLookAt(newPos, newVrp, up );
        m_viewer->navigationPolicyUpdate();
    }
}
