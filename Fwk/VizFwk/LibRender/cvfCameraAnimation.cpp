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
#include "cvfCameraAnimation.h"
#include "cvfCamera.h"
#include "cvfTimer.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::CameraAnimation
/// \ingroup Render
///
/// Support class for creating a camera path animation from one camera configuration to another.
///
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// Configures the animation with start and end point
//--------------------------------------------------------------------------------------------------
CameraAnimation::CameraAnimation(const Vec3d& currentPos, const Vec3d& currentDir, const Vec3d& currentUp, const Vec3d& newPos, const Vec3d& newDir, const Vec3d& newUp)
:   m_currentPos(currentPos),
    m_currentDir(currentDir),
    m_currentUp(currentUp),
    m_newPos(newPos),
    m_newDir(newDir),
    m_newUp(newUp),
    m_animDuration(0.75),
    m_animDone(false)
{
}


//--------------------------------------------------------------------------------------------------
/// Returns the next position in the animation. After the duration of the animation is used up, 
/// the method returns the end point and after that always returns false.
/// 
/// So usage would be:
/// \code
/// cvf::Vec3d pos, dir, up;
/// while(anim.newPosition(&pos, &dir, &up))
/// {
///     m_camera->setFromLookAt(pos, pos + dir, up);
///     repaint();
/// }
/// \endcode
//--------------------------------------------------------------------------------------------------
bool CameraAnimation::newPosition(Vec3d* pos, Vec3d* dir, Vec3d* up, double* relativeTimeStamp)
{
    if (m_animDone)
    {
        return false;
    }

    if (m_timer.isNull())
    {
        m_timer = new Timer;
    }

    double timeNow = m_timer->time();

    if (timeNow > m_animDuration)
    {
        *pos = m_newPos;
        *dir = m_newDir;
        *up = m_newUp;
        if (relativeTimeStamp) *relativeTimeStamp = 1.0;

        m_animDone = true;

        return true;
    }

    *pos = m_currentPos + (m_newPos - m_currentPos)*(timeNow/m_animDuration);
    *dir = m_currentDir + (m_newDir - m_currentDir)*(timeNow/m_animDuration);
    *up  = m_currentUp  + (m_newUp  - m_currentUp )*(timeNow/m_animDuration);

    if (relativeTimeStamp) *relativeTimeStamp = timeNow/m_animDuration;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Sets the duration of the animation
//--------------------------------------------------------------------------------------------------
void CameraAnimation::setDuration(double seconds)
{
    m_animDuration = seconds;
}

} // namespace cvf

