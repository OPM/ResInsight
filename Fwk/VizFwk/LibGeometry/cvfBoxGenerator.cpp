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
#include "cvfBoxGenerator.h"
#include "cvfPatchGenerator.h"
#include "cvfGeometryUtils.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::BoxGenerator
/// \ingroup Geometry
///
/// Generates axis aligned box
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor
/// 
/// Will generate a box with one corner in (0,0,0) and the other corner in (1,1,1). 
/// Tesselation will be done using one quad for each of the box's six faces.
//--------------------------------------------------------------------------------------------------
BoxGenerator::BoxGenerator()
:   m_minCoord(0, 0, 0),
    m_maxCoord(1, 1, 1),
    m_subdivisions(1, 1, 1)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxGenerator::setMinMax(const Vec3d& minCoord, const Vec3d& maxCoord)
{
    m_minCoord = minCoord;
    m_maxCoord = maxCoord;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxGenerator::setOriginAndExtent(const Vec3d& origin, const Vec3d& extent)
{
    m_minCoord = origin;
    m_maxCoord = origin + extent;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxGenerator::setCenterAndExtent(const Vec3d& center, const Vec3d& extent)
{
    m_minCoord = center - extent/2;
    m_maxCoord = center + extent/2;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxGenerator::setSubdivisions(uint subdivX, uint subdivY, uint subdivZ)
{
    CVF_ASSERT(subdivX > 0);
    CVF_ASSERT(subdivY > 0);
    CVF_ASSERT(subdivZ > 0);

    m_subdivisions.set(subdivX, subdivY, subdivZ);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxGenerator::generate(GeometryBuilder* builder)
{
    uint cellCountX = m_subdivisions.x();
    uint cellCountY = m_subdivisions.y();
    uint cellCountZ = m_subdivisions.z();
    CVF_ASSERT(cellCountX > 0);
    CVF_ASSERT(cellCountY > 0);
    CVF_ASSERT(cellCountZ > 0);

    if (cellCountX == 1 && cellCountY == 1 && cellCountZ == 1)
    {
        GeometryUtils::createBox(Vec3f(m_minCoord), Vec3f(m_maxCoord), builder);
        return;
    }


    // Face ordering is
    //
    //     *---------*                Faces:
    //    /|        /|     |z           0 bottom   
    //   / |       / |     | /y         1 top      
    //  *---------*  |     |/           2 front    
    //  |  3------|--*     *---x        3 right    
    //  | /       | /                   4 back     
    //  |/        |/                    5 left     
    //  *---------*                     

    const Vec3d extent(m_maxCoord - m_minCoord);

    PatchGenerator patchGen;

    // Bottom
    patchGen.setOrigin(Vec3d(m_minCoord.x(), m_minCoord.y(), m_minCoord.z()));
    patchGen.setAxes(Vec3d::Y_AXIS, Vec3d::X_AXIS);
    patchGen.setExtent(extent.y(), extent.x());
    patchGen.setSubdivisions(cellCountY, cellCountX);
    patchGen.generate(builder);

    // Top
    patchGen.setOrigin(Vec3d(m_minCoord.x(), m_minCoord.y(), m_maxCoord.z()));
    patchGen.setAxes(Vec3d::X_AXIS, Vec3d::Y_AXIS);
    patchGen.setExtent(extent.x(), extent.y());
    patchGen.setSubdivisions(cellCountX, cellCountY);
    patchGen.generate(builder);

    // Front
    patchGen.setOrigin(Vec3d(m_minCoord.x(), m_minCoord.y(), m_minCoord.z()));
    patchGen.setAxes(Vec3d::X_AXIS, Vec3d::Z_AXIS);
    patchGen.setExtent(extent.x(), extent.z());
    patchGen.setSubdivisions(cellCountX, cellCountZ);
    patchGen.generate(builder);

    // Right
    patchGen.setOrigin(Vec3d(m_maxCoord.x(), m_minCoord.y(), m_minCoord.z()));
    patchGen.setAxes(Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    patchGen.setExtent(extent.y(), extent.z());
    patchGen.setSubdivisions(cellCountY, cellCountZ);
    patchGen.generate(builder);

    // Back
    patchGen.setOrigin(Vec3d(m_maxCoord.x(), m_maxCoord.y(), m_minCoord.z()));
    patchGen.setAxes(-Vec3d::X_AXIS, Vec3d::Z_AXIS);
    patchGen.setExtent(extent.x(), extent.z());
    patchGen.setSubdivisions(cellCountX, cellCountZ);
    patchGen.generate(builder);

    // Left
    patchGen.setOrigin(Vec3d(m_minCoord.x(), m_maxCoord.y(), m_minCoord.z()));
    patchGen.setAxes(-Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    patchGen.setExtent(extent.y(), extent.z());
    patchGen.setSubdivisions(cellCountY, cellCountZ);
    patchGen.generate(builder);
}


} // namespace cvf

