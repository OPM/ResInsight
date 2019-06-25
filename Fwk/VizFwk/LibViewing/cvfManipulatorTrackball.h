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
#include "cvfQuat.h"
#include "cvfMatrix4.h"

namespace cvf {

class Camera;


//==================================================================================================
//
// 
//
//==================================================================================================
class ManipulatorTrackball : public Object
{
public:
    enum NavigationType
    {
        NONE,
        PAN,
        WALK,
        ZOOM,
        ROTATE
    };

public:
    ManipulatorTrackball();
    ~ManipulatorTrackball();

    void            setCamera(Camera* camera);
    void            setRotationPoint(const Vec3d& rotPoint);
    void            setView(const Vec3d& alongDirection, const Vec3d& upDirection);

    NavigationType  activeNavigation() const;
    void            startNavigation(NavigationType navigationType, int x, int y);
    bool            updateNavigation(int x, int y);
    void            endNavigation();
    void            setRotationSensitivity(double scaleFactor);

private:
    bool            pan(int posX, int posY);
    bool            walk(int posX, int posY);
    bool            zoom(int posX, int posY);
    bool            rotate(int posX, int posY);

    static Quatd    trackballRotation(double oldPosX, double oldPosY, double newPosX, double newPosY, const Mat4d& currViewMatrix, double sensitivityFactor);
    static Vec3d    projectToSphere(double radius, double posX, double posY);

private:
    ref<Camera>     m_camera;               // Camera that should be manipulated
    Vec3d           m_rotationPoint;        // Rotation point. Used both for rotations and to control pan/walk speed

    NavigationType  m_activeNavigation;     // The active navigation type
    int             m_lastPosX;             // Previous mouse position
    int             m_lastPosY;         
    Vec3d           m_walkStartCameraPos;   // Starting camera position for a walk operation

    double          m_walkSensitivity;      // Scale factor for sensitivity of rotations
    double          m_rotateSensitivity;    // Scale factor for walk sensitivity
};

}
