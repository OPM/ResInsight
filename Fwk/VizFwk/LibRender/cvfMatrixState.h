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

#include "cvfMatrix4.h"

namespace cvf {

class Camera;


//==================================================================================================
//
// 
//
//==================================================================================================
class MatrixState
{
public:
    MatrixState(const Camera& camera);
    MatrixState(const Vec2i& viewportPosition, const Vec2ui& viewportSize, const Mat4d& projectionMatrix, const Mat4d& viewMatrix);

    void            setViewMatrix(const Mat4d& viewMatrix);

    void            setModelMatrix(const Mat4d& modelMatrix);
    void            clearModelMatrix();

    const Mat4f&    projectionMatrix() const;
    const Mat4f&    viewMatrix() const;
    Mat4f           viewMatrixInverse() const;
    const Mat4f&    modelMatrix() const;
    Mat4f           modelMatrixInverse() const;
    Mat4f           modelMatrixInverseTranspose() const;
    Mat4f           modelViewMatrix() const;
    Mat4f           modelViewMatrixInverse() const;
    Mat4f           modelViewProjectionMatrix() const;
    Mat4f           modelViewProjectionMatrixInverse() const;

    Mat3f           normalMatrix() const;

    float           pixelHeightAtUnitDistance() const;
    Vec2i           viewportPosition() const;
    Vec2ui          viewportSize() const;

    uint            versionTick() const;

private:
    void            computePixelHeight();

private:
    Mat4f   m_projectionMatrix;         // Float version of projection matrix (as specified by user)
    Mat4f   m_viewMatrix;               // Float version of view matrix (as specified by user)
    Mat4f   m_modelMatrix;              // Float version of model matrix (as specified by user)
    bool    m_modelMatrixIsSet;         // Set to true when a model matrix is set, false when no model matrix is currently set
    Mat4f   m_viewProjectionMatrix;     // Combined view matrix and projection matrix

    Vec2i   m_viewportPosition;         // Position of the viewport
    Vec2ui  m_viewportSize;             // Size of the viewport
    float   m_pixelHeightAtUnitDistance;// Height of a pixel at unit distance (distance of 1.0 from camera) in world system. For perspective projections this value must be multiplied by the distance to the point in question

    uint    m_versionTick;              // Versioning to be able to detect changes
};




}
