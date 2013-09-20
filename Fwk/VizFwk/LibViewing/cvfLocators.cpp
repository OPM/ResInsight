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

    if (x == m_lastPosX && y == m_lastPosY) return false;

    const double vpPixSizeX = m_camera->viewport()->width();
    const double vpPixSizeY = m_camera->viewport()->height();
    if (vpPixSizeX <= 0 || vpPixSizeY <= 0) return false;

    // Normalized movement in screen plane 
    const double tx = (x - m_lastPosX)/vpPixSizeX;
    const double ty = (y - m_lastPosY)/vpPixSizeY;

    Vec3d oldPos = m_pos;

    if (m_operation == PAN)
    {
        updatePan(tx, ty);
    }
    else if (m_operation == WALK)
    {
        updateWalk(ty);
    }

    m_lastPosX = x;
    m_lastPosY = y;

    if (m_pos == oldPos)
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
void LocatorPanWalkRotate::updatePan(double tx, double ty)
{
    CVF_ASSERT(m_camera.notNull());

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
void LocatorPanWalkRotate::updateWalk(double ty)
{
    CVF_ASSERT(m_camera.notNull());

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


} // namespace cvf

