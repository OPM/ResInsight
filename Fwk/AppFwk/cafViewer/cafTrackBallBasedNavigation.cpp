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
#include "cvfHitItemCollection.h"
#include "cvfManipulatorTrackball.h"
#include "cvfModel.h"
#include "cvfRay.h"
#include "cvfScene.h"
#include "cvfViewport.h"

#include <QDebug>
#include <QInputEvent>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::TrackBallBasedNavigation::TrackBallBasedNavigation()
    : m_isRotCenterInitialized( false )
    , m_isNavigating( false )
    , m_hasMovedMouseDuringNavigation( false )
    , m_isZooming( false )
    , m_lastPosX( 0 )
    , m_lastPosY( 0 )
    , m_lastWheelZoomPosX( -1 )
    , m_lastWheelZoomPosY( -1 )
    , m_consumeEvents( false )
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
    m_trackball->setCamera( m_viewer->mainCamera() );
    m_isRotCenterInitialized        = false;
    m_isRotationEnabled             = true;
    m_hasMovedMouseDuringNavigation = false;
    m_isNavigating                  = false;
    m_isZooming                     = false;
    m_lastPosX                      = 0;
    m_lastPosY                      = 0;
    m_lastWheelZoomPosX             = -1;
    m_lastWheelZoomPosY             = -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::initializeRotationCenter()
{
    if ( m_isRotCenterInitialized || m_trackball.isNull() || !m_viewer->currentScene() ||
         !m_viewer->currentScene()->boundingBox().isValid() )
    {
        return;
    }

    cvf::Vec3d pointOfInterest = m_viewer->currentScene()->boundingBox().center();

    this->setPointOfInterest( pointOfInterest );
}

