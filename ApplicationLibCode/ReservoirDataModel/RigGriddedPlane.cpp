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

#include "RigGriddedPlane.h"

#include "cvfTextureImage.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGriddedPlane::RigGriddedPlane()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGriddedPlane::~RigGriddedPlane()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGriddedPlane::stepVector( cvf::Vec3d start, cvf::Vec3d stop, int nSteps )
{
    cvf::Vec3d vec = stop - start;
    return vec.getNormalized() * ( vec.length() / nSteps );
}

//--------------------------------------------------------------------------------------------------
///  Point index in input
///
///     1 ____________ 2
///      |           /
///      |          /
///      |         /
///      |        /
///      |_______/
///      0         3
///
/// Assumes 0->3 and 1->2 is parallel
//--------------------------------------------------------------------------------------------------
void RigGriddedPlane::generateGeometry( std::vector<cvf::Vec3d> cornerPoints, int numHorzCells, int numVertCells )
{
    cvf::Vec3d step0to1 = stepVector( cornerPoints[0], cornerPoints[1], numVertCells );
    cvf::Vec3d step0to3 = stepVector( cornerPoints[0], cornerPoints[3], numHorzCells );
    cvf::Vec3d step1to2 = stepVector( cornerPoints[1], cornerPoints[2], numHorzCells );
    cvf::Vec3d step3to2 = stepVector( cornerPoints[3], cornerPoints[2], numVertCells );

    // ** generate vertices

    m_vertices.clear();
    m_vertices.reserve( (size_t)( ( numVertCells + 1 ) * ( numHorzCells + 1 ) ) );

    cvf::Vec3d p     = cornerPoints[0];
    cvf::Vec3d pLast = cornerPoints[3];

    for ( int v = 0; v <= numVertCells; v++ )
    {
        cvf::Vec3d stepHorz = stepVector( p, pLast, numHorzCells );
        cvf::Vec3d p2       = p;
        for ( int h = 0; h <= numHorzCells; h++ )
        {
            m_vertices.push_back( p2 );
            p2 += stepHorz;
            pLast = p2;
        }
        p += step0to1;
        pLast += step3to2;
    }

    // ** generate indices

    m_quadIndices.clear();
    m_quadIndices.reserve( (size_t)( numVertCells * numHorzCells * 4 ) );

    int index = 0;

    for ( int v = 0; v < numVertCells; v++ )
    {
        int i = index;
        for ( int h = 0; h < numHorzCells; h++, i++ )
        {
            m_quadIndices.push_back( i );
            m_quadIndices.push_back( i + numHorzCells + 1 );
            m_quadIndices.push_back( i + numHorzCells + 2 );
            m_quadIndices.push_back( i + 1 );
        }
        index += numHorzCells + 1;
    }

    // ** generate mesh lines

    m_meshLines.clear();

    // horizontal lines

    cvf::Vec3d startP = cornerPoints[0];
    cvf::Vec3d endP   = cornerPoints[3];

    for ( int v = 0; v <= numVertCells; v++ )
    {
        m_meshLines.push_back( { startP, endP } );
        startP += step0to1;
        endP += step3to2;
    }

    // vertical lines

    startP = cornerPoints[0];
    endP   = cornerPoints[1];

    for ( int h = 0; h <= numHorzCells; h++ )
    {
        m_meshLines.push_back( { startP, endP } );
        startP += step0to3;
        endP += step1to2;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigGriddedPlane::vertices() const
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<unsigned int>& RigGriddedPlane::quadIndices() const
{
    return m_quadIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<cvf::Vec3d>>& RigGriddedPlane::meshLines() const
{
    return m_meshLines;
}
