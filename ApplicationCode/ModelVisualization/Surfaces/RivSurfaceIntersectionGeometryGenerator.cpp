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

#include "RivSurfaceIntersectionGeometryGenerator.h"

#include "RigMainGrid.h"
#include "RigResultAccessor.h"

#include "RigSurface.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"

#include "RivExtrudedCurveIntersectionPartMgr.h"
#include "RivHexGridIntersectionTools.h"
#include "RivPolylineGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include "cvfDrawableGeo.h"
#include "cvfGeometryTools.h"
#include "cvfPlane.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRay.h"
#include "cvfScalarMapper.h"

#include "../cafHexInterpolator/cafHexInterpolator.h"
#include "RivSectionFlattner.h"

#include "RiaLogging.h"
#include "clipper.hpp"

cvf::ref<caf::DisplayCoordTransform> displayCoordTransform( const RimIntersection* intersection )
{
    Rim3dView* rimView = nullptr;
    intersection->firstAncestorOrThisOfType( rimView );
    CVF_ASSERT( rimView );

    cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
    return transForm;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSurfaceIntersectionGeometryGenerator::RivSurfaceIntersectionGeometryGenerator( RimSurfaceInView* surfInView,
                                                                                  const RivIntersectionHexGridInterface* grid )
    : m_surfaceInView( surfInView )
    , m_hexGrid( grid )
{
    m_triangleVxes            = new cvf::Vec3fArray;
    m_cellBorderLineVxes      = new cvf::Vec3fArray;
    m_faultCellBorderLineVxes = new cvf::Vec3fArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSurfaceIntersectionGeometryGenerator::~RivSurfaceIntersectionGeometryGenerator()
{
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
#define isFace( faceEnum ) ( 0 <= faceEnum && faceEnum <= 5 )
        using FaceType = cvf::StructGridInterface::FaceType;

        if ( isFace( cellFaceForEachClippedTriangleEdge[triVxIdx] ) )
        {
            const RigFault* fault =
                m_hexGrid->findFaultFromCellIndexAndCellFace( globalCellIdx,
                                                              (FaceType)cellFaceForEachClippedTriangleEdge[triVxIdx] );
            if ( fault )
            {
                cvf::Vec3d highestVx = p0.z() > p1.z() ? p0 : p1;

                auto itIsInsertedPair = faultToHighestFaultMeshVxMap.insert( {fault, highestVx} );
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
void RivSurfaceIntersectionGeometryGenerator::calculateArrays()
{
    if ( m_triangleVxes->size() ) return;
    if ( m_hexGrid.isNull() ) return;
    if ( !m_surfaceInView->surface()->surfaceData() ) return;

    std::vector<cvf::Vec3f> outputTriangleVertices;

    MeshLinesAccumulator meshAcc( m_hexGrid.p() );

    m_usedSurfaceData = m_surfaceInView->surface()->surfaceData();

    const std::vector<cvf::Vec3d>& nativeVertices        = m_usedSurfaceData->vertices();
    const std::vector<unsigned>&   nativeTriangleIndices = m_usedSurfaceData->triangleIndices();
    cvf::Vec3d                     displayModelOffset    = m_hexGrid->displayOffset();
    double                         depthOffset           = m_surfaceInView->depthOffset();

    m_triVxToCellCornerWeights.reserve( nativeTriangleIndices.size() * 24 );
    outputTriangleVertices.reserve( nativeTriangleIndices.size() * 24 );

#pragma omp parallel num_threads( 6 ) // More threads have nearly no effect
    {
        // Loop local memory allocation.
        // Must be thread private in omp parallelization
        std::vector<caf::HexGridIntersectionTools::ClipVx> hexPlaneCutTriangleVxes;
        hexPlaneCutTriangleVxes.reserve( 2 * 6 * 3 );

        std::vector<int> cellFaceForEachTriangleEdge;
        cellFaceForEachTriangleEdge.reserve( 2 * 6 * 3 );

        std::vector<cvf::Vec3d> cellCutTriangles;
        cellCutTriangles.reserve( 2 * 6 * 3 );

        std::vector<cvf::Vec3d> clippedTriangleVxes;
        clippedTriangleVxes.reserve( 2 * 6 * 3 );

        std::vector<int> cellFaceForEachClippedTriangleEdge;
        cellFaceForEachClippedTriangleEdge.reserve( 2 * 6 * 3 );

        // End loop local memory

#pragma omp for // default scheduling absolutely best
        for ( int ntVxIdx = 0; ntVxIdx < static_cast<int>( nativeTriangleIndices.size() ); ntVxIdx += 3 )
        {
            cvf::Vec3d p0 = nativeVertices[nativeTriangleIndices[ntVxIdx + 0]];
            cvf::Vec3d p1 = nativeVertices[nativeTriangleIndices[ntVxIdx + 1]];
            cvf::Vec3d p2 = nativeVertices[nativeTriangleIndices[ntVxIdx + 2]];

            p0.z() = p0.z() - depthOffset;
            p1.z() = p1.z() - depthOffset;
            p2.z() = p2.z() - depthOffset;

            cvf::BoundingBox triangleBBox;
            triangleBBox.add( p0 );
            triangleBBox.add( p1 );
            triangleBBox.add( p2 );

            cvf::Vec3d maxHeightVec;

            std::vector<size_t> triIntersectedCellCandidates;
            m_hexGrid->findIntersectingCells( triangleBBox, &triIntersectedCellCandidates );

            cvf::Plane plane;
            plane.setFromPoints( p0, p1, p2 );

            std::array<cvf::Vec3d, 8> cellCorners;
            std::array<size_t, 8>     cornerIndices;

            for ( size_t ticIdx = 0; ticIdx < triIntersectedCellCandidates.size(); ++ticIdx )
            {
                size_t globalCellIdx = triIntersectedCellCandidates[ticIdx];

                if ( !m_hexGrid->useCell( globalCellIdx ) ) continue;

                hexPlaneCutTriangleVxes.clear();
                cellFaceForEachTriangleEdge.clear();
                cellCutTriangles.clear();

                m_hexGrid->cellCornerVertices( globalCellIdx, &cellCorners[0] );
                m_hexGrid->cellCornerIndices( globalCellIdx, &cornerIndices[0] );

                int triangleCount =
                    caf::HexGridIntersectionTools::planeHexIntersectionMCTet( plane,
                                                                              &cellCorners[0],
                                                                              &cornerIndices[0],
                                                                              &hexPlaneCutTriangleVxes,
                                                                              &cellFaceForEachTriangleEdge );

                if ( triangleCount == 0 ) continue;

                for ( const auto& clipVx : hexPlaneCutTriangleVxes )
                {
                    cellCutTriangles.push_back( clipVx.vx );
                }

                clippedTriangleVxes.clear();
                cellFaceForEachClippedTriangleEdge.clear();

                caf::HexGridIntersectionTools::clipPlanarTrianglesWithInPlaneTriangle( cellCutTriangles,
                                                                                       cellFaceForEachTriangleEdge,
                                                                                       p0,
                                                                                       p1,
                                                                                       p2,
                                                                                       &clippedTriangleVxes,
                                                                                       &cellFaceForEachClippedTriangleEdge );
                if ( clippedTriangleVxes.empty() ) continue;

                for ( uint triVxIdx = 0; triVxIdx < clippedTriangleVxes.size(); triVxIdx += 3 )
                {
                    // Accumulate triangle vertices

                    cvf::Vec3d point0( clippedTriangleVxes[triVxIdx + 0] - displayModelOffset );
                    cvf::Vec3d point1( clippedTriangleVxes[triVxIdx + 1] - displayModelOffset );
                    cvf::Vec3d point2( clippedTriangleVxes[triVxIdx + 2] - displayModelOffset );

                    // Interpolation from nodes

                    std::array<double, 8> cornerWeights0 =
                        caf::HexInterpolator::vertexWeights( cellCorners, clippedTriangleVxes[triVxIdx + 0] );
                    std::array<double, 8> cornerWeights1 =
                        caf::HexInterpolator::vertexWeights( cellCorners, clippedTriangleVxes[triVxIdx + 1] );
                    std::array<double, 8> cornerWeights2 =
                        caf::HexInterpolator::vertexWeights( cellCorners, clippedTriangleVxes[triVxIdx + 2] );

#pragma omp critical
                    {
                        outputTriangleVertices.emplace_back( point0 );
                        outputTriangleVertices.emplace_back( point1 );
                        outputTriangleVertices.emplace_back( point2 );

                        // Accumulate mesh lines

                        meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge,
                                                     triVxIdx + 0,
                                                     globalCellIdx,
                                                     point0,
                                                     point1 );
                        meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge,
                                                     triVxIdx + 1,
                                                     globalCellIdx,
                                                     point1,
                                                     point2 );
                        meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge,
                                                     triVxIdx + 2,
                                                     globalCellIdx,
                                                     point2,
                                                     point0 );

                        // Mapping to cell index

                        m_triangleToCellIdxMap.push_back( globalCellIdx );

                        // Interpolation from nodes
                        m_triVxToCellCornerWeights.emplace_back(
                            RivIntersectionVertexWeights( cornerIndices, cornerWeights0 ) );
                        m_triVxToCellCornerWeights.emplace_back(
                            RivIntersectionVertexWeights( cornerIndices, cornerWeights1 ) );
                        m_triVxToCellCornerWeights.emplace_back(
                            RivIntersectionVertexWeights( cornerIndices, cornerWeights2 ) );
                    }
                }
            }
        }
    }

    m_triangleVxes->assign( outputTriangleVertices );
    m_cellBorderLineVxes->assign( meshAcc.cellBorderLineVxes );
    m_faultCellBorderLineVxes->assign( meshAcc.faultCellBorderLineVxes );

    for ( const auto& it : meshAcc.faultToHighestFaultMeshVxMap )
    {
        m_faultMeshLabelAndAnchorPositions.push_back( {it.first->name(), it.second} );
    }
}

//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivSurfaceIntersectionGeometryGenerator::generateSurface()
{
    calculateArrays();

    if ( m_triangleVxes->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromTriangleVertexArray( m_triangleVxes.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivSurfaceIntersectionGeometryGenerator::createMeshDrawable()
{
    if ( !( m_cellBorderLineVxes.notNull() && m_cellBorderLineVxes->size() != 0 ) ) return nullptr;

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
cvf::ref<cvf::DrawableGeo> RivSurfaceIntersectionGeometryGenerator::createFaultMeshDrawable()
{
    if ( !( m_faultCellBorderLineVxes.notNull() && m_faultCellBorderLineVxes->size() != 0 ) ) return nullptr;

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
const std::vector<std::pair<QString, cvf::Vec3d>>&
    RivSurfaceIntersectionGeometryGenerator::faultMeshLabelAndAnchorPositions()
{
    return m_faultMeshLabelAndAnchorPositions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivSurfaceIntersectionGeometryGenerator::triangleToCellIndex() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triangleToCellIdxMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RivIntersectionVertexWeights>&
    RivSurfaceIntersectionGeometryGenerator::triangleVxToCellCornerInterpolationWeights() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triVxToCellCornerWeights;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivSurfaceIntersectionGeometryGenerator::triangleVxes() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triangleVxes.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView* RivSurfaceIntersectionGeometryGenerator::intersection() const
{
    return m_surfaceInView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivSurfaceIntersectionGeometryGenerator::isAnyGeometryPresent() const
{
    if ( m_triangleVxes->size() == 0 )
    {
        return false;
    }
    else
    {
        return true;
    }
}
