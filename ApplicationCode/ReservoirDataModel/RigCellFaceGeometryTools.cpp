/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RigCellFaceGeometryTools.h"

#include "RigCell.h"
#include "RigMainGrid.h"

#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::StructGridInterface::FaceType
    RigCellFaceGeometryTools::calculateCellFaceOverlap( const RigCell&           c1,
                                                        const RigCell&           c2,
                                                        const RigMainGrid&       mainGrid,
                                                        std::vector<size_t>*     connectionPolygon,
                                                        std::vector<cvf::Vec3d>* connectionIntersections )
{
    // Try to find the shared face

    bool isPossibleNeighborInDirection[6] = {true, true, true, true, true, true};

    if ( c1.hostGrid() == c2.hostGrid() )
    {
        char hasNeighbourInAnyDirection = 0;

        size_t i1, j1, k1;
        c1.hostGrid()->ijkFromCellIndex( c1.gridLocalCellIndex(), &i1, &j1, &k1 );
        size_t i2, j2, k2;
        c2.hostGrid()->ijkFromCellIndex( c2.gridLocalCellIndex(), &i2, &j2, &k2 );

        isPossibleNeighborInDirection[cvf::StructGridInterface::POS_I] = ( ( i1 + 1 ) == i2 );
        isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_I] = ( ( i2 + 1 ) == i1 );
        isPossibleNeighborInDirection[cvf::StructGridInterface::POS_J] = ( ( j1 + 1 ) == j2 );
        isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_J] = ( ( j2 + 1 ) == j1 );
        isPossibleNeighborInDirection[cvf::StructGridInterface::POS_K] = ( ( k1 + 1 ) == k2 );
        isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_K] = ( ( k2 + 1 ) == k1 );

        hasNeighbourInAnyDirection = isPossibleNeighborInDirection[cvf::StructGridInterface::POS_I] +
                                     isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_I] +
                                     isPossibleNeighborInDirection[cvf::StructGridInterface::POS_J] +
                                     isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_J] +
                                     isPossibleNeighborInDirection[cvf::StructGridInterface::POS_K] +
                                     isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_K];

        // If cell 2 is not adjancent with respect to any of the six ijk directions,
        // assume that we have no overlapping area.

        if ( !hasNeighbourInAnyDirection )
        {
            // Add to search map
            // m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c1GlobIdx][cvf::StructGridInterface::NO_FACE].push_back(cnIdx);
            // m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c2GlobIdx][cvf::StructGridInterface::NO_FACE].push_back(cnIdx);

            // cvf::Trace::show("NNC: No direct neighbors : C1: " + cvf::String((int)m_connections[cnIdx].m_c1GlobIdx) +
            // " C2: " + cvf::String((int)m_connections[cnIdx].m_c2GlobIdx));
            return cvf::StructGridInterface::NO_FACE;
        }
    }

    for ( unsigned char fIdx = 0; fIdx < 6; ++fIdx )
    {
        if ( !isPossibleNeighborInDirection[fIdx] )
        {
            continue;
        }

        // Calculate connection polygon

        std::vector<size_t>     polygon;
        std::vector<cvf::Vec3d> intersections;
        std::array<size_t, 4>   face1;
        std::array<size_t, 4>   face2;
        c1.faceIndices( ( cvf::StructGridInterface::FaceType )( fIdx ), &face1 );
        c2.faceIndices( cvf::StructGridInterface::oppositeFace( ( cvf::StructGridInterface::FaceType )( fIdx ) ), &face2 );

        bool foundOverlap =
            cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                   &intersections,
                                                                   (cvf::EdgeIntersectStorage<size_t>*)nullptr,
                                                                   cvf::wrapArrayConst( &mainGrid.nodes() ),
                                                                   face1.data(),
                                                                   face2.data(),
                                                                   1e-6 );

        if ( foundOverlap )
        {
            if ( connectionPolygon ) ( *connectionPolygon ) = polygon;
            if ( connectionIntersections ) ( *connectionIntersections ) = intersections;
            return ( cvf::StructGridInterface::FaceType )( fIdx );
        }
    }

    return cvf::StructGridInterface::NO_FACE;
}