//--------------------------------------------------------------------------------------------------
/// Repositions and orients the camera to view the rotation point along the
/// direction "alongDirection". The distance to the rotation point is maintained.
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection )
{
    m_trackball->setView( alongDirection, upDirection );
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
void caf::TrackBallBasedNavigation::setPointOfInterest( cvf::Vec3d poi )
{
    m_pointOfInterest = poi;
    m_trackball->setRotationPoint( poi );
    m_isRotCenterInitialized = true;
    m_viewer->updateParallelProjectionCameraPosFromPointOfInterestMove( m_pointOfInterest );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::pickAndSetPointOfInterest( int winPosX, int winPosY )
{
    cvf::HitItemCollection hic;
    bool                   hitSomething = m_viewer->rayPick( winPosX, winPosY, &hic );

    if ( hitSomething )
    {
        cvf::Vec3d pointOfInterest = hic.firstItem()->intersectionPoint();

        if ( m_viewer->isMousePosWithinComparisonView( winPosX, winPosY ) )
        {
            pointOfInterest -= m_viewer->comparisonViewEyePointOffset();
        }

        this->setPointOfInterest( pointOfInterest );
    }
    else
    {
        initializeRotationCenter();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::updatePointOfInterestDuringZoomIfNecessary( int winPosX, int winPosY )
{
    if ( shouldRaytraceForNewPoiDuringWheelZoom( winPosX, winPosY ) )
    {
        this->pickAndSetPointOfInterest( winPosX, winPosY );
        updateWheelZoomPosition( winPosX, winPosY );
    }
    else
    {
        initializeRotationCenter();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::forcePointOfInterestUpdateDuringNextWheelZoom()
{
    m_lastWheelZoomPosX = -1;
    m_lastWheelZoomPosY = -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::zoomAlongRay( cvf::Ray* ray, int delta )
{
    if ( ray && abs( delta ) > 0 )
    {
        cvf::Vec3d pos, vrp, up;
        m_viewer->mainCamera()->toLookAt( &pos, &vrp, &up );

        double     scale  = delta / 8.0 * 1.0 / 150 * ( pos - m_pointOfInterest ).length();
        cvf::Vec3d trans  = scale * ray->direction();
        cvf::Vec3d newPos = pos + trans;
        cvf::Vec3d newVrp = vrp + trans;

        m_viewer->mainCamera()->setFromLookAt( newPos, newVrp, up );
        m_viewer->updateParallelProjectionHeightFromMoveZoom( m_pointOfInterest );
        m_viewer->updateParallelProjectionCameraPosFromPointOfInterestMove( m_pointOfInterest );

        // Ceeviz Workaround for #3697:
        // Ceeviz may create a singular projection*view matrix internally. In which case we need to revert.
        cvf::Mat4d projectionMatrix = m_viewer->mainCamera()->projectionMatrix();
        cvf::Mat4d viewMatrix       = m_viewer->mainCamera()->viewMatrix();
        cvf::Mat4d multMatrix       = projectionMatrix * viewMatrix;
        double     determinant      = std::fabs( multMatrix.determinant() );

        if ( determinant < 1.0e-15 )
        {
            m_viewer->mainCamera()->setFromLookAt( pos, vrp, up );
            m_viewer->updateParallelProjectionHeightFromMoveZoom( m_pointOfInterest );
            m_viewer->updateParallelProjectionCameraPosFromPointOfInterestMove( m_pointOfInterest );
#ifndef NDEBUG
            projectionMatrix = m_viewer->mainCamera()->projectionMatrix();
            viewMatrix       = m_viewer->mainCamera()->viewMatrix();
            multMatrix       = projectionMatrix * viewMatrix;
            determinant      = std::fabs( multMatrix.determinant() );
            CVF_ASSERT( determinant > 1.0e-15 );
#endif
        }

        m_viewer->navigationPolicyUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::cvfEventPos( int qtX, int qtY, int* cvfX, int* cvfY )
{
    *cvfX = qtX;
    *cvfY = m_viewer->height() - qtY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Ray> caf::TrackBallBasedNavigation::createZoomRay( int cvfXPos, int cvfYPos )
{
    cvf::ref<cvf::Ray> ray;
    cvf::Camera*       cam = m_viewer->mainCamera();
    ray                    = cam->rayFromWindowCoordinates( cvfXPos, cvfYPos );

    if ( ray.notNull() && cam->projection() == cvf::Camera::ORTHO )
    {
        cvf::Vec3d camDir = cam->direction();
        cvf::Plane focusPlane;
        focusPlane.setFromPointAndNormal( m_pointOfInterest, -camDir );
        cvf::Vec3d intersectionPoint;
        ray->planeIntersect( focusPlane, &intersectionPoint );

        cvf::ref<cvf::Ray> orthoZoomRay = new cvf::Ray();
        orthoZoomRay->setOrigin( cam->position() );
        orthoZoomRay->setDirection( ( intersectionPoint - cam->position() ).getNormalized() );
        ray = orthoZoomRay;
    }

    return ray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::TrackBallBasedNavigation::updateWheelZoomPosition( int winPosX, int winPosY )
{
    m_lastWheelZoomPosX = winPosX;
    m_lastWheelZoomPosY = winPosY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::TrackBallBasedNavigation::shouldRaytraceForNewPoiDuringWheelZoom( int winPosX, int winPosY ) const
{
    // Raytrace if the last zoom position isn't set
    if ( m_lastWheelZoomPosX == -1 || m_lastWheelZoomPosY == -1 )
    {
        return true;
    }
    int diffX = winPosX - m_lastWheelZoomPosX;
    int diffY = winPosY - m_lastWheelZoomPosY;

    const int pixelThreshold = 5;
    if ( diffX * diffX + diffY * diffY > pixelThreshold * pixelThreshold )
    {
        return true;
    }
    return false;
}

#include <QMouseEvent>

//#include <windows.h>
//
//#pragma warning(disable:4996)
// void openDebugWindow()
//{
//    AllocConsole();
//    freopen("conin$", "r", stdin);
//    freopen("conout$", "w", stdout);
//    freopen("conout$", "w", stderr);
//}
//
//#include <iostream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::RotationSensitivityCalculator::init( QMouseEvent* eventAtRotationStart )
{
    m_lastPosX = eventAtRotationStart->x();
    m_lastPosY = eventAtRotationStart->y();

    m_lastTime = eventAtRotationStart->timestamp();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double caf::RotationSensitivityCalculator::calculateSensitivity( QMouseEvent* eventWhenRotating )
{
    double sensitivity = 1.0;

    if ( m_isEnabled )
    {
        if ( m_fixedSensitivity == std::numeric_limits<double>::infinity() )
        {
            auto          presentTime   = eventWhenRotating->timestamp();
            unsigned long timeSinceLast = presentTime - m_lastTime;
            if ( timeSinceLast == 0 ) timeSinceLast = 1; // one millisecond

            int deltaX = eventWhenRotating->x() - m_lastPosX;
            int deltaY = eventWhenRotating->y() - m_lastPosY;

            cvf::Vec2d mouseVelocity( deltaX, deltaY );
            mouseVelocity /= 1.0e-3 * timeSinceLast;

            double mouseVelocityLength     = mouseVelocity.length();
            double mouseVelocityLengthCorr = 0.1 * mouseVelocityLength + 0.9 * m_lastMouseVelocityLenght;

            double slowLimit = 170.0;

            if ( mouseVelocityLengthCorr < slowLimit )
                sensitivity = mouseVelocityLengthCorr * mouseVelocityLengthCorr / ( slowLimit * slowLimit );

            m_lastPosX                = eventWhenRotating->x();
            m_lastPosY                = eventWhenRotating->y();
            m_lastTime                = eventWhenRotating->timestamp();
            m_lastMouseVelocityLenght = 0.8 * mouseVelocityLength + 0.2 * m_lastMouseVelocityLenght;
        }
        else
        {
            sensitivity = m_fixedSensitivity;
        }
    }

    return sensitivity;
}
