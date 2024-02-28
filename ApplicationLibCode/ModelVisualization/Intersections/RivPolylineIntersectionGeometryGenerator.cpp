/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////

#include "RivPolylineIntersectionGeometryGenerator.h"

#include "RivIntersectionHexGridInterface.h"
#include "RivSectionFlattener.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include "cvfPlane.h"

#pragma optimize( "", off )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPolylineIntersectionGeometryGenerator::RivPolylineIntersectionGeometryGenerator( std::vector<cvf::Vec3d>&         polyline,
                                                                                    RivIntersectionHexGridInterface* grid )
    : m_polyline( polyline )
    , m_hexGrid( grid )
{
    m_triangleVxes = new cvf::Vec3fArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPolylineIntersectionGeometryGenerator::~RivPolylineIntersectionGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivPolylineIntersectionGeometryGenerator::isAnyGeometryPresent() const
{
    return m_triangleVxes->size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivPolylineIntersectionGeometryGenerator::triangleToCellIndex() const
{
    CVF_ASSERT( m_triangleVxes->size() > 0 );
    return m_triangleToCellIdxMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivPolylineIntersectionGeometryGenerator::triangleVxes() const
{
    CVF_ASSERT( m_triangleVxes->size() > 0 );
    return m_triangleVxes.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RivIntersectionVertexWeights>& RivPolylineIntersectionGeometryGenerator::triangleVxToCellCornerInterpolationWeights() const
{
    CVF_ASSERT( m_triangleVxes->size() > 0 );

    // Not implemented error
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineIntersectionGeometryGenerator::generateIntersectionGeometry( cvf::UByteArray* visibleCells )
{
    calculateArrays( visibleCells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineIntersectionGeometryGenerator::calculateArrays( cvf::UByteArray* visibleCells )
{
    if ( m_triangleVxes->size() != 0 || m_hexGrid.isNull() ) return;

    std::vector<cvf::Vec3f> calculatedTriangleVertices;

    // MeshLinesAccumulator meshAcc( m_hexGrid.p() );

    cvf::BoundingBox gridBBox = m_hexGrid->boundingBox();

    const double topDepth    = gridBBox.max().z();
    const double bottomDepth = gridBBox.min().z();

    std::array<cvf::Vec3d, 8> corners;
    gridBBox.cornerVertices( corners.data() );

    cvf::Vec3d p1_low( corners[0].x(), corners[0].y(), bottomDepth );
    cvf::Vec3d p2_low( corners[1].x(), corners[1].y(), bottomDepth );
    cvf::Vec3d p3_low( corners[2].x(), corners[2].y(), bottomDepth );

    cvf::Plane lowPlane;
    lowPlane.setFromPoints( p1_low, p2_low, p3_low );

    cvf::Vec3d p1_high( p1_low.x(), p1_low.y(), topDepth );
    cvf::Vec3d p2_high( p2_low.x(), p2_low.y(), topDepth );
    cvf::Vec3d p3_high( p3_low.x(), p3_low.y(), topDepth );

    cvf::Plane highPlane;
    highPlane.setFromPoints( p1_high, p2_high, p3_high );
    highPlane.flip();

    const auto       zAxisDirection = -cvf::Vec3d::Z_AXIS; // NOTE: Negative or positive direction?
    const cvf::Vec3d maxHeightVec   = zAxisDirection * gridBBox.radius();
    if ( m_polyline.size() > 1 )
    {
        const size_t numPoints = m_polyline.size();
        size_t       pointIdx  = 0;
        while ( pointIdx < numPoints - 1 )
        {
            size_t nextPointIdx = RivSectionFlattener::indexToNextValidPoint( m_polyline, zAxisDirection, pointIdx );
            if ( nextPointIdx == size_t( -1 ) || nextPointIdx >= m_polyline.size() )
            {
                break;
            }

            // Start and end point of polyline segment
            const cvf::Vec3d p1 = m_polyline[pointIdx];
            const cvf::Vec3d p2 = m_polyline[nextPointIdx];

            std::vector<size_t> columnCellCandidates =
                createPolylineSegmentCellCandidates( *m_hexGrid, p1, p2, maxHeightVec, topDepth, bottomDepth );

            cvf::Plane plane;
            plane.setFromPoints( p1, p2, p2 + maxHeightVec );

            // Planes parallel to z-axis for p1 and p2, to prevent triangles outside the polyline segment
            cvf::Plane p1Plane;
            p1Plane.setFromPoints( p1, p1 + maxHeightVec, p1 + plane.normal() );
            cvf::Plane p2Plane;
            p2Plane.setFromPoints( p2, p2 + maxHeightVec, p2 - plane.normal() );

            std::vector<caf::HexGridIntersectionTools::ClipVx> hexPlaneCutTriangleVxes;
            hexPlaneCutTriangleVxes.reserve( 5 * 3 );
            std::vector<int> cellFaceForEachTriangleEdge;
            cellFaceForEachTriangleEdge.reserve( 5 * 3 );
            cvf::Vec3d cellCorners[8];
            size_t     cornerIndices[8];
            for ( const auto globalCellIdx : columnCellCandidates )
            {
                if ( ( visibleCells != nullptr ) && ( ( *visibleCells )[globalCellIdx] == 0 ) ) continue;
                if ( !m_hexGrid->useCell( globalCellIdx ) ) continue;

                hexPlaneCutTriangleVxes.clear();
                m_hexGrid->cellCornerVertices( globalCellIdx, cellCorners );
                m_hexGrid->cellCornerIndices( globalCellIdx, cornerIndices );

                // Triangle vertices for polyline segment
                caf::HexGridIntersectionTools::planeHexIntersectionMC( plane,
                                                                       cellCorners,
                                                                       cornerIndices,
                                                                       &hexPlaneCutTriangleVxes,
                                                                       &cellFaceForEachTriangleEdge );

                // Clip triangles outside the polyline segment using p1 and p2 planes
                std::vector<caf::HexGridIntersectionTools::ClipVx> clippedTriangleVxes;
                std::vector<int>                                   cellFaceForEachClippedTriangleEdge;

                caf::HexGridIntersectionTools::clipTrianglesBetweenTwoParallelPlanes( hexPlaneCutTriangleVxes,
                                                                                      cellFaceForEachTriangleEdge,
                                                                                      p1Plane,
                                                                                      p2Plane,
                                                                                      &clippedTriangleVxes,
                                                                                      &cellFaceForEachClippedTriangleEdge );

                for ( caf::HexGridIntersectionTools::ClipVx& clvx : clippedTriangleVxes )
                {
                    if ( !clvx.isVxIdsNative ) clvx.derivedVxLevel = 0;
                }

                // Fill triangle vertices vector with clipped triangle vertices
                size_t clippedTriangleCount = clippedTriangleVxes.size() / 3;
                for ( size_t triangleIdx = 0; triangleIdx < clippedTriangleCount; ++triangleIdx )
                {
                    const size_t vxIdx0 = triangleIdx * 3;
                    const size_t vxIdx1 = vxIdx0 + 1;
                    const size_t vxIdx2 = vxIdx0 + 2;

                    // Accumulate triangle vertices
                    cvf::Vec3f point0( clippedTriangleVxes[vxIdx0].vx );
                    cvf::Vec3f point1( clippedTriangleVxes[vxIdx1].vx );
                    cvf::Vec3f point2( clippedTriangleVxes[vxIdx2].vx );

                    calculatedTriangleVertices.emplace_back( point0 );
                    calculatedTriangleVertices.emplace_back( point1 );
                    calculatedTriangleVertices.emplace_back( point2 );

                    // TODO: Accumulate mesh lines?
                    // meshAcc.accumulateMeshLines( cellFaceForEachTriangleEdge, ...

                    m_triangleToCellIdxMap.push_back( globalCellIdx );
                }
            }
            pointIdx = nextPointIdx;
        }
    }

    m_triangleVxes->assign( calculatedTriangleVertices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RivPolylineIntersectionGeometryGenerator::createPolylineSegmentCellCandidates( const RivIntersectionHexGridInterface& hexGrid,
                                                                                                   const cvf::Vec3d& startPoint,
                                                                                                   const cvf::Vec3d& endPoint,
                                                                                                   const cvf::Vec3d& heightVector,
                                                                                                   const double      topDepth,
                                                                                                   const double      bottomDepth )
{
    cvf::BoundingBox sectionBBox;
    sectionBBox.add( startPoint );
    sectionBBox.add( endPoint );
    sectionBBox.add( startPoint + heightVector );
    sectionBBox.add( startPoint - heightVector );
    sectionBBox.add( endPoint + heightVector );
    sectionBBox.add( endPoint - heightVector );

    sectionBBox.cutAbove( topDepth );
    sectionBBox.cutBelow( bottomDepth );

    return hexGrid.findIntersectingCells( sectionBBox );
}
