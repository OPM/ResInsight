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
#include "cvfLocators.h"
#include "cvfCamera.h"
#include "cvfViewport.h"
#include "cvfRay.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::LocatorTranslateOnPlane
/// \ingroup Viewing
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LocatorTranslateOnPlane::LocatorTranslateOnPlane(Camera* camera)
:   m_camera(camera),
    m_plane(0, 0, 1, 0),
    m_pos(0, 0, 0),
    m_lastPos(0, 0, 0)
{
    CVF_ASSERT(camera);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LocatorTranslateOnPlane::~LocatorTranslateOnPlane()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LocatorTranslateOnPlane::setPosition(const Vec3d& position, const Vec3d& planeNormal)
{
    m_pos = position;

    m_plane.setFromPointAndNormal(position, planeNormal);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d LocatorTranslateOnPlane::position() const
{
    return m_pos;
}


//--------------------------------------------------------------------------------------------------
/// The window coordinates are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
void LocatorTranslateOnPlane::start(int x, int y)
{
    CVF_ASSERT(m_camera.notNull());
    ref<Ray> ray = m_camera->rayFromWindowCoordinates(x, y);
    
    Vec3d isect(0, 0, 0);
    ray->planeIntersect(m_plane, &isect);

    m_lastPos = isect;
}


//--------------------------------------------------------------------------------------------------
/// The window coordinates are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
bool LocatorTranslateOnPlane::update(int x, int y)
{
    CVF_ASSERT(m_camera.notNull());

    Vec3d oldPos = m_pos;

    ref<Ray> ray = m_camera->rayFromWindowCoordinates(x, y);

    Vec3d isect(0, 0, 0);
    if (ray->planeIntersect(m_plane, &isect))
    {
        Vec3d delta = (isect - m_lastPos);
        m_plane.projectVector(delta, &delta);
        m_pos += delta;

        m_lastPos = isect;
    }

    if (m_pos == oldPos)
    {
        return false;
    }
    else
    {
        return true;
    }
}




//==================================================================================================
///
/// \class cvf::LocatorPanWalkRotate
/// \ingroup Viewing
///
/// Currently, rotate is not supported
/// 
//==================================================================================================
LocatorPanWalkRotate::LocatorPanWalkRotate(Camera* camera)
:   m_camera(camera),
    m_operation(PAN),
    m_pos(0, 0, 0),
    m_rotQuat(0, 0, 0, 1),
    m_lastPosX(0),
    m_lastPosY(0)
{
    CVF_ASSERT(camera);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LocatorPanWalkRotate::~LocatorPanWalkRotate()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LocatorPanWalkRotate::setOperation(Operation op)
{
    m_operation = op;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LocatorPanWalkRotate::setPosition(const Vec3d& position)
{
    m_pos = position;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d LocatorPanWalkRotate::position() const
{
    return m_pos;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LocatorPanWalkRotate::setOrientation(const Mat3d& m)
{
    m_rotQuat = Quatd::fromRotationMatrix(Mat4d(m));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Mat3d LocatorPanWalkRotate::orientation() const
{
    return m_rotQuat.toMatrix3();
}


//--------------------------------------------------------------------------------------------------
/// The window coordinates are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
void LocatorPanWalkRotate::start(int x, int y)
{
    m_lastPosX = x;
    m_lastPosY = y;
}


//--------------------------------------------------------------------------------------------------
/// The window coordinates are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
bool LocatorPanWalkRotate::update(int x, int y)
{
    CVF_ASSERT(m_camera.notNull());

    if (x == m_lastPosX && y == m_lastPosY) 
    {
        return false;
    }

    // Need a non-zero viewport
    if (m_camera->viewport()->width() <= 0 || 
        m_camera->viewport()->height() <= 0)
    {
        return false;
    }


    Vec3d oldPos = m_pos;
    Quatd oldRotQuat = m_rotQuat;

    if (m_operation == PAN)
    {
        updatePan(x, y);
    }
    else if (m_operation == WALK)
    {
        updateWalk(y);
    }
    else if (m_operation == ROTATE)
    {
        updateRotation(x, y);
    }

    m_lastPosX = x;
    m_lastPosY = y;

    if (m_pos == oldPos && m_rotQuat == oldRotQuat)
    {
        return false;
    }
    else
    {
        return true;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LocatorPanWalkRotate::updatePan(int oglWinCoordX, int oglWinCoordY)
{
    CVF_ASSERT(m_camera.notNull());

    // Normalized movement in screen plane [0, 1]
    const double vpPixSizeX = m_camera->viewport()->width();
    const double vpPixSizeY = m_camera->viewport()->height();
    CVF_ASSERT(vpPixSizeX > 0 && vpPixSizeY > 0);
    const double tx = (oglWinCoordX - m_lastPosX)/vpPixSizeX;
    const double ty = (oglWinCoordY - m_lastPosY)/vpPixSizeY;

    // Viewport size in world coordinates
    const double aspect = m_camera->aspectRatio();
    const double vpWorldSizeY = m_camera->frontPlaneFrustumHeight();
    const double vpWorldSizeX = vpWorldSizeY*aspect;

    const Vec3d camUp = m_camera->up();
    const Vec3d camRight = m_camera->right();

    Camera::ProjectionType projType = m_camera->projection();
 
    if (projType == Camera::PERSPECTIVE)
    {
        // Compute distance from camera to point projected onto camera forward direction
        const Vec3d camPos = m_camera->position();
        const Vec3d camDir = m_camera->direction();
        const Vec3d vDiff = m_pos - camPos;
        const double camPointDist = Math::abs(camDir*vDiff);

        const double nearPlane = m_camera->nearPlane();
        Vec3d vX =  camRight*((tx*vpWorldSizeX)/nearPlane)*camPointDist;
        Vec3d vY =  camUp*((ty*vpWorldSizeY)/nearPlane)*camPointDist;
        
        Vec3d translation = vX + vY;
        m_pos += translation;
    }

    else if (projType == Camera::ORTHO)
    {
        Vec3d vX =  camRight*tx*vpWorldSizeX;
        Vec3d vY =  camUp*ty*vpWorldSizeY;

        Vec3d translation = vX + vY;
        m_pos += translation;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LocatorPanWalkRotate::updateWalk(int oglWinCoordY)
{
    CVF_ASSERT(m_camera.notNull());

    // Normalized movement in screen plane [0, 1]
    const double vpPixSizeY = m_camera->viewport()->height();
    CVF_ASSERT(vpPixSizeY > 0);
    const double ty = (oglWinCoordY - m_lastPosY)/vpPixSizeY;

    const double vpWorldSizeY = m_camera->frontPlaneFrustumHeight();
    const Vec3d camDir = m_camera->direction();

    Camera::ProjectionType projType = m_camera->projection();

    // This is the distance that we will move the point in response to a full (whole viewport) movement of the mouse
    // Might need to revisit this to determine target distance
    // In that case it might be an idea to look at the trackball manipulator
    double targetDist = 0;
    if (projType == Camera::PERSPECTIVE)
    {
        // Compute distance from camera to point projected onto camera forward direction
        const Vec3d camPos = m_camera->position();
        const Vec3d vDiff = m_pos - camPos;
        const double camPointDist = Math::abs(camDir*vDiff);

        targetDist = camPointDist;
    }
    else if (projType == Camera::ORTHO)
    {
        targetDist = vpWorldSizeY;
    }

    double moveDist = ty*targetDist;
    Vec3d translation = camDir*moveDist;

    m_pos += translation;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LocatorPanWalkRotate::updateRotation(int oglWinCoordX, int oglWinCoordY)
{
    CVF_ASSERT(m_camera.notNull());

    const double vpPixSizeX = m_camera->viewport()->width();
    const double vpPixSizeY = m_camera->viewport()->height();
    CVF_ASSERT(vpPixSizeX > 0 && vpPixSizeY > 0);

    // Scale the new/last positions to the range [-1.0, 1.0] 
    const double newPosX = 2.0*(oglWinCoordX/vpPixSizeX) - 1.0;
    const double newPosY = 2.0*((vpPixSizeY - oglWinCoordY)/vpPixSizeY) - 1.0;
    const double oldPosX = 2.0*(m_lastPosX/vpPixSizeX) - 1.0;
    const double oldPosY = 2.0*((vpPixSizeY - m_lastPosY)/vpPixSizeY) - 1.0;

    // For now, use hard-coded value for trackball radius and sensitivity
    // Sensitivity could be exposed directly, but trackball rotation needs more consideration.
    // An idea would be for the user to be able to set an approximate size of the locator's visual representation in world coords,
    // and then we could estimate its current size relative to viewport, and then use that as trackball radius.
    // See trackballRotation() for some more info on trackballRadius (we've always used 0.8 as an approximation)
    const double trackballRadius = 0.8;
    const double rotateSensitivity = 1.0;
    Mat4d viewMat = m_camera->viewMatrix();
    Quatd incrementalRotationQuat = trackballRotation(oldPosX, -oldPosY, newPosX, -newPosY, viewMat, trackballRadius, rotateSensitivity);
    
    // Update rotation quaternion
    Mat4d incRotationMatrix = incrementalRotationQuat.toMatrix4();
    incRotationMatrix.translatePostMultiply(-m_pos);
    incRotationMatrix.translatePreMultiply(m_pos);

    Mat4d rotMat = m_rotQuat.toMatrix4();
    rotMat = incRotationMatrix*rotMat;

    m_rotQuat = Quatd::fromRotationMatrix(rotMat);
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
Quatd LocatorPanWalkRotate::trackballRotation(double oldPosX, double oldPosY, double newPosX, double newPosY, const Mat4d& currViewMatrix, double trackballRadius, double sensitivityFactor)
{
    // This particular function was chosen after trying out several variations.
    // Implemented by Gavin Bell, lots of ideas from Thant Tessman and	the August '88 
    // issue of Siggraph's "Computer Graphics," pp. 121-129.

    // This size should really be based on the distance from the center of rotation to the point on 
    // the object underneath the mouse.  That point would then track the mouse as closely as possible.  
    //const double TRACKBALL_RADIUS = 0.8f;

    // Clamp to valid range
    oldPosX = Math::clamp(oldPosX, -1.0, 1.0);
    oldPosY = Math::clamp(oldPosY, -1.0, 1.0);
    newPosX = Math::clamp(newPosX, -1.0, 1.0);
    newPosY = Math::clamp(newPosY, -1.0, 1.0);

    // First, figure out z-coordinates for projection of P1 and P2 to deformed sphere
    Vec3d p1 = projectToSphere(trackballRadius, oldPosX, oldPosY);
    Vec3d p2 = projectToSphere(trackballRadius, newPosX, newPosY);

    // Axis of rotation is the cross product of P1 and P2
    Vec3d a = p1 ^ p2; 

    // Figure out how much to rotate around that axis.
    Vec3d d = p1 - p2;
    double t = d.length()/(2.0*trackballRadius);

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
/// 
//--------------------------------------------------------------------------------------------------
Vec3d LocatorPanWalkRotate::projectToSphere(double radius, double posX, double posY)
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

} // namespace cvf

