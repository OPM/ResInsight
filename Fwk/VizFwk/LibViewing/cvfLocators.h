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

#include "cvfPlane.h"
#include "cvfQuat.h"

namespace cvf {

class Camera;


//==================================================================================================
//
// 
//
//==================================================================================================
class Locator : public Object
{
public:
    Locator() {};

    virtual Vec3d   position() const = 0;
    virtual void    start(int x, int y) = 0;
    virtual bool    update(int x, int y) = 0;
};



//==================================================================================================
//
// Locator for translating a point limited by a plane
//
//==================================================================================================
class LocatorTranslateOnPlane : public Locator
{
public:
    LocatorTranslateOnPlane(Camera* camera);
    ~LocatorTranslateOnPlane();

    void            setPosition(const Vec3d& position, const Vec3d& planeNormal);
    virtual Vec3d   position() const;
    virtual void    start(int x, int y);
    virtual bool    update(int x, int y);

public:
    ref<Camera> m_camera;               
    Plane       m_plane;    // Default plane is XY plane
    Vec3d       m_pos;        
    Vec3d       m_lastPos;        
};



//==================================================================================================
//
// 
//
//==================================================================================================
class LocatorPanWalkRotate : public Locator
{
public:
    enum Operation
    {
        PAN,
        WALK,
        ROTATE
    };

public:
    LocatorPanWalkRotate(Camera* camera);
    ~LocatorPanWalkRotate();

    void            setOperation(Operation op);

    void            setPosition(const Vec3d& position);
    virtual Vec3d   position() const;
    void            setOrientation(const Mat3d& m);
    Mat3d           orientation() const;

    virtual void    start(int x, int y);
    virtual bool    update(int x, int y);

private:
    void            updatePan(int oglWinCoordX, int oglWinCoordY);
    void            updateWalk(int oglWinCoordY);
    void            updateRotation(int oglWinCoordX, int oglWinCoordY);

    static Quatd    trackballRotation(double oldPosX, double oldPosY, double newPosX, double newPosY, const Mat4d& currViewMatrix, double trackballRadius, double sensitivityFactor);
    static Vec3d    projectToSphere(double radius, double posX, double posY);

private:
    ref<Camera> m_camera;       
    Operation   m_operation;    // Default operation is PAN
    Vec3d       m_pos;        
    Quatd       m_rotQuat;
    int         m_lastPosX;     // Last position, window coords, origin lower left (OpenGL style)
    int         m_lastPosY;         
};




// Ideas from name discussion regarding other locator classes
//class LocatorTranslateOnLine;
//class LocatorRotateRoundAxis;


}
