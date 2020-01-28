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
#include "RigNncConnection.h"

#include "cvfGeometryTools.h"

#include "cafAssert.h"

#include <QDebug>

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigConnection> RigCellFaceGeometryTools::computeOtherNncs( const RigMainGrid*                mainGrid,
                                                                       const std::vector<RigConnection>& nativeConnections )
{
    // Compute Non-Neighbor Connections (NNC) not reported by Eclipse. NNCs with zero transmissibility are not reported
    // by Eclipse. Use faults as basis for subset of cells to find NNC connection for. The imported connections from
    // Eclipse are located at the beginning of the connections vector.

    std::vector<RigConnection> otherConnections;

    class CellPair
    {
    public:
        CellPair( size_t globalIdx1, size_t globalIdx2 )
        {
            if ( globalIdx1 < globalIdx2 )
            {
                m_globalCellIdx1 = globalIdx1;
                m_globalCellIdx2 = globalIdx2;
            }
            else
            {
                m_globalCellIdx1 = globalIdx2;
                m_globalCellIdx2 = globalIdx1;
            }
        }

        bool operator<( const CellPair& other ) const
        {
            if ( m_globalCellIdx1 != other.m_globalCellIdx1 )
            {
                return m_globalCellIdx1 < other.m_globalCellIdx1;
            }

            return ( m_globalCellIdx2 < other.m_globalCellIdx2 );
        }

    private:
        size_t m_globalCellIdx1;
        size_t m_globalCellIdx2;
    };

    std::set<CellPair> nativeCellPairs;

    for ( const auto& c : nativeConnections )
    {
        nativeCellPairs.emplace( CellPair( c.m_c1GlobIdx, c.m_c2GlobIdx ) );
    }

    if ( nativeConnections.size() != nativeCellPairs.size() )
    {
        QString message = QString( "Nnc connection imported from Eclipse are not unique\nNNC count : %1\nUnique : %2" )
                              .arg( nativeConnections.size() )
                              .arg( nativeCellPairs.size() );
        qDebug() << message;
    }

    std::set<CellPair> otherCellPairs;

    const cvf::Collection<RigFault>& faults = mainGrid->faults();
    for ( size_t faultIdx = 0; faultIdx < faults.size(); faultIdx++ )
    {
        const RigFault* fault = faults.at( faultIdx );

        const std::vector<RigFault::FaultFace>& faultFaces = fault->faultFaces();

        // #pragma omp parallel for
        for ( int faceIdx = 0; faceIdx < static_cast<int>( faultFaces.size() ); faceIdx++ )
        {
            const RigFault::FaultFace& f = faultFaces[faceIdx];

            size_t                             sourceReservoirCellIndex = f.m_nativeReservoirCellIndex;
            cvf::StructGridInterface::FaceType sourceCellFace           = f.m_nativeFace;

            const std::vector<cvf::Vec3d>& mainGridNodes = mainGrid->nodes();

            cvf::BoundingBox      bb;
            std::array<size_t, 4> sourceFaceIndices;
            mainGrid->globalCellArray()[sourceReservoirCellIndex].faceIndices( sourceCellFace, &sourceFaceIndices );

            bb.add( mainGridNodes[sourceFaceIndices[0]] );
            bb.add( mainGridNodes[sourceFaceIndices[1]] );
            bb.add( mainGridNodes[sourceFaceIndices[2]] );
            bb.add( mainGridNodes[sourceFaceIndices[3]] );

            std::vector<size_t> closeCells;
            mainGrid->findIntersectingCells( bb, &closeCells );

            cvf::StructGridInterface::FaceType candidateFace = cvf::StructGridInterface::oppositeFace( sourceCellFace );

            size_t neighborCellIndex = std::numeric_limits<size_t>::max();
            size_t ni                = std::numeric_limits<size_t>::max();
            size_t nj                = std::numeric_limits<size_t>::max();
            size_t nk                = std::numeric_limits<size_t>::max();

            {
                size_t i;
                size_t j;
                size_t k;
                mainGrid->ijkFromCellIndex( sourceReservoirCellIndex, &i, &j, &k );

                mainGrid->neighborIJKAtCellFace( i, j, k, sourceCellFace, &ni, &nj, &nk );

                if ( mainGrid->isCellValid( ni, nj, nk ) )
                {
                    neighborCellIndex = mainGrid->cellIndexFromIJK( ni, nj, nk );
                }
            }

            for ( size_t candidateCellIndex : closeCells )
            {
                if ( candidateCellIndex == sourceReservoirCellIndex )
                {
                    // Exclude cellIndex for source cell
                    continue;
                }

                if ( candidateCellIndex == neighborCellIndex )
                {
                    // Exclude direct neighbor
                    continue;
                }

                if ( neighborCellIndex != std::numeric_limits<size_t>::max() )
                {
                    // Find target IJK index based on source cell and cell face
                    // Exclude cells not matching destination target index

                    size_t ci = std::numeric_limits<size_t>::max();
                    size_t cj = std::numeric_limits<size_t>::max();
                    size_t ck = std::numeric_limits<size_t>::max();
                    mainGrid->ijkFromCellIndex( candidateCellIndex, &ci, &cj, &ck );

                    auto gridAxis = cvf::StructGridInterface::gridAxisFromFace( sourceCellFace );
                    if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_I )
                    {
                        if ( ni != ci )
                        {
                            continue;
                        }
                    }
                    else if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_J )
                    {
                        if ( nj != cj )
                        {
                            continue;
                        }
                    }
                    else if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_K )
                    {
                        if ( nk != ck )
                        {
                            continue;
                        }
                    }
                }

                CellPair candidate( sourceReservoirCellIndex, candidateCellIndex );

                if ( nativeCellPairs.count( candidate ) > 0 )
                {
                    continue;
                }

                if ( otherCellPairs.count( candidate ) > 0 )
                {
                    continue;
                }

                std::vector<size_t>     polygon;
                std::vector<cvf::Vec3d> intersections;

                std::array<size_t, 4> candidateFaceIndices;
                mainGrid->globalCellArray()[candidateCellIndex].faceIndices( candidateFace, &candidateFaceIndices );

                bool foundOverlap =
                    cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                           &intersections,
                                                                           (cvf::EdgeIntersectStorage<size_t>*)nullptr,
                                                                           cvf::wrapArrayConst( &mainGridNodes ),
                                                                           sourceFaceIndices.data(),
                                                                           candidateFaceIndices.data(),
                                                                           1e-6 );

                if ( foundOverlap )
                {
                    RigConnection conn;
                    conn.m_c1GlobIdx = sourceReservoirCellIndex;
                    conn.m_c1Face    = sourceCellFace;
                    conn.m_c2GlobIdx = candidateCellIndex;

                    conn.m_polygon = RigCellFaceGeometryTools::extractPolygon( mainGridNodes, polygon, intersections );

#pragma omp critical( critical_section_nnc_computations )
                    {
                        otherCellPairs.emplace( candidate );
                        otherConnections.emplace_back( conn );
                    }
                }
            }
        }
    }

    return otherConnections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigCellFaceGeometryTools::extractPolygon( const std::vector<cvf::Vec3d>& nativeNodes,
                                                                  const std::vector<size_t>&     connectionPolygon,
                                                                  const std::vector<cvf::Vec3d>& connectionIntersections )
{
    std::vector<cvf::Vec3d> allPolygonNodes;

    for ( size_t polygonIndex : connectionPolygon )
    {
        if ( polygonIndex < nativeNodes.size() )
            allPolygonNodes.push_back( nativeNodes[polygonIndex] );
        else
            allPolygonNodes.push_back( connectionIntersections[polygonIndex - nativeNodes.size()] );
    }

    return allPolygonNodes;
}
