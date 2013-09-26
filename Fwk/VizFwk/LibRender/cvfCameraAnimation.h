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


#pragma once

#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfTimer.h"

namespace cvf {


//==================================================================================================
//
// Camera animation class
//
//==================================================================================================
class CameraAnimation : public Object
{
public:
    CameraAnimation(const Vec3d& currentPos, const Vec3d& currentDir, const Vec3d& currentUp, const Vec3d& newPos, const Vec3d& newDir, const Vec3d& newUp);

    void setDuration(double seconds);

    bool newPosition(Vec3d* pos, Vec3d* dir, Vec3d* up, double* relativeTimeStamp = NULL);

private:
    Vec3d m_currentPos;
    Vec3d m_currentDir;
    Vec3d m_currentUp;
    Vec3d m_newPos;
    Vec3d m_newDir;
    Vec3d m_newUp;

    ref<Timer> m_timer;
    double m_animDuration;
    bool m_animDone;
};

}
