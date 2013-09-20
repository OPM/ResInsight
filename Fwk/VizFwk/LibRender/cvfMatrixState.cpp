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
#include "cvfMatrixState.h"
#include "cvfCamera.h"
#include "cvfViewport.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::MatrixState
/// \ingroup Render
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MatrixState::MatrixState(const Camera& camera)
:   m_projectionMatrix(camera.projectionMatrix()),
    m_viewMatrix(camera.viewMatrix()),
    m_modelMatrix(),
    m_modelMatrixIsSet(false),
    m_viewProjectionMatrix(m_projectionMatrix*m_viewMatrix),
    m_viewportPosition(camera.viewport()->x(), camera.viewport()->y()),
    m_viewportSize(static_cast<uint>(camera.viewport()->width()), static_cast<uint>(camera.viewport()->height())),
    m_pixelHeightAtUnitDistance(0.0f),
    m_versionTick(1)
{
    // This won't work for orthographic
    //m_pixelSize = static_cast<float>(2.0*Math::tan(Math::toRadians(camera.fieldOfViewYDeg()/2.0))/m_viewportSize.y());

    computePixelHeight();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MatrixState::MatrixState(const Vec2i& viewportPosition, const Vec2ui& viewportSize, const Mat4d& projectionMatrix, const Mat4d& viewMatrix)
:   m_projectionMatrix(projectionMatrix),
    m_viewMatrix(viewMatrix),
    m_modelMatrix(),
    m_modelMatrixIsSet(false),
    m_viewProjectionMatrix(m_projectionMatrix*m_viewMatrix),
    m_viewportPosition(viewportPosition),
    m_viewportSize(viewportSize),
    m_pixelHeightAtUnitDistance(0.0f),
    m_versionTick(1)
{
    computePixelHeight();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MatrixState::setViewMatrix(const Mat4d& viewMatrix)
{
    m_viewMatrix = Mat4f(viewMatrix);
    m_viewProjectionMatrix = m_projectionMatrix*m_viewMatrix;
    m_versionTick++;
}


//--------------------------------------------------------------------------------------------------
/// Computes height of a pixel at unit distance in world system
//--------------------------------------------------------------------------------------------------
void MatrixState::computePixelHeight()
{
    // We're computing a pixelSize at a distance of 1 from camera/eye.
    // Strategy is to transform a sample point using the projection matrix, and then determine it's relative height in the viewport
    Vec4f p = m_projectionMatrix*Vec4f(0, 1, -1, 1);
    p /= p.w();

    // We're now in normalized device coordinates. (cube of (-1, 1))
    // Divide to find relative height
    float relHeight = p.y()/2.0f;
    float viewportHeight = static_cast<float>(m_viewportSize.y());
    if (relHeight*viewportHeight > 0.0f)
    {
        m_pixelHeightAtUnitDistance = 1.0f/(relHeight*viewportHeight);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MatrixState::setModelMatrix(const Mat4d& modelMatrix)
{
    m_modelMatrix.set(modelMatrix);
    m_modelMatrixIsSet = true;
    m_versionTick++;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MatrixState::clearModelMatrix()
{
    if (m_modelMatrixIsSet)
    {
        m_modelMatrixIsSet = false;
        m_versionTick++;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Mat4f& MatrixState::projectionMatrix() const
{
    return m_projectionMatrix;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Mat4f& MatrixState::viewMatrix() const
{
    return m_viewMatrix;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Mat4f MatrixState::viewMatrixInverse() const
{
    return m_viewMatrix.getInverted();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Mat4f& MatrixState::modelMatrix() const
{
    if (m_modelMatrixIsSet)
    {
        return m_modelMatrix;
    }
    else
    {
        return Mat4f::IDENTITY;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mat4f MatrixState::modelMatrixInverse() const
{
    if (m_modelMatrixIsSet)
    {
        return m_modelMatrix.getInverted();
    }
    else
    {
        return Mat4f::IDENTITY;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mat4f MatrixState::modelMatrixInverseTranspose() const
{
    if (m_modelMatrixIsSet)
    {
        return m_modelMatrix.getInverted().getTransposed();
    }
    else
    {
        return Mat4f::IDENTITY;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mat4f MatrixState::modelViewMatrix() const
{
    if (m_modelMatrixIsSet)
    {
        return m_viewMatrix*m_modelMatrix;
    }
    else
    {
        return m_viewMatrix;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Mat4f MatrixState::modelViewMatrixInverse() const
{
    if (m_modelMatrixIsSet)
    {
        return (m_viewMatrix*m_modelMatrix).getInverted();
    }
    else
    {
        return m_viewMatrix.getInverted();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mat4f MatrixState::modelViewProjectionMatrix() const
{
    if (m_modelMatrixIsSet)
    {
        return m_viewProjectionMatrix*m_modelMatrix;
    }
    else
    {
        return m_viewProjectionMatrix;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mat4f MatrixState::modelViewProjectionMatrixInverse() const
{
    if (m_modelMatrixIsSet)
    {
        return (m_viewProjectionMatrix*m_modelMatrix).getInverted();
    }
    else
    {
        return m_viewProjectionMatrix.getInverted();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mat3f MatrixState::normalMatrix() const
{
    Mat3f normMatr = modelViewMatrix().toMatrix3();
    normMatr.invert();
    normMatr.transpose();
    return normMatr;
}


//--------------------------------------------------------------------------------------------------
/// Height (size) of a pixel at unit distance in world system. 
/// 
/// For perspective projections this value must be multiplied by the distance to the point in question
//--------------------------------------------------------------------------------------------------
float MatrixState::pixelHeightAtUnitDistance() const
{
    return m_pixelHeightAtUnitDistance;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2i MatrixState::viewportPosition() const
{
    return m_viewportPosition;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2ui MatrixState::viewportSize() const
{
    return m_viewportSize;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint MatrixState::versionTick() const
{
    return m_versionTick;
}


} // namespace cvf

