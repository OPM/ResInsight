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
    m_polygonVertices = new cvf::Vec3fArray;
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
bool RivPolylineIntersectionGeometryGenerator::isAnyGeometryPresent() const
{
    return m_polylineSegmentsMeshData.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivPolylineIntersectionGeometryGenerator::polygonVxes() const
{
    CVF_ASSERT( m_polygonVertices->size() > 0 );
    return m_polygonVertices.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivPolylineIntersectionGeometryGenerator::vertiesPerPolygon() const
{
    CVF_ASSERT( m_verticesPerPolygon.size() > 0 );
    return m_verticesPerPolygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivPolylineIntersectionGeometryGenerator::polygonToCellIndex() const
{
    CVF_ASSERT( m_polygonToCellIdxMap.size() > 0 );
    return m_polygonToCellIdxMap;
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

    // Mesh data per polyline segment
    std::vector<PolylineSegmentMeshData> polylineSegmentMeshData   = {};
    std::vector<cvf::Vec3f>              calculatedPolygonVertices = {};

    cvf::BoundingBox gridBBox       = m_hexGrid->boundingBox();
    const double     topDepth       = gridBBox.max().z();
    const double     bottomDepth    = gridBBox.min().z();
    const auto       zAxisDirection = -cvf::Vec3d::Z_AXIS; // NOTE: Negative or positive direction?
    const cvf::Vec3d maxHeightVec   = zAxisDirection * gridBBox.radius();

    if ( m_polylineUtm.size() > 1 )
    {
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

            // Mesh data for polyline segment
            std::vector<float>     polygonVerticesUz     = {};
            std::vector<cvf::uint> verticesPerPolygon    = {};
            std::vector<cvf::uint> polygonToCellIndexMap = {};

            // Handle triangles per cell
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
                    const size_t vxIdx0 = triangleIdx * 3;
                    const auto&  vx0    = clippedTriangleVxes[vxIdx0 + 0].vx;
                    const auto&  vx1    = clippedTriangleVxes[vxIdx0 + 1].vx;
                    const auto&  vx2    = clippedTriangleVxes[vxIdx0 + 2].vx;

                    // Convert to local coordinates, where p1 is origin.
                    // The z-values are global z-values in the uz-coordinates.
                    const cvf::Vec3d point0( vx0.x() - p1.x(), vx0.y() - p1.y(), vx0.z() );
                    const cvf::Vec3d point1( vx1.x() - p1.x(), vx1.y() - p1.y(), vx1.z() );
                    const cvf::Vec3d point2( vx2.x() - p1.x(), vx2.y() - p1.y(), vx2.z() );

                    // TODO: Ensure counter clockwise order of vertices point0, point1, point2?

                    // Add triangle to enclosing polygon line handler
                    enclosingPolygonGenerator.addTriangleVertices( point0, point1, point2 );
                }

                // Must be a valid polygon to continue
                if ( !enclosingPolygonGenerator.isValidPolygon() )
                {
                    continue;
                }

                // Construct enclosing polygon after adding each triangle
                enclosingPolygonGenerator.constructEnclosingPolygon();
                const auto& vertices = enclosingPolygonGenerator.getPolygonVertices();

                // Construct local uz-coordinates using Pythagoras theorem
                for ( const auto& vertex : vertices )
                {
                    // NOTE: Can welding provide drifting of vertex positions?
                    // TODO: Project (x,y) into plane instead?
                    //
                    // Convert to local uz-coordinates, u is the distance along the normalized U-axis
                    const auto u = std::sqrt( vertex.x() * vertex.x() + vertex.y() * vertex.y() );
                    const auto z = vertex.z();

                    polygonVerticesUz.push_back( u );
                    polygonVerticesUz.push_back( z );

                    // Keep old code for debugging purposes
                    calculatedPolygonVertices.push_back( cvf::Vec3f( vertex + p1 ) );
                }
                verticesPerPolygon.push_back( static_cast<cvf::uint>( vertices.size() ) );
                polygonToCellIndexMap.push_back( static_cast<cvf::uint>( globalCellIdx ) );

                // Keep old code for debugging purposes
                m_verticesPerPolygon.push_back( vertices.size() ); // TODO: Remove when not needed for debug
                m_polygonToCellIdxMap.push_back( globalCellIdx ); // TODO: Remove when not needed for debug
            }

            // Create polygon indices array
            const auto             numVertices = static_cast<size_t>( polygonVerticesUz.size() / 2 );
            std::vector<cvf::uint> polygonIndices( numVertices );
            std::iota( polygonIndices.begin(), polygonIndices.end(), 0 );

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

    m_polygonVertices->assign( calculatedPolygonVertices ); // TODO: Remove when not needed for debug
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
