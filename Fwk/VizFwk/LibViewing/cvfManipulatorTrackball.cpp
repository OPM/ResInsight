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


#include "cvfBase.h"
#include "cvfManipulatorTrackball.h"
#include "cvfCamera.h"
#include "cvfViewport.h"

#include <cstdlib>

namespace cvf {



//==================================================================================================
///
/// \class cvf::ManipulatorTrackball
/// \ingroup Viewing
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ManipulatorTrackball::ManipulatorTrackball()
{
    m_rotationPoint.setZero();

    m_activeNavigation = NONE;
    m_lastPosX = 0;
    m_lastPosY = 0;
    m_walkStartCameraPos.setZero();

    m_walkSensitivity = 1.0;
    m_rotateSensitivity = 1.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ManipulatorTrackball::~ManipulatorTrackball()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// Set the camera that should be manipulated
//--------------------------------------------------------------------------------------------------
void ManipulatorTrackball::setCamera(Camera* camera)
{
    m_camera = camera;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ManipulatorTrackball::setRotationPoint(const Vec3d& rotPoint)
{
    m_rotationPoint = rotPoint;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ManipulatorTrackball::NavigationType ManipulatorTrackball::activeNavigation() const
{
    return m_activeNavigation;
}


//--------------------------------------------------------------------------------------------------
/// The window coordinates are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
void ManipulatorTrackball::startNavigation(NavigationType navigationType, int x, int y)
{
    if (m_camera.isNull()) return;
    if (m_activeNavigation == navigationType) return;

    m_lastPosX = x;
    m_lastPosY = y;

    // Register camera's starting position for walk
    m_walkStartCameraPos = m_camera->position();

    m_activeNavigation = navigationType;
}


//--------------------------------------------------------------------------------------------------
/// The window coordinates are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
bool ManipulatorTrackball::updateNavigation(int x, int y)
{
    if (m_activeNavigation == PAN)
    {
        return pan(x, y);
    }
    else if (m_activeNavigation == WALK)
    {
        return walk(x, y);
    }
    else if (m_activeNavigation == ZOOM)
    {
        return zoom(x, y);
    }
    else if (m_activeNavigation == ROTATE)
    {
        return rotate(x, y);
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ManipulatorTrackball::endNavigation()
{
    m_activeNavigation = NONE;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ManipulatorTrackball::setRotationSensitivity(double scaleFactor)
{
    m_rotateSensitivity = scaleFactor;
}

//--------------------------------------------------------------------------------------------------
/// Pan the camera up/down and left/right
/// 
/// \return  Returns true if input caused changes to the camera an false if no changes occurred
//--------------------------------------------------------------------------------------------------
bool ManipulatorTrackball::pan(int posX, int posY)
{
    if (m_camera.isNull()) return false;
    if (posX == m_lastPosX && posY == m_lastPosY) return false;

    const double vpPixSizeX = m_camera->viewport()->width();
    const double vpPixSizeY = m_camera->viewport()->height();
    if (vpPixSizeX <= 0 || vpPixSizeY <= 0) return false;

    // Normalized movement in screen plane 
    double tx = (posX - m_lastPosX)/vpPixSizeX;
    double ty = (posY - m_lastPosY)/vpPixSizeY;

    // Viewport size in world coordinates
    const double aspect = m_camera->aspectRatio();
    const double vpWorldSizeY = m_camera->frontPlaneFrustumHeight();
    const double vpWorldSizeX = vpWorldSizeY*aspect;

    const Vec3d camUp = m_camera->up();
    const Vec3d camRight = m_camera->right();


    Vec3d translation(0, 0, 0);

    Camera::ProjectionType projType = m_camera->projection();
    if (projType == Camera::PERSPECTIVE)
    {
        const Vec3d camPos = m_camera->position();
        const Vec3d camDir = m_camera->direction();
        const double nearPlane = m_camera->nearPlane();

        // Compute distance from camera to rotation point projected onto camera forward direction
        const Vec3d vDiff = m_rotationPoint - camPos;
        const double camRotPointDist = Math::abs(camDir*vDiff);

        Vec3d vX =  camRight*((tx*vpWorldSizeX)/nearPlane)*camRotPointDist;
        Vec3d vY =  camUp*((ty*vpWorldSizeY)/nearPlane)*camRotPointDist;
        translation = vX + vY;
    }
    else if (projType == Camera::ORTHO)
    {
        Vec3d vX =  camRight*tx*vpWorldSizeX;
        Vec3d vY =  camUp*ty*vpWorldSizeY;
        translation = vX + vY;
    }

    Mat4d viewMat = m_camera->viewMatrix();
    viewMat.translatePostMultiply(translation);
    m_camera->setViewMatrix(viewMat);

    m_lastPosX = posX;
    m_lastPosY = posY;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Walk forward backward along the current camera direction
/// 
/// \return  Returns true if input caused changes to the camera an false if no changes occurred
/// 
/// \remarks As this function moves the camera, it will have no effect when projection is orthographic
/// \todo    Needs better control over movement when camera position gets close to the rotation point.
//--------------------------------------------------------------------------------------------------
bool ManipulatorTrackball::walk(int posX, int posY)
{
    if (m_camera.isNull()) return false;
    if (posX == m_lastPosX && posY == m_lastPosY) return false;

    const double vpPixSizeY = m_camera->viewport()->height();
    if (vpPixSizeY <= 0) return false;

    // !!!!
    // Should be a member variable, settable as a ratio of the model bounding box/sphere
    const double minWalkTargetDistance = 1.0;

    const double ty = (m_lastPosY - posY)/vpPixSizeY;

    // Determine target distance to travel along camera direction
    // This is the distance that we will move the camera in response to a full (whole viewport) movement of the mouse
    const Vec3d camDir = m_camera->direction();
    const Vec3d vDiff = m_rotationPoint - m_walkStartCameraPos;
    double targetDist = Math::abs(camDir*vDiff);
    if (targetDist < minWalkTargetDistance) targetDist = minWalkTargetDistance;

    // Figure out movement to append
    double moveDist = ty*targetDist*m_walkSensitivity;
    Vec3d moveVec = camDir*moveDist;

    Mat4d viewMat = m_camera->viewMatrix();
    viewMat.translatePostMultiply(moveVec);
    m_camera->setViewMatrix(viewMat);

    m_lastPosX = posX;
    m_lastPosY = posY;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ManipulatorTrackball::zoom(int posX, int posY)
{
    if (m_camera.isNull()) return false;
    if (posX == m_lastPosX && posY == m_lastPosY) return false;

    const double vpPixSizeY = m_camera->viewport()->height();
    if (vpPixSizeY <= 0) return false;

    const double ty = (m_lastPosY - posY)/vpPixSizeY;

    const double frustumHeight = m_camera->frontPlaneFrustumHeight();

    double newFrustumHeight = frustumHeight;
    
    if (ty >= 0.0f) newFrustumHeight = frustumHeight*(1 + ty);
    else			newFrustumHeight = frustumHeight/(1 - ty); 
    
    const double nearPlane = m_camera->nearPlane();
    const double farPlane = m_camera->farPlane();

    Camera::ProjectionType projType = m_camera->projection();
    if (projType == Camera::PERSPECTIVE)
    {
        double fovY = 2*Math::atan((newFrustumHeight/2)/nearPlane);
        if (fovY > 0)
        {
            m_camera->setProjectionAsPerspective(Math::toDegrees(fovY), nearPlane, farPlane);
        }
    }
    else if (projType == Camera::ORTHO)
    {
        if (newFrustumHeight > 0)
        {
            m_camera->setProjectionAsOrtho(newFrustumHeight, nearPlane, farPlane);
        }
    }


    m_lastPosX = posX;
    m_lastPosY = posY;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Rotate
/// 
/// \return  Returns true if input caused changes to the camera an false if no changes occurred
//--------------------------------------------------------------------------------------------------
bool ManipulatorTrackball::rotate(int posX, int posY)
{
    if (m_camera.isNull()) return false;
    if (posX == m_lastPosX && posY == m_lastPosY) return false;

    const double vpPixSizeX = m_camera->viewport()->width();
    const double vpPixSizeY = m_camera->viewport()->height();
    if (vpPixSizeX <= 0 || vpPixSizeY <= 0) return false;

    const double vpPosX     = posX       - static_cast<int>(m_camera->viewport()->x());
    const double vpPosY     = posY       - static_cast<int>(m_camera->viewport()->y());
    const double vpLastPosX = m_lastPosX - static_cast<int>(m_camera->viewport()->x());
    const double vpLastPosY = m_lastPosY - static_cast<int>(m_camera->viewport()->y());

    // Scale the new/last positions to the range [-1.0, 1.0] 
    double newPosX =  2.0*(vpPosX/vpPixSizeX) - 1.0;
    double newPosY =  2.0*(vpPosY/vpPixSizeY) - 1.0;
    double lastPosX = 2.0*(vpLastPosX/vpPixSizeX) - 1.0;
    double lastPosY = 2.0*(vpLastPosY/vpPixSizeY) - 1.0;

    Mat4d viewMat = m_camera->viewMatrix();

    // Compute rotation quaternion
    Quatd rotQuat = trackballRotation(lastPosX, lastPosY, newPosX, newPosY, viewMat, m_rotateSensitivity);

    // Update navigation by modifying the view matrix
    Mat4d rotMatr = rotQuat.toMatrix4();
    rotMatr.translatePostMultiply(-m_rotationPoint);
    rotMatr.translatePreMultiply(m_rotationPoint);

    viewMat = viewMat*rotMatr;
    m_camera->setViewMatrix(viewMat);

    m_lastPosX = posX;
    m_lastPosY = posY;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Compute quaternion rotation
///
/// \param  oldPosX            x coordinate of the last position of the mouse, in the range [-1.0, 1.0]
/// \param  oldPosY            y coordinate of the last position of the mouse, in the range [-1.0, 1.0]
/// \param  newPosX            x coordinate of current position of the mouse, in the range [-1.0, 1.0]
/// \param  newPosY            y coordinate of current position of the mouse, in the range [-1.0, 1.0]
/// \param  currViewMatrix     Current transformation matrix. The inverse is used when calculating the rotation
/// \param  sensitivityFactor  Mouse sensitivity factor
///
/// Simulate a track-ball. Project the points onto the virtual trackball, then figure out the axis 
/// of rotation. This is a deformed trackball-- is a trackball in the center, but is deformed into a 
/// hyperbolic sheet of rotation away from the center.  
//--------------------------------------------------------------------------------------------------
Quatd ManipulatorTrackball::trackballRotation(double oldPosX, double oldPosY, double newPosX, double newPosY, const Mat4d& currViewMatrix, double sensitivityFactor)
{
    // This particular function was chosen after trying out several variations.
    // Implemented by Gavin Bell, lots of ideas from Thant Tessman and	the August '88 
    // issue of Siggraph's "Computer Graphics," pp. 121-129.

    // This size should really be based on the distance from the center of rotation to the point on 
    // the object underneath the mouse.  That point would then track the mouse as closely as possible.  
    const double TRACKBALL_RADIUS = 0.8f;

    // Clamp to valid range
    oldPosX = Math::clamp(oldPosX, -1.0, 1.0);
    oldPosY = Math::clamp(oldPosY, -1.0, 1.0);
    newPosX = Math::clamp(newPosX, -1.0, 1.0);
    newPosY = Math::clamp(newPosY, -1.0, 1.0);

    // First, figure out z-coordinates for projection of P1 and P2 to deformed sphere
    Vec3d p1 = projectToSphere(TRACKBALL_RADIUS, oldPosX, oldPosY);
    Vec3d p2 = projectToSphere(TRACKBALL_RADIUS, newPosX, newPosY);

    // Axis of rotation is the cross product of P1 and P2
    Vec3d a = p1 ^ p2; 

    // Figure out how much to rotate around that axis.
    Vec3d d = p1 - p2;
    double t = d.length()/(2.0*TRACKBALL_RADIUS);

    // Avoid problems with out-of-control values...
    t = Math::clamp(t, -1.0, 1.0);

    double phi = 2.0*Math::asin(t);

    // Scale by sensitivity factor
    phi *= sensitivityFactor;

    // Use inverted matrix to find rotation axis
    Mat4d invMatr = currViewMatrix.getInverted();
    a.transformVector(invMatr);

    // Get quaternion to be returned by pointer
    Quatd quat = Quatd::fromAxisAngle(a, phi);
    return quat;
}



//--------------------------------------------------------------------------------------------------
/// Project an x,y pair onto sphere 
///
/// \param radius   Radius of the sphere
/// \param posX     X coordinate
/// \param posY     Y coordinate
/// 
/// \return	Vector containing the projected point
/// 
/// Projects the x,y pair onto a sphere with the given radius \b or a hyperbolic sheet if we 
/// are away from the center of the sphere. 
//--------------------------------------------------------------------------------------------------
Vec3d ManipulatorTrackball::projectToSphere(double radius, double posX, double posY)
{
    double d = Math::sqrt(posX*posX + posY*posY);

    if (d < radius*SQRT1_2_D)  	
    {
        // Inside sphere 
        double projectedZ = Math::sqrt(radius*radius - d*d);

        return Vec3d(posX, posY, projectedZ);
    }
    else
    {	
        // On hyperbola 
        double t = radius/SQRT2_D;
        double projectedZ = t*t/d;

        return Vec3d(posX, posY, projectedZ);
    }
}

//--------------------------------------------------------------------------------------------------
/// Repositions and orients the camera to view the rotation point along the 
/// direction "alongDirection". The distance to the rotation point is maintained.
///
//--------------------------------------------------------------------------------------------------
void ManipulatorTrackball::setView( const Vec3d& alongDirection, const Vec3d& upDirection )
{
    if (m_camera.isNull()) return;

    Vec3d dir = alongDirection;
    if (!dir.normalize()) return;
    Vec3d up = upDirection;
    if(!up.normalize()) up = Vec3d::Z_AXIS;
    
    if((up * dir) < 1e-2) up = dir.perpendicularVector();

    Vec3d cToE = m_camera->position() - m_rotationPoint;
    Vec3d newEye = m_rotationPoint - cToE.length() * dir;

    m_camera->setFromLookAt(newEye, m_rotationPoint, upDirection);
}


} // namespace cvf

