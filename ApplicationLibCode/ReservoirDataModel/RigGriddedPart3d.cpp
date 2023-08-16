/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RigGriddedPart3d.h"

#include "cvfTextureImage.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGriddedPart3d::RigGriddedPart3d()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGriddedPart3d::~RigGriddedPart3d()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGriddedPart3d::stepVector( cvf::Vec3d start, cvf::Vec3d stop, int nSteps )
{
    cvf::Vec3d vec = stop - start;
    return vec.getNormalized() * ( vec.length() / nSteps );
}

//--------------------------------------------------------------------------------------------------
///  Point index in input
///
///
///      3 ----------- 7
///        |         |
///        |         |
///        |         |
///      2 |---------| 6
///        |         \
///        |          \
///        |           \
///      1 -------------| 5
///        |            |
///        |            |
///        |            |
///        |            |
///      0 -------------- 4
///
/// Assumes 0->4, 1->5, 2->6 and 3->7 is parallel
///
///
/// Output elements will be of type HEX8
///
///     3---------2
///    /|        /|
///   / |       / |
///  0---------1  |
///  |  7------|--6
///  | /       | /
///  |/        |/
///  4---------5
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::generateGeometry( std::vector<cvf::Vec3d> cornerPoints,
                                         int                     numHorzCells,
                                         int                     numVertCellsLower,
                                         int                     numVertCellsMiddle,
                                         int                     numVertCellsUpper )
{
    // cvf::Vec3d step0to1 = stepVector( cornerPoints[0], cornerPoints[1], numVertCells );
    // cvf::Vec3d step0to3 = stepVector( cornerPoints[0], cornerPoints[3], numHorzCells );
    // cvf::Vec3d step1to2 = stepVector( cornerPoints[1], cornerPoints[2], numHorzCells );
    // cvf::Vec3d step3to2 = stepVector( cornerPoints[3], cornerPoints[2], numVertCells );

    // ** generate vertices

    m_vertices.clear();
    // m_vertices.reserve( (size_t)( ( numVertCells + 1 ) * ( numHorzCells + 1 ) ) );

    // ** generate indices

    m_elementIndices.clear();
    // m_elementIndices.reserve( (size_t)( numVertCells * numHorzCells * 4 ) );

    m_borderSurfaces.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigGriddedPart3d::vertices() const
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<unsigned int>& RigGriddedPart3d::elementIndices() const
{
    return m_elementIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RigGriddedPart3d::BorderSurface, std::vector<unsigned int>>& RigGriddedPart3d::borderSurfaces() const
{
    return m_borderSurfaces;
}
