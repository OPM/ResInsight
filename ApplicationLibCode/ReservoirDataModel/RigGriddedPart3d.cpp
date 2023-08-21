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
    : m_thickness( 10.0 )
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
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::generateGeometry( std::vector<cvf::Vec3d> inputPoints, int nHorzCells, int nVertCellsLower, int nVertCellsMiddle, int nVertCellsUpper )
{
    m_borderSurfaceElements.clear();
    m_vertices.clear();
    m_elementIndices.clear();

    cvf::Vec3d step0to1 = stepVector( inputPoints[0], inputPoints[1], nVertCellsLower );
    cvf::Vec3d step1to2 = stepVector( inputPoints[1], inputPoints[2], nVertCellsMiddle );
    cvf::Vec3d step2to3 = stepVector( inputPoints[2], inputPoints[3], nVertCellsUpper );

    cvf::Vec3d step4to5 = stepVector( inputPoints[4], inputPoints[5], nVertCellsLower );
    cvf::Vec3d step5to6 = stepVector( inputPoints[5], inputPoints[6], nVertCellsMiddle );
    cvf::Vec3d step6to7 = stepVector( inputPoints[6], inputPoints[7], nVertCellsUpper );

    cvf::Vec3d step0to4 = stepVector( inputPoints[0], inputPoints[4], nHorzCells );
    cvf::Vec3d step1to5 = stepVector( inputPoints[1], inputPoints[5], nHorzCells );
    cvf::Vec3d step2to6 = stepVector( inputPoints[2], inputPoints[6], nHorzCells );
    cvf::Vec3d step3to7 = stepVector( inputPoints[3], inputPoints[7], nHorzCells );

    cvf::Vec3d tVec = step0to4 ^ step0to1;
    tVec.normalize();
    tVec *= m_thickness;
    const std::vector<double> m_thicknessFactors = { -1.0, 0.0, 1.0 };
    const int                 nThicknessCells    = 2;
    const int                 nVertCells         = nVertCellsLower + nVertCellsMiddle + nVertCellsUpper;

    const std::vector<int>        vertCells  = { nVertCellsLower, nVertCellsMiddle, nVertCellsUpper + 1 };
    const std::vector<cvf::Vec3d> firstSteps = { step0to1, step1to2, step2to3 };
    const std::vector<cvf::Vec3d> lastSteps  = { step4to5, step5to6, step6to7 };

    // ** generate vertices

    m_vertices.reserve( (size_t)( ( nVertCells + 1 ) * ( nHorzCells + 1 ) ) );

    cvf::Vec3d p     = inputPoints[0];
    cvf::Vec3d pLast = inputPoints[4];

    for ( int i = 0; i < (int)sizeof( vertCells ); i++ )
    {
        for ( int v = 0; v < vertCells[i]; v++ )
        {
            cvf::Vec3d stepHorz = stepVector( p, pLast, nHorzCells );
            cvf::Vec3d p2       = p;
            for ( int h = 0; h <= nHorzCells; h++ )
            {
                for ( int t = 0; t <= (int)m_thicknessFactors.size(); t++ )
                {
                    m_vertices.push_back( p2 + m_thicknessFactors[t] * tVec );
                }

                p2 += stepHorz;
                pLast = p2;
            }
            p += firstSteps[i];
            pLast += lastSteps[i];
        }
    }

    // ** generate elements of type hex8

    m_elementIndices.reserve( (size_t)( ( nVertCellsLower + nVertCellsMiddle + nVertCellsUpper ) * nHorzCells * 8 ) );

    int index = 0;

    const int nextLayerIdxOff = ( nHorzCells + 1 ) * ( nThicknessCells + 1 );

    for ( int v = 0; v < nVertCells; v++ )
    {
        int i = index;
        for ( int h = 0; h < nHorzCells; h++, i++ )
        {
            for ( int t = 0; t < nThicknessCells; t++ )
            {
                m_elementIndices.push_back( i + nHorzCells );
                m_elementIndices.push_back( i + nHorzCells + 1 );
                m_elementIndices.push_back( i + 1 );
                m_elementIndices.push_back( i );

                m_elementIndices.push_back( i + nHorzCells + ( nThicknessCells + 1 ) * nHorzCells );
                m_elementIndices.push_back( i + nHorzCells + ( nThicknessCells + 1 ) * nHorzCells + 1 );
            }
        }
        index += nHorzCells + 1;
    }

    // ** generate indices

    // m_elementIndices.reserve( (size_t)( numVertCells * numHorzCells * 4 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigGriddedPart3d::vertices() const
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
/// Output elements will be of type HEX8
///
///     7---------6
///    /|        /|
///   / |       / |
///  4---------5  |     z
///  |  3------|--2       |   y
///  | /       | /        | /
///  |/        |/         |/
///  0---------1           ----- x
///
//--------------------------------------------------------------------------------------------------
const std::vector<unsigned int>& RigGriddedPart3d::elementIndices() const
{
    return m_elementIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RigGriddedPart3d::BorderSurface, std::vector<unsigned int>>& RigGriddedPart3d::borderSurfaceElements() const
{
    return m_borderSurfaceElements;
}
