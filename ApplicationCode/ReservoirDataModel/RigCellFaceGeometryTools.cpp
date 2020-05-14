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

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigMainGrid.h"
#include "RigNncConnection.h"

#include "cvfGeometryTools.h"

#include "cafAssert.h"

#include <QDebug>

#include <omp.h>

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

    bool isPossibleNeighborInDirection[6] = { true, true, true, true, true, true };

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

void assignThreadConnections( RigConnectionContainer& allConnections, RigConnectionContainer& threadConnections )
{
#pragma omp critical
    {
        allConnections.push_back( threadConnections );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnectionContainer RigCellFaceGeometryTools::computeOtherNncs( const RigMainGrid*            mainGrid,
                                                                   const RigConnectionContainer& nativeConnections,
                                                                   const RigActiveCellInfo*      activeCellInfo,
                                                                   bool                          includeInactiveCells )
{
    // Compute Non-Neighbor Connections (NNC) not reported by Eclipse. NNCs with zero transmissibility are not reported
    // by Eclipse. Use faults as basis for subset of cells to find NNC connection for. The imported connections from
    // Eclipse are located at the beginning of the connections vector.

    std::set<std::pair<unsigned, unsigned>> nativeCellPairs;

    for ( size_t i = 0; i < nativeConnections.size(); ++i )
    {
        RigConnection c = nativeConnections[i];
        nativeCellPairs.emplace( static_cast<unsigned>( c.c1GlobIdx() ), static_cast<unsigned>( c.c2GlobIdx() ) );
    }

    if ( nativeConnections.size() != nativeCellPairs.size() )
    {
        QString message = QString( "Nnc connection imported from Eclipse are not unique\nNNC count : %1\nUnique : %2" )
                              .arg( nativeConnections.size() )
                              .arg( nativeCellPairs.size() );
        qDebug() << message;
    }

    const cvf::Collection<RigFault>& faults = mainGrid->faults();

    RigConnectionContainer otherConnections;

    for ( int faultIdx = 0; faultIdx < (int)faults.size(); faultIdx++ )
    {
        const RigFault* fault = faults.at( faultIdx );

        const std::vector<RigFault::FaultFace>& faultFaces = fault->faultFaces();

        // Build a vector of active face indices so we don't have to have this check inside the parallel loop.
        // This makes the load balancing much better in the loop.
        std::vector<size_t> activeFaceIndices;
        activeFaceIndices.reserve( faultFaces.size() );
        for ( size_t faceIdx = 0; faceIdx < faultFaces.size(); ++faceIdx )
        {
            const RigFault::FaultFace& f = faultFaces[faceIdx];

            bool atLeastOneCellActive = true;
            if ( !includeInactiveCells && activeCellInfo && activeCellInfo->reservoirActiveCellCount() > 0u )
            {
                atLeastOneCellActive = activeCellInfo->isActive( f.m_nativeReservoirCellIndex ) ||
                                       activeCellInfo->isActive( f.m_oppositeReservoirCellIndex );
            }

            if ( atLeastOneCellActive ) activeFaceIndices.push_back( faceIdx );
        }

        size_t totalNumberOfConnections = 0u;
#pragma omp parallel
        {
            RigConnectionContainer threadConnections;
#pragma omp for schedule( guided ) reduction( + : totalNumberOfConnections )
            for ( int activeFaceIdx = 0; activeFaceIdx < static_cast<int>( activeFaceIndices.size() ); activeFaceIdx++ )
            {
                size_t faceIdx = activeFaceIndices[activeFaceIdx];
                extractConnectionsForFace( faultFaces[faceIdx], mainGrid, nativeCellPairs, threadConnections );
            }
#pragma omp barrier
            otherConnections.reserve( otherConnections.size() + totalNumberOfConnections );

            // Merge together connections per thread
            assignThreadConnections( otherConnections, threadConnections );
        } // end parallel region
    }

    otherConnections.remove_duplicates();
    return otherConnections;
}

void RigCellFaceGeometryTools::extractConnectionsForFace( const RigFault::FaultFace&                     face,
                                                          const RigMainGrid*                             mainGrid,
                                                          const std::set<std::pair<unsigned, unsigned>>& nativeCellPairs,
                                                          RigConnectionContainer&                        connections )
{
    size_t                             sourceReservoirCellIndex = face.m_nativeReservoirCellIndex;
    cvf::StructGridInterface::FaceType sourceCellFace           = face.m_nativeFace;

    if ( sourceReservoirCellIndex >= mainGrid->cellCount() )
    {
        return;
    }

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
        mainGrid->ijkFromCellIndexUnguarded( sourceReservoirCellIndex, &i, &j, &k );

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

        if ( candidateCellIndex >= mainGrid->cellCount() )
        {
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
            mainGrid->ijkFromCellIndexUnguarded( candidateCellIndex, &ci, &cj, &ck );

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

        std::pair<unsigned, unsigned> candidate( static_cast<unsigned>( sourceReservoirCellIndex ),
                                                 static_cast<unsigned>( candidateCellIndex ) );

        if ( nativeCellPairs.count( candidate ) > 0 )
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
            RigConnection conn( sourceReservoirCellIndex,
                                candidateCellIndex,
                                sourceCellFace,
                                RigCellFaceGeometryTools::extractPolygon( mainGridNodes, polygon, intersections ) );

            connections.push_back( conn );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RigCellFaceGeometryTools::extractPolygon( const std::vector<cvf::Vec3d>& nativeNodes,
                                                                  const std::vector<size_t>&     connectionPolygon,
                                                                  const std::vector<cvf::Vec3d>& connectionIntersections )
{
    std::vector<cvf::Vec3f> allPolygonNodes;

    for ( size_t polygonIndex : connectionPolygon )
    {
        if ( polygonIndex < nativeNodes.size() )
            allPolygonNodes.push_back( cvf::Vec3f( nativeNodes[polygonIndex] ) );
        else
            allPolygonNodes.push_back( cvf::Vec3f( connectionIntersections[polygonIndex - nativeNodes.size()] ) );
    }

    return allPolygonNodes;
}
