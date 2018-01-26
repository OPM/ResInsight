//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015 Ceetron Solutions AS
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

#include "cafTrackBallBasedNavigation.h"

#include "cafViewer.h"
#include "cvfCamera.h"
#include "cvfScene.h"
#include "cvfModel.h"
#include "cvfViewport.h"
#include "cvfHitItemCollection.h"
#include "cvfRay.h"
#include "cvfManipulatorTrackball.h"

#include <QInputEvent>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::TrackBallBasedNavigation::TrackBallBasedNavigation() : 
    m_isRotCenterInitialized(false),
    m_isNavigating(false),
    m_hasMovedMouseDuringNavigation(false),
    m_isZooming(false),
    m_lastPosX(0),
    m_lastPosY(0),
    m_consumeEvents(false)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::TrackBallBasedNavigation::~TrackBallBasedNavigation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::init()
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
void caf::TrackBallBasedNavigation::initializeRotationCenter()
{
    if (m_isRotCenterInitialized
        || m_trackball.isNull()
        || !m_viewer->currentScene()
        || !m_viewer->currentScene()->boundingBox().isValid())
    {
        return;
    }

   cvf::Vec3d pointOfInterest = m_viewer->currentScene()->boundingBox().center();

   this->setPointOfInterest(pointOfInterest);
}

//--------------------------------------------------------------------------------------------------
/// Repositions and orients the camera to view the rotation point along the 
/// direction "alongDirection". The distance to the rotation point is maintained.
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection )
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
cvf::Vec3d caf::TrackBallBasedNavigation::pointOfInterest()
{
   initializeRotationCenter();     
   return m_pointOfInterest;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::setPointOfInterest(cvf::Vec3d poi)
{
    m_pointOfInterest = poi;
    m_trackball->setRotationPoint(poi);
    m_isRotCenterInitialized = true;
    m_viewer->updateParallelProjectionCameraPosFromPointOfInterestMove(m_pointOfInterest);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::zoomAlongRay(cvf::Ray* ray, int delta)
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
        m_viewer->updateParallelProjectionHeightFromMoveZoom(m_pointOfInterest);
        m_viewer->updateParallelProjectionCameraPosFromPointOfInterestMove(m_pointOfInterest);

        m_viewer->navigationPolicyUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::cvfEventPos(int qtX, int qtY, int* cvfX, int* cvfY)
{
    *cvfX = qtX;
    *cvfY = m_viewer->height() - qtY;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Ray> caf::TrackBallBasedNavigation::createZoomRay(int cvfXPos, int cvfYPos)
{
    cvf::ref<cvf::Ray> ray;
    cvf::Camera* cam = m_viewer->mainCamera();
    ray = cam->rayFromWindowCoordinates(cvfXPos, cvfYPos);

    if (ray.notNull() && cam->projection() == cvf::Camera::ORTHO)
    {
        cvf::Vec3d camDir = cam->direction();
        cvf::Plane focusPlane;
        focusPlane.setFromPointAndNormal(m_pointOfInterest, -camDir);
        cvf::Vec3d intersectionPoint;
        ray->planeIntersect(focusPlane, &intersectionPoint);

        cvf::ref<cvf::Ray> orthoZoomRay = new cvf::Ray();
        orthoZoomRay->setOrigin(cam->position());
        orthoZoomRay->setDirection((intersectionPoint - cam->position()).getNormalized());
        ray = orthoZoomRay;
    }

    return ray;
}
