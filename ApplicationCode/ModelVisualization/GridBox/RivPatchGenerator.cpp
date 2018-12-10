/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////


#include "RivPatchGenerator.h"

#include "cvfGeometryUtils.h"
#include "cvfArray.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivPatchGenerator::RivPatchGenerator()
:   m_origin(0, 0, 0),
    m_axisU(cvf::Vec3d::X_AXIS),
    m_axisV(cvf::Vec3d::Y_AXIS),
    m_useQuads(true),
    m_windingCCW(true)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPatchGenerator::setOrigin(const cvf::Vec3d& origin)
{
    m_origin = origin;
}

//--------------------------------------------------------------------------------------------------
/// Set the axes
/// 
/// The specified axes will be normalized
//--------------------------------------------------------------------------------------------------
void RivPatchGenerator::setAxes(const cvf::Vec3d& axisU, const cvf::Vec3d& axisV)
{
    m_axisU = axisU.getNormalized();
    m_axisV = axisV.getNormalized();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPatchGenerator::setSubdivisions(const std::vector<double>& uValues, const std::vector<double>& vValues)
{
    m_uValues = uValues;
    m_vValues = vValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPatchGenerator::generate(cvf::GeometryBuilder* builder)
{
    CVF_ASSERT(m_uValues.size() > 0);
    CVF_ASSERT(m_vValues.size() > 0);

    size_t numVertices = m_uValues.size() * m_vValues.size();

    cvf::Vec3fArray vertices;
    vertices.reserve(numVertices);
    
    for (size_t v = 0; v < m_vValues.size(); v++)
    {
        cvf::Vec3d rowOrigo(m_origin + m_axisV * m_vValues[v]);

        for (size_t u = 0; u < m_uValues.size(); u++)
        {
            vertices.add(cvf::Vec3f(rowOrigo + m_axisU * m_uValues[u]));
        }
    }

    cvf::uint baseNodeIdx = builder->addVertices(vertices);

    if (m_useQuads)
    {
        cvf::UIntArray conn;
        cvf::GeometryUtils::tesselatePatchAsQuads(static_cast<int>(m_uValues.size()), static_cast<int>(m_vValues.size()), baseNodeIdx, m_windingCCW, &conn);
        builder->addQuads(conn);
    }
    else
    {
        cvf::UIntArray conn;
        cvf::GeometryUtils::tesselatePatchAsTriangles(static_cast<int>(m_uValues.size()), static_cast<int>(m_vValues.size()), baseNodeIdx, m_windingCCW, &conn);
        builder->addTriangles(conn);
    }
}

