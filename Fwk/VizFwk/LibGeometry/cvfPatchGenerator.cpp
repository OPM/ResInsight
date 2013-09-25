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
#include "cvfPatchGenerator.h"
#include "cvfGeometryUtils.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::PatchGenerator
/// \ingroup Geometry
///
/// Generates 2D patches
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PatchGenerator::PatchGenerator()
:   m_origin(0, 0, 0),
    m_axisU(Vec3d::X_AXIS),
    m_axisV(Vec3d::Y_AXIS),
    m_extentU(1),
    m_extentV(1),
    m_cellCountU(1),
    m_cellCountV(1), 
    m_useQuads(true),
    m_windingCCW(true)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PatchGenerator::setOrigin(const Vec3d& origin)
{
    m_origin = origin;
}


//--------------------------------------------------------------------------------------------------
/// Set the axes
/// 
/// The specified axes will be normalized
//--------------------------------------------------------------------------------------------------
void PatchGenerator::setAxes(const Vec3d& axisU, const Vec3d& axisV)
{
    m_axisU = axisU.getNormalized();
    m_axisV = axisV.getNormalized();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PatchGenerator::setExtent(double extentU, double extentV)
{
    m_extentU = extentU;
    m_extentV = extentV;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PatchGenerator::setSubdivisions(uint subdivU, uint subdivV)
{
    CVF_ASSERT(subdivU > 0 && subdivV > 0);
    m_cellCountU = subdivU;
    m_cellCountV = subdivV;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PatchGenerator::setQuads(bool useQuads)
{
    m_useQuads = useQuads;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PatchGenerator::setWindingCCW(bool windingCCW)
{
    m_windingCCW = windingCCW;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PatchGenerator::generate(GeometryBuilder* builder)
{
    CVF_ASSERT(m_cellCountU > 0);
    CVF_ASSERT(m_cellCountV > 0);

    size_t numVertices = (m_cellCountU + 1)*(m_cellCountV + 1);

    Vec3fArray vertices;
    vertices.reserve(numVertices);

    const Vec3d unitU = (m_extentU*m_axisU)/m_cellCountU;
    const Vec3d unitV = (m_extentV*m_axisV)/m_cellCountV;

    uint v;
    for (v = 0; v <= m_cellCountV; v++)
    {
        Vec3d rowOrigo(m_origin + unitV*v);

        uint u;
        for (u = 0; u <= m_cellCountU; u++)
        {
            vertices.add(Vec3f(rowOrigo + unitU*u));
        }
    }

    uint baseNodeIdx = builder->addVertices(vertices);

    if (m_useQuads)
    {
        UIntArray conn;
        GeometryUtils::tesselatePatchAsQuads(m_cellCountU + 1, m_cellCountV + 1, baseNodeIdx, m_windingCCW, &conn);
        builder->addQuads(conn);
    }
    else
    {
        UIntArray conn;
        GeometryUtils::tesselatePatchAsTriangles(m_cellCountU + 1, m_cellCountV + 1, baseNodeIdx, m_windingCCW, &conn);
        builder->addTriangles(conn);
    }

}


} // namespace cvf

