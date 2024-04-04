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

#include "RivEnclosingPolygonGenerator.h"
#include "RivIntersectionHexGridInterface.h"
#include "RivSectionFlattener.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"
#include "cafLine.h"

#include "cvfPlane.h"
#include "cvfVertexWelder.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPolylineIntersectionGeometryGenerator::RivPolylineIntersectionGeometryGenerator( const std::vector<cvf::Vec2d>&   polylineUtmXy,
                                                                                    RivIntersectionHexGridInterface* grid )
    : m_polylineUtm( initializePolylineUtmFromPolylineUtmXy( polylineUtmXy ) )
    , m_hexGrid( grid )
{
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
std::vector<cvf::Vec3d>
    RivPolylineIntersectionGeometryGenerator::initializePolylineUtmFromPolylineUtmXy( const std::vector<cvf::Vec2d>& polylineUtmXy )
{
    std::vector<cvf::Vec3d> polylineUtm;
    polylineUtm.reserve( polylineUtmXy.size() );

    const double zValue = 0.0;
    for ( const auto& xy : polylineUtmXy )
    {
        polylineUtm.push_back( cvf::Vec3d( xy.x(), xy.y(), zValue ) );
    }
    return polylineUtm;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<PolylineSegmentMeshData>& RivPolylineIntersectionGeometryGenerator::polylineSegmentsMeshData() const
{
    CVF_ASSERT( m_polylineSegmentsMeshData.size() > 0 );
    return m_polylineSegmentsMeshData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivPolylineIntersectionGeometryGenerator::isAnyGeometryPresent() const
{
    return m_polylineSegmentsMeshData.size() > 0;
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
    if ( m_hexGrid == nullptr || m_polylineSegmentsMeshData.size() != 0 ) return;
    if ( m_polylineUtm.size() < 2 ) return;

    cvf::BoundingBox gridBBox       = m_hexGrid->boundingBox();
    const double     topDepth       = gridBBox.max().z();
    const double     bottomDepth    = gridBBox.min().z();
    const auto       zAxisDirection = -cvf::Vec3d::Z_AXIS; // NOTE: Negative or positive direction?
    const cvf::Vec3d maxHeightVec   = zAxisDirection * gridBBox.radius();

    // Weld vertices per polyline segment
    // - Low welding distance, as the goal is to weld duplicate vertices
    // - Number of buckets is set per segment, utilizing number of cells intersecting the segment
    const double weldingDistance = 1.0e-3;
    const double weldingCellSize = 4.0 * weldingDistance;

    const size_t numPoints = m_polylineUtm.size();
    size_t       pointIdx  = 0;

    // Loop over polyline segments
    //
    // Create intersection geometry for each polyline segment and clip triangles outside the
    // polyline segment, using UTM-coordinates.
    //
    // Afterwards convert to "local" coordinates (p1 as origin), where the local uz-coordinates
    // for each vertex is calculated using Pythagoras theorem.
    //
    // As the plane is parallel to the z-axis, the local uz-coordinates per polyline segment
    // can be calculated using Pythagoras theorem. Where a segment is defined between p1 and p2,
    // which implies p1 is origin of the local coordinate system uz.
    //
    // For a segment a UTM coordinate (x_utm,y_utm,z_utm) converts to u = sqrt(x^2 + y^2) and z = z.
    // Where x, y and z are local vertex coordinates, after subtracting the (x_utm, y_utm, z_utm)-values
    // for p1 from the vertex UTM-coordinates.
    while ( pointIdx < numPoints - 1 )
    {
        size_t nextPointIdx = RivSectionFlattener::indexToNextValidPoint( m_polylineUtm, zAxisDirection, pointIdx );
        if ( nextPointIdx == size_t( -1 ) || nextPointIdx >= m_polylineUtm.size() )
        {
            break;
        }

        // Start and end point of polyline segment in UTM-coordinates
        const cvf::Vec3d p1 = m_polylineUtm[pointIdx];
        const cvf::Vec3d p2 = m_polylineUtm[nextPointIdx];

        // Get cell candidates for the polyline segment (subset of global cells)
        std::vector<size_t> columnCellCandidates =
            createPolylineSegmentCellCandidates( *m_hexGrid, p1, p2, maxHeightVec, topDepth, bottomDepth );

        // Plane for the polyline segment
        cvf::Plane plane;
        plane.setFromPoints( p1, p2, p2 + maxHeightVec );

        // Planes parallel to z-axis for p1 and p2, to prevent triangles outside the polyline segment
        cvf::Plane p1Plane;
        p1Plane.setFromPoints( p1, p1 + maxHeightVec, p1 + plane.normal() );
        cvf::Plane p2Plane;
        p2Plane.setFromPoints( p2, p2 + maxHeightVec, p2 - plane.normal() );

        // Placeholder for triangle vertices per cell
        std::vector<caf::HexGridIntersectionTools::ClipVx> hexPlaneCutTriangleVxes;
        hexPlaneCutTriangleVxes.reserve( 5 * 3 );
        std::vector<int> cellFaceForEachTriangleEdge;
        cellFaceForEachTriangleEdge.reserve( 5 * 3 );
        cvf::Vec3d cellCorners[8];
        size_t     cornerIndices[8];

        // Polyline segment data
        std::vector<cvf::uint> polygonIndices        = {};
        std::vector<cvf::uint> verticesPerPolygon    = {};
        std::vector<cvf::uint> polygonToCellIndexMap = {};

        // Welder for segment vertices
        // - Number of buckets is size of columnCellCandidates divided by 8 to avoid too many buckets (Random selected value).
        //   Usage of columnCellCandidates is to get a dynamic number of buckets usable for respective segment.
        const cvf::uint   numWelderBuckets = static_cast<cvf::uint>( columnCellCandidates.size() / size_t( 8 ) );
        cvf::VertexWelder segmentVertexWelder;
        segmentVertexWelder.initialize( weldingDistance, weldingCellSize, numWelderBuckets );

        // Intersection per grid cell - transform from set of triangles to polygon for cell
        for ( const auto globalCellIdx : columnCellCandidates )
        {
            if ( ( visibleCells != nullptr ) && ( ( *visibleCells )[globalCellIdx] == 0 ) ) continue;
            if ( !m_hexGrid->useCell( globalCellIdx ) ) continue;

            // Perform intersection and clipping of triangles using UTM-coordinates
            hexPlaneCutTriangleVxes.clear();
            m_hexGrid->cellCornerVertices( globalCellIdx, cellCorners );
            m_hexGrid->cellCornerIndices( globalCellIdx, cornerIndices );

            // Triangle vertices for polyline segment
            caf::HexGridIntersectionTools::planeHexIntersectionMC( plane,
                                                                   cellCorners,
                                                                   cornerIndices,
                                                                   &hexPlaneCutTriangleVxes,
                                                                   &cellFaceForEachTriangleEdge );

            // Clip triangles outside the polyline segment using the planes for point p1 and p2
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

            // Object to for adding triangle vertices, well vertices and generate polygon vertices
            RivEnclosingPolygonGenerator enclosingPolygonGenerator;

            // Add clipped triangle vertices to the polygon generator using local coordinates
            size_t clippedTriangleCount = clippedTriangleVxes.size() / 3;
            for ( size_t triangleIdx = 0; triangleIdx < clippedTriangleCount; ++triangleIdx )
            {
                // Get triangle vertices
                const size_t tVxIdx0 = triangleIdx * 3;
                const auto&  tVx0    = clippedTriangleVxes[tVxIdx0 + 0].vx;
                const auto&  tVx1    = clippedTriangleVxes[tVxIdx0 + 1].vx;
                const auto&  tVx2    = clippedTriangleVxes[tVxIdx0 + 2].vx;

                // TODO: Ensure counter clockwise order of vertices point0, point1, point2?

                // Convert to local coordinates, where p1 is origin.
                // The z-values are global z-values in the uz-coordinates.
                const cvf::Vec3f vx0( tVx0.x() - p1.x(), tVx0.y() - p1.y(), tVx0.z() );
                const cvf::Vec3f vx1( tVx1.x() - p1.x(), tVx1.y() - p1.y(), tVx1.z() );
                const cvf::Vec3f vx2( tVx2.x() - p1.x(), tVx2.y() - p1.y(), tVx2.z() );

                // Weld vertices and get vertex index
                bool        isWelded = false;
                const auto& wVxIdx0  = segmentVertexWelder.weldVertex( vx0, &isWelded );
                const auto& wVxIdx1  = segmentVertexWelder.weldVertex( vx1, &isWelded );
                const auto& wVxIdx2  = segmentVertexWelder.weldVertex( vx2, &isWelded );

                // Add triangle to enclosing polygon line handler
                enclosingPolygonGenerator.addTriangleVertexIndices( wVxIdx0, wVxIdx1, wVxIdx2 );
            }

            // Must be a valid polygon to continue
            if ( !enclosingPolygonGenerator.isValidPolygon() )
            {
                continue;
            }

            // Construct enclosing polygon after adding all triangles for cell
            enclosingPolygonGenerator.constructEnclosingPolygon();

            // Add vertex index to polygon indices
            const auto& vertexIndices = enclosingPolygonGenerator.getPolygonVertexIndices();
            polygonIndices.insert( polygonIndices.end(), vertexIndices.begin(), vertexIndices.end() );

            verticesPerPolygon.push_back( static_cast<cvf::uint>( vertexIndices.size() ) );
            polygonToCellIndexMap.push_back( static_cast<cvf::uint>( globalCellIdx ) );
        }

        // Build vertex array for polyline segment
        std::vector<float> polygonVerticesUz;
        for ( cvf::uint i = 0; i < segmentVertexWelder.vertexCount(); i++ )
        {
            const auto& vertex = segmentVertexWelder.vertex( i );
            // NOTE: Can welding provide drifting of vertex positions?
            // TODO: Project (x,y) into plane instead?
            //
            // auto projectedVertex = plane.projectPoint( vertex );

            // Convert to local uz-coordinates, u is the distance along the normalized U-axis.
            // Construct local uz-coordinates using Pythagoras theorem
            const auto u = std::sqrt( vertex.x() * vertex.x() + vertex.y() * vertex.y() );
            const auto z = vertex.z();
            polygonVerticesUz.push_back( u );
            polygonVerticesUz.push_back( z );
        }

        // Construct polyline segment mesh data
        PolylineSegmentMeshData polylineSegmentData;
        polylineSegmentData.startUtmXY            = cvf::Vec2d( p1.x(), p1.y() );
        polylineSegmentData.endUtmXY              = cvf::Vec2d( p2.x(), p2.y() );
        polylineSegmentData.vertexArrayUZ         = polygonVerticesUz;
        polylineSegmentData.verticesPerPolygon    = verticesPerPolygon;
        polylineSegmentData.polygonIndices        = polygonIndices;
        polylineSegmentData.polygonToCellIndexMap = polygonToCellIndexMap;

        // Add polyline segment mesh data to list
        m_polylineSegmentsMeshData.push_back( polylineSegmentData );

        // Set next polyline segment start index
        pointIdx = nextPointIdx;
    }
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
