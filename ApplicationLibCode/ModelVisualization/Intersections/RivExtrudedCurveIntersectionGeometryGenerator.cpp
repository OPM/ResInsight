/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivExtrudedCurveIntersectionGeometryGenerator.h"

#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigSurface.h"
#include "RigSurfaceResampler.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimSurface.h"
#include "RimSurfaceIntersectionBand.h"
#include "RimSurfaceIntersectionCurve.h"

#include "RivExtrudedCurveIntersectionPartMgr.h"
#include "RivIntersectionHexGridInterface.h"
#include "RivPolylineGenerator.h"
#include "RivSectionFlattener.h"

#include "cafDisplayCoordTransform.h"
#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include "cvfDrawableGeo.h"
#include "cvfGeometryTools.h"
#include "cvfPlane.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRay.h"
#include "cvfScalarMapper.h"

cvf::ref<caf::DisplayCoordTransform> displayCoordTransform( const RimExtrudedCurveIntersection* intersection )
{
    auto rimView = intersection->firstAncestorOrThisOfTypeAsserted<Rim3dView>();

    cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
    return transForm;
}

//--------------------------------------------------------------------------------------------------
/// isFlattened means to transform each flat section of the intersection onto the XZ plane
/// placed adjacent to each other as if they were rotated around the common extrusion line like a hinge
//--------------------------------------------------------------------------------------------------
RivExtrudedCurveIntersectionGeometryGenerator::RivExtrudedCurveIntersectionGeometryGenerator( RimExtrudedCurveIntersection* crossSection,
                                                                                              std::vector<std::vector<cvf::Vec3d>>& polylines,
                                                                                              const cvf::Vec3d& extrusionDirection,
                                                                                              RivIntersectionHexGridInterface* grid,
                                                                                              bool                             isFlattened,
                                                                                              const cvf::Vec3d& flattenedPolylineStartPoint )
    : m_intersection( crossSection )
    , m_polylines( polylines )
    , m_extrusionDirection( extrusionDirection )
    , m_hexGrid( grid )
    , m_isFlattened( isFlattened )
    , m_flattenedPolylineStartPoint( flattenedPolylineStartPoint )
{
    m_triangleVxes            = new cvf::Vec3fArray;
    m_cellBorderLineVxes      = new cvf::Vec3fArray;
    m_faultCellBorderLineVxes = new cvf::Vec3fArray;

    if ( m_isFlattened ) m_extrusionDirection = -cvf::Vec3d::Z_AXIS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivExtrudedCurveIntersectionGeometryGenerator::~RivExtrudedCurveIntersectionGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionGeometryGenerator::calculateLineSegementTransforms()
{
    if ( m_isFlattened )
    {
        if ( m_polylines.empty() || m_polylines.back().empty() ) return;

        cvf::Vec3d startOffset = m_flattenedPolylineStartPoint;

        for ( const std::vector<cvf::Vec3d>& polyLine : m_polylines )
        {
            startOffset.z() = polyLine[0].z();
            m_lineSegmentTransforms.emplace_back(
                RivSectionFlattener::calculateFlatteningCSsForPolyline( polyLine, m_extrusionDirection, startOffset, &startOffset ) );
        }
    }
    else
    {
        m_lineSegmentTransforms.clear();

        cvf::Vec3d displayOffset( 0, 0, 0 );
        {
            auto gridView = m_intersection->firstAncestorOrThisOfType<RimGridView>();
            if ( gridView && gridView->ownerCase() )
            {
                displayOffset = gridView->ownerCase()->displayModelOffset();
            }
        }

        cvf::Mat4d invSectionCS = cvf::Mat4d::fromTranslation( -displayOffset );

        for ( const auto& polyLine : m_polylines )
        {
            m_lineSegmentTransforms.emplace_back();
            std::vector<cvf::Mat4d>& segmentTransforms = m_lineSegmentTransforms.back();
            for ( size_t lIdx = 0; lIdx < polyLine.size(); ++lIdx )
            {
                segmentTransforms.push_back( invSectionCS );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionGeometryGenerator::calculateTransformedPolyline()
{
    if ( m_lineSegmentTransforms.size() != m_polylines.size() ) return;

    for ( size_t lineIdx = 0; lineIdx < m_polylines.size(); ++lineIdx )
    {
        std::vector<cvf::Vec3d> flatPolyline;

        const auto& polyline = m_polylines[lineIdx];
        for ( size_t index = 0; index < polyline.size(); ++index )
        {
            flatPolyline.push_back( transformPointByPolylineSegmentIndex( polyline[index], lineIdx, index ) );
        }

        m_transformedPolyLines.emplace_back( flatPolyline );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionGeometryGenerator::calculateSurfaceIntersectionPoints()
{
    m_transformedSurfaceIntersectionPolylines.clear();

    if ( !m_polylines.empty() )
    {
        auto firstPolyLine = m_polylines.front();

        std::vector<RimSurface*> surfaces;
        for ( auto curve : m_intersection->surfaceIntersectionCurves() )
        {
            if ( curve->surface() ) surfaces.push_back( curve->surface() );
        }
        for ( auto band : m_intersection->surfaceIntersectionBands() )
        {
            if ( band->surface1() ) surfaces.push_back( band->surface1() );
            if ( band->surface2() ) surfaces.push_back( band->surface2() );
        }

        for ( auto rimSurface : surfaces )
        {
            if ( !rimSurface ) return;

            rimSurface->loadDataIfRequired();
            auto surface = rimSurface->surfaceData();
            if ( !surface ) return;

            std::vector<cvf::Vec3d> transformedSurfacePolyline;

            // Resample polyline to required resolution
            const double maxLineSegmentLength = 1.0;
            const auto resampledPolyline = RigSurfaceResampler::computeResampledPolylineWithSegmentInfo( firstPolyLine, maxLineSegmentLength );

            for ( const auto& [point, segmentIndex] : resampledPolyline )
            {
                cvf::Vec3d pointAbove = cvf::Vec3d( point.x(), point.y(), 10000.0 );
                cvf::Vec3d pointBelow = cvf::Vec3d( point.x(), point.y(), -10000.0 );

                cvf::Vec3d intersectionPoint;
                bool foundMatch = RigSurfaceResampler::findClosestPointOnSurface( surface, pointAbove, pointBelow, intersectionPoint );
                if ( foundMatch )
                {
                    const size_t lineIndex = 0;
                    transformedSurfacePolyline.push_back( transformPointByPolylineSegmentIndex( intersectionPoint, lineIndex, segmentIndex ) );
                }
            }

            if ( !transformedSurfacePolyline.empty() ) m_transformedSurfaceIntersectionPolylines[rimSurface] = transformedSurfacePolyline;
        }
    }
}

class MeshLinesAccumulator
{
public:
    MeshLinesAccumulator( const RivIntersectionHexGridInterface* hexGrid )
        : m_hexGrid( hexGrid )
    {
    }

    std::vector<cvf::Vec3f>               cellBorderLineVxes;
    std::vector<cvf::Vec3f>               faultCellBorderLineVxes;
    std::map<const RigFault*, cvf::Vec3d> faultToHighestFaultMeshVxMap;

    void accumulateMeshLines( const std::vector<int>& cellFaceForEachClippedTriangleEdge,
                              uint                    triVxIdx,
                              size_t                  globalCellIdx,
                              const cvf::Vec3d&       p0,
                              const cvf::Vec3d&       p1 )
    {
        auto isFace    = []( int faceEnum ) { return 0 <= faceEnum && faceEnum <= 5; };
        using FaceType = cvf::StructGridInterface::FaceType;

        if ( isFace( cellFaceForEachClippedTriangleEdge[triVxIdx] ) )
        {
            const RigFault* fault =
                m_hexGrid->findFaultFromCellIndexAndCellFace( globalCellIdx, (FaceType)cellFaceForEachClippedTriangleEdge[triVxIdx] );
            if ( fault )
            {
                cvf::Vec3d highestVx = p0.z() > p1.z() ? p0 : p1;

                auto itIsInsertedPair = faultToHighestFaultMeshVxMap.insert( { fault, highestVx } );
                if ( !itIsInsertedPair.second )
                {
                    if ( itIsInsertedPair.first->second.z() < highestVx.z() )
                    {
                        itIsInsertedPair.first->second = highestVx;
                    }
                }

                faultCellBorderLineVxes.emplace_back( p0 );
                faultCellBorderLineVxes.emplace_back( p1 );
            }
            else
            {
                cellBorderLineVxes.emplace_back( p0 );
                cellBorderLineVxes.emplace_back( p1 );
            }
        }
    }

private:
    cvf::cref<RivIntersectionHexGridInterface> m_hexGrid;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionGeometryGenerator::calculateArrays( cvf::UByteArray* visibleCells )
{
    if ( m_triangleVxes->size() ) return;

    if ( m_hexGrid.isNull() ) return;

    m_extrusionDirection.normalize();
    std::vector<cvf::Vec3f> triangleVertices;

    MeshLinesAccumulator meshAcc( m_hexGrid.p() );

    cvf::BoundingBox gridBBox = m_hexGrid->boundingBox();

    m_hexGrid->setKIntervalFilter( m_intersection->kLayerFilterEnabled(), m_intersection->kFilterText().toStdString() );

    calculateLineSegementTransforms();
    calculateTransformedPolyline();

    // set up our horizontal cut planes
    const double topDepth    = m_intersection->upperFilterDepth( gridBBox.max().z() );
    const double bottomDepth = m_intersection->lowerFilterDepth( gridBBox.min().z() );

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

    for ( size_t pLineIdx = 0; pLineIdx < m_polylines.size(); ++pLineIdx )
    {
        const std::vector<cvf::Vec3d>& polyLine = m_polylines[pLineIdx];

        if ( polyLine.size() < 2 ) continue;

        size_t lineCount = polyLine.size();
        size_t lIdx      = 0;
        while ( lIdx < lineCount - 1 )
        {
            size_t idxToNextP = RivSectionFlattener::indexToNextValidPoint( polyLine, m_extrusionDirection, lIdx );

            if ( idxToNextP == size_t( -1 ) ) break;

            cvf::Vec3d p1 = polyLine[lIdx];
            cvf::Vec3d p2 = polyLine[idxToNextP];

            cvf::BoundingBox sectionBBox;
            sectionBBox.add( p1 );
            sectionBBox.add( p2 );

            cvf::Vec3d maxHeightVec;

            double maxSectionHeightUp   = 0;
            double maxSectionHeightDown = 0;

            if ( m_intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_AZIMUTHLINE )
            {
                maxSectionHeightUp   = m_intersection->lengthUp();
                maxSectionHeightDown = m_intersection->lengthDown();

                if ( maxSectionHeightUp + maxSectionHeightDown == 0 )
                {
                    return;
                }

                cvf::Vec3d maxHeightVecDown = m_extrusionDirection * maxSectionHeightUp;
                cvf::Vec3d maxHeightVecUp   = m_extrusionDirection * maxSectionHeightDown;

                sectionBBox.add( p1 + maxHeightVecUp );
                sectionBBox.add( p1 - maxHeightVecDown );
                sectionBBox.add( p2 + maxHeightVecUp );
                sectionBBox.add( p2 - maxHeightVecDown );

                maxHeightVec = maxHeightVecUp + maxHeightVecDown;
            }
            else
            {
                maxHeightVec = m_extrusionDirection * gridBBox.radius();

                sectionBBox.add( p1 + maxHeightVec );
                sectionBBox.add( p1 - maxHeightVec );
                sectionBBox.add( p2 + maxHeightVec );
                sectionBBox.add( p2 - maxHeightVec );
            }

            sectionBBox.cutBelow( bottomDepth );
            sectionBBox.cutAbove( topDepth );

            std::vector<size_t> columnCellCandidates = m_hexGrid->findIntersectingCells( sectionBBox );

            cvf::Plane plane;
            plane.setFromPoints( p1, p2, p2 + maxHeightVec );

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

            cvf::Mat4d invSectionCS = m_lineSegmentTransforms[pLineIdx][lIdx];

            for ( auto globalCellIdx : columnCellCandidates )
            {
                if ( ( visibleCells != nullptr ) && ( ( *visibleCells )[globalCellIdx] == 0 ) ) continue;
                if ( !m_hexGrid->useCell( globalCellIdx ) ) continue;

                hexPlaneCutTriangleVxes.clear();
                m_hexGrid->cellCornerVertices( globalCellIdx, cellCorners );
                m_hexGrid->cellCornerIndices( globalCellIdx, cornerIndices );

                caf::HexGridIntersectionTools::planeHexIntersectionMC( plane,
                                                                       cellCorners,
                                                                       cornerIndices,
                                                                       &hexPlaneCutTriangleVxes,
                                                                       &cellFaceForEachTriangleEdge );

                if ( m_intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_AZIMUTHLINE )
                {
                    bool hasAnyPointsOnSurface = false;
                    for ( const caf::HexGridIntersectionTools::ClipVx& vertex : hexPlaneCutTriangleVxes )
                    {
                        cvf::Vec3d temp        = vertex.vx - p1;
                        double     dot         = temp.dot( m_extrusionDirection );
                        double     lengthCheck = 0;

                        if ( dot < 0 )
                        {
                            lengthCheck = maxSectionHeightUp;
                        }
                        else
                        {
                            lengthCheck = maxSectionHeightDown;
                        }

                        double distance = cvf::Math::sqrt( cvf::GeometryTools::linePointSquareDist( p1, p2, vertex.vx ) );
                        if ( distance < lengthCheck )
                        {
                            hasAnyPointsOnSurface = true;
                            break;
                        }
                    }
                    if ( !hasAnyPointsOnSurface )
                    {
                        continue;
                    }
                }

                std::vector<caf::HexGridIntersectionTools::ClipVx> clippedTriangleVxes_stage1;
                std::vector<int>                                   cellFaceForEachClippedTriangleEdge_stage1;

                caf::HexGridIntersectionTools::clipTrianglesBetweenTwoParallelPlanes( hexPlaneCutTriangleVxes,
                                                                                      cellFaceForEachTriangleEdge,
                                                                                      p1Plane,
                                                                                      p2Plane,
                                                                                      &clippedTriangleVxes_stage1,
                                                                                      &cellFaceForEachClippedTriangleEdge_stage1 );

                for ( caf::HexGridIntersectionTools::ClipVx& clvx : clippedTriangleVxes_stage1 )
                    if ( !clvx.isVxIdsNative ) clvx.derivedVxLevel = 0;

                std::vector<caf::HexGridIntersectionTools::ClipVx> clippedTriangleVxes;
                std::vector<int>                                   cellFaceForEachClippedTriangleEdge;

                caf::HexGridIntersectionTools::clipTrianglesBetweenTwoParallelPlanes( clippedTriangleVxes_stage1,
                                                                                      cellFaceForEachClippedTriangleEdge_stage1,
                                                                                      lowPlane,
                                                                                      highPlane,
                                                                                      &clippedTriangleVxes,
                                                                                      &cellFaceForEachClippedTriangleEdge );

                for ( caf::HexGridIntersectionTools::ClipVx& clvx : clippedTriangleVxes )
                    if ( !clvx.isVxIdsNative && clvx.derivedVxLevel == -1 ) clvx.derivedVxLevel = 1;

                size_t clippedTriangleCount = clippedTriangleVxes.size() / 3;

                for ( uint tIdx = 0; tIdx < clippedTriangleCount; ++tIdx )
                {
                    uint triVxIdx = tIdx * 3;

                    // Accumulate triangle vertices
                    cvf::Vec3d point0( clippedTriangleVxes[triVxIdx + 0].vx );
                    cvf::Vec3d point1( clippedTriangleVxes[triVxIdx + 1].vx );
                    cvf::Vec3d point2( clippedTriangleVxes[triVxIdx + 2].vx );

                    point0 = point0.getTransformedPoint( invSectionCS );
                    point1 = point1.getTransformedPoint( invSectionCS );
                    point2 = point2.getTransformedPoint( invSectionCS );

                    if ( m_isFlattened )
                    {
                        // The points are transformed in to the XZ-plane with Y = zero.
                        // Set all y values to zero to avoid numerical issues
                        point0.y() = 0.0;
                        point1.y() = 0.0;
                        point2.y() = 0.0;
                    }

                    triangleVertices.emplace_back( point0 );
                    triangleVertices.emplace_back( point1 );
                    triangleVertices.emplace_back( point2 );

                    // Accumulate mesh lines
                    meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge, triVxIdx + 0, globalCellIdx, point0, point1 );
                    meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge, triVxIdx + 1, globalCellIdx, point1, point2 );
                    meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge, triVxIdx + 2, globalCellIdx, point2, point0 );

                    // Mapping to cell index
                    m_triangleToCellIdxMap.push_back( globalCellIdx );

                    // Interpolation from nodes
                    for ( int i = 0; i < 3; ++i )
                    {
                        caf::HexGridIntersectionTools::ClipVx cvx = clippedTriangleVxes[triVxIdx + i];
                        if ( cvx.isVxIdsNative )
                        {
                            m_triVxToCellCornerWeights.emplace_back( cvx.clippedEdgeVx1Id, cvx.clippedEdgeVx2Id, cvx.normDistFromEdgeVx1 );
                        }
                        else
                        {
                            caf::HexGridIntersectionTools::ClipVx cvx1;
                            caf::HexGridIntersectionTools::ClipVx cvx2;

                            if ( cvx.derivedVxLevel == 0 )
                            {
                                cvx1 = hexPlaneCutTriangleVxes[cvx.clippedEdgeVx1Id];
                                cvx2 = hexPlaneCutTriangleVxes[cvx.clippedEdgeVx2Id];
                            }
                            else if ( cvx.derivedVxLevel == 1 )
                            {
                                cvx1 = clippedTriangleVxes_stage1[cvx.clippedEdgeVx1Id];
                                cvx2 = clippedTriangleVxes_stage1[cvx.clippedEdgeVx2Id];
                            }
                            else
                            {
                                CVF_ASSERT( false );
                            }

                            if ( cvx1.isVxIdsNative && cvx2.isVxIdsNative )
                            {
                                m_triVxToCellCornerWeights.emplace_back( cvx1.clippedEdgeVx1Id,
                                                                         cvx1.clippedEdgeVx2Id,
                                                                         cvx1.normDistFromEdgeVx1,
                                                                         cvx2.clippedEdgeVx1Id,
                                                                         cvx2.clippedEdgeVx2Id,
                                                                         cvx2.normDistFromEdgeVx1,
                                                                         cvx.normDistFromEdgeVx1 );
                            }
                            else
                            {
                                caf::HexGridIntersectionTools::ClipVx cvx11;
                                caf::HexGridIntersectionTools::ClipVx cvx12;
                                caf::HexGridIntersectionTools::ClipVx cvx21;
                                caf::HexGridIntersectionTools::ClipVx cvx22;

                                if ( cvx1.isVxIdsNative )
                                {
                                    cvx11 = cvx1;
                                    cvx12 = cvx1;
                                }
                                else if ( cvx1.derivedVxLevel == 0 )
                                {
                                    cvx11 = hexPlaneCutTriangleVxes[cvx1.clippedEdgeVx1Id];
                                    cvx12 = hexPlaneCutTriangleVxes[cvx1.clippedEdgeVx2Id];
                                }
                                else if ( cvx2.derivedVxLevel == 1 )
                                {
                                    cvx11 = clippedTriangleVxes_stage1[cvx1.clippedEdgeVx1Id];
                                    cvx12 = clippedTriangleVxes_stage1[cvx1.clippedEdgeVx2Id];
                                }
                                else
                                {
                                    CVF_ASSERT( false );
                                }

                                if ( cvx2.isVxIdsNative )
                                {
                                    cvx21 = cvx2;
                                    cvx22 = cvx2;
                                }
                                else if ( cvx2.derivedVxLevel == 0 )
                                {
                                    cvx21 = hexPlaneCutTriangleVxes[cvx2.clippedEdgeVx1Id];
                                    cvx22 = hexPlaneCutTriangleVxes[cvx2.clippedEdgeVx2Id];
                                }
                                else if ( cvx2.derivedVxLevel == 1 )
                                {
                                    cvx21 = clippedTriangleVxes_stage1[cvx2.clippedEdgeVx1Id];
                                    cvx22 = clippedTriangleVxes_stage1[cvx2.clippedEdgeVx2Id];
                                }
                                else
                                {
                                    CVF_ASSERT( false );
                                }

                                CVF_TIGHT_ASSERT( cvx11.isVxIdsNative && cvx12.isVxIdsNative && cvx21.isVxIdsNative && cvx22.isVxIdsNative );

                                m_triVxToCellCornerWeights.emplace_back( cvx11.clippedEdgeVx1Id,
                                                                         cvx11.clippedEdgeVx2Id,
                                                                         cvx11.normDistFromEdgeVx1,
                                                                         cvx12.clippedEdgeVx1Id,
                                                                         cvx12.clippedEdgeVx2Id,
                                                                         cvx2.normDistFromEdgeVx1,
                                                                         cvx21.clippedEdgeVx1Id,
                                                                         cvx21.clippedEdgeVx2Id,
                                                                         cvx21.normDistFromEdgeVx1,
                                                                         cvx22.clippedEdgeVx1Id,
                                                                         cvx22.clippedEdgeVx2Id,
                                                                         cvx22.normDistFromEdgeVx1,
                                                                         cvx1.normDistFromEdgeVx1,
                                                                         cvx2.normDistFromEdgeVx1,
                                                                         cvx.normDistFromEdgeVx1 );
                            }
                        }
                    }
                }
            }
            lIdx = idxToNextP;
        }
    }

    m_triangleVxes->assign( triangleVertices );
    m_cellBorderLineVxes->assign( meshAcc.cellBorderLineVxes );
    m_faultCellBorderLineVxes->assign( meshAcc.faultCellBorderLineVxes );

    for ( const auto& it : meshAcc.faultToHighestFaultMeshVxMap )
    {
        m_faultMeshLabelAndAnchorPositions.emplace_back( it.first->name(), it.second );
    }

    calculateSurfaceIntersectionPoints();
}

//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivExtrudedCurveIntersectionGeometryGenerator::generateSurface( cvf::UByteArray* visibleCells )
{
    calculateArrays( visibleCells );

    CVF_ASSERT( m_triangleVxes.notNull() );

    if ( m_triangleVxes->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromTriangleVertexArray( m_triangleVxes.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivExtrudedCurveIntersectionGeometryGenerator::createMeshDrawable()
{
    if ( !m_cellBorderLineVxes.notNull() || m_cellBorderLineVxes->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( m_cellBorderLineVxes.p() );

    cvf::ref<cvf::PrimitiveSetDirect> prim = new cvf::PrimitiveSetDirect( cvf::PT_LINES );
    prim->setIndexCount( m_cellBorderLineVxes->size() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivExtrudedCurveIntersectionGeometryGenerator::createFaultMeshDrawable()
{
    if ( !m_faultCellBorderLineVxes.notNull() || m_faultCellBorderLineVxes->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( m_faultCellBorderLineVxes.p() );

    cvf::ref<cvf::PrimitiveSetDirect> prim = new cvf::PrimitiveSetDirect( cvf::PT_LINES );
    prim->setIndexCount( m_faultCellBorderLineVxes->size() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivExtrudedCurveIntersectionGeometryGenerator::createLineAlongPolylineDrawable()
{
    return RivPolylineGenerator::createLineAlongPolylineDrawable( m_transformedPolyLines );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivExtrudedCurveIntersectionGeometryGenerator::createLineAlongExtrusionLineDrawable( const std::vector<cvf::Vec3d>& extrusionLine )
{
    cvf::ref<caf::DisplayCoordTransform> transform = displayCoordTransform( intersection() );
    std::vector<cvf::Vec3d>              displayCoords;

    for ( const auto& pt : extrusionLine )
    {
        displayCoords.push_back( transform->translateToDisplayCoord( pt ) );
    }

    return RivPolylineGenerator::createLineAlongPolylineDrawable( std::vector<std::vector<cvf::Vec3d>>( { displayCoords } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivExtrudedCurveIntersectionGeometryGenerator::createPointsFromPolylineDrawable()
{
    return RivPolylineGenerator::createPointsFromPolylineDrawable( m_transformedPolyLines );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivExtrudedCurveIntersectionGeometryGenerator::createPointsFromExtrusionLineDrawable( const std::vector<cvf::Vec3d>& extrusionLine )
{
    cvf::ref<caf::DisplayCoordTransform> transform = displayCoordTransform( intersection() );
    std::vector<cvf::Vec3d>              displayCoords;

    for ( const auto& pt : extrusionLine )
    {
        displayCoords.push_back( transform->translateToDisplayCoord( pt ) );
    }

    return RivPolylineGenerator::createPointsFromPolylineDrawable( std::vector<std::vector<cvf::Vec3d>>( { displayCoords } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<cvf::Vec3d>>& RivExtrudedCurveIntersectionGeometryGenerator::flattenedOrOffsettedPolyLines()
{
    return m_transformedPolyLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::pair<QString, cvf::Vec3d>>& RivExtrudedCurveIntersectionGeometryGenerator::faultMeshLabelAndAnchorPositions()
{
    return m_faultMeshLabelAndAnchorPositions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivExtrudedCurveIntersectionGeometryGenerator::triangleToCellIndex() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triangleToCellIdxMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RivIntersectionVertexWeights>& RivExtrudedCurveIntersectionGeometryGenerator::triangleVxToCellCornerInterpolationWeights() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triVxToCellCornerWeights;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivExtrudedCurveIntersectionGeometryGenerator::triangleVxes() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triangleVxes.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivExtrudedCurveIntersectionGeometryGenerator::cellMeshVxes() const
{
    CVF_ASSERT( m_cellBorderLineVxes->size() );
    return m_cellBorderLineVxes.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivExtrudedCurveIntersectionGeometryGenerator::faultMeshVxes() const
{
    return m_faultCellBorderLineVxes.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionGeometryGenerator::ensureGeometryIsCalculated()
{
    calculateArrays( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtrudedCurveIntersection* RivExtrudedCurveIntersectionGeometryGenerator::intersection() const
{
    return m_intersection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivExtrudedCurveIntersectionGeometryGenerator::transformPointByPolylineSegmentIndex( const cvf::Vec3d& domainCoord,
                                                                                                size_t            lineIndex,
                                                                                                size_t            segmentIndex )
{
    CVF_ASSERT( lineIndex < m_lineSegmentTransforms.size() );
    CVF_ASSERT( segmentIndex < m_lineSegmentTransforms[lineIndex].size() );

    // Each line segment along the polyline has a transformation matrix representing the transformation from 3D into
    // flat 2D. Return the transformed domain coord using the required transformation matrix

    return domainCoord.getTransformedPoint( m_lineSegmentTransforms[lineIndex][segmentIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RivExtrudedCurveIntersectionGeometryGenerator::unflattenTransformMatrix( const cvf::Vec3d& intersectionPointFlat ) const
{
    cvf::Mat4d flattenMx = cvf::Mat4d::IDENTITY;

    for ( size_t pLineIdx = 0; pLineIdx < m_transformedPolyLines.size(); pLineIdx++ )
    {
        const std::vector<cvf::Vec3d>& polyLine = m_transformedPolyLines[pLineIdx];
        for ( size_t pIdx = 0; pIdx < polyLine.size(); pIdx++ )
        {
            if ( polyLine[pIdx].x() >= intersectionPointFlat.x() )
            {
                size_t csIdx = pIdx > 0 ? pIdx - 1 : 0;
                flattenMx    = m_lineSegmentTransforms[pLineIdx][csIdx];
                break;
            }
            if ( pIdx == polyLine.size() - 1 )
            {
                flattenMx = m_lineSegmentTransforms[pLineIdx][pIdx];
            }
        }
    }

    return flattenMx.getInverted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivExtrudedCurveIntersectionGeometryGenerator::isAnyGeometryPresent() const
{
    return m_triangleVxes->size() != 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RimSurface*, std::vector<cvf::Vec3d>> RivExtrudedCurveIntersectionGeometryGenerator::transformedSurfaceIntersectionPolylines() const
{
    return m_transformedSurfaceIntersectionPolylines;
}
