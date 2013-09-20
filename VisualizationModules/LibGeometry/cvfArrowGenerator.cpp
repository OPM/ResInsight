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
#include "cvfArrowGenerator.h"
#include "cvfGeometryUtils.h"

namespace cvf {

//==================================================================================================
///
/// \class cvf::ArrowGenerator
/// \ingroup Geometry
///
/// Creates an arrow starting in origin and pointing in the direction of the +Z axis 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ArrowGenerator::ArrowGenerator()
:   m_shaftRelativeRadius(0.025f),
    m_headRelativeRadius(0.085f),
    m_headRelativeLength(0.25f),
    m_numSlices(20)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ArrowGenerator::setShaftRelativeRadius(float shaftRelativeRadius)
{
    m_shaftRelativeRadius = shaftRelativeRadius;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ArrowGenerator::setHeadRelativeRadius(float headRelativeRadius)
{
    m_headRelativeRadius = headRelativeRadius;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ArrowGenerator::setHeadRelativeLength(float headRelativeLength)
{
    m_headRelativeLength = headRelativeLength;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ArrowGenerator::setNumSlices(uint numSlices)
{
    m_numSlices = numSlices;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ArrowGenerator::generate(GeometryBuilder* builder)
{
    const uint numPolysZDir = 1;

    GeometryUtils::createObliqueCylinder(m_shaftRelativeRadius, m_shaftRelativeRadius, 1.0f - m_headRelativeLength, 0, 0, m_numSlices, true, true, false, numPolysZDir, builder);

    uint startIdx = builder->vertexCount();
    GeometryUtils::createCone(m_headRelativeRadius, m_headRelativeLength, m_numSlices, true, true, true, builder);
    uint endIdx = builder->vertexCount() - 1;

    Mat4f mat = Mat4f::fromTranslation(Vec3f(0, 0, 1.0f - m_headRelativeLength));
    builder->transformVertexRange(startIdx, endIdx, mat);
}

} // namespace cvf
