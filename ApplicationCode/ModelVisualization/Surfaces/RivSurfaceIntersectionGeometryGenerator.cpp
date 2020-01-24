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
RivSurfaceIntersectionGeometryGenerator::~RivSurfaceIntersectionGeometryGenerator() {}

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
template <typename Vec3Type>
double closestAxisSignedAreaPlanarPolygon( const cvf::Vec3d& planeNormal, const std::vector<Vec3Type>& polygon )
{
    int Z = cvf::GeometryTools::findClosestAxis( planeNormal );
    int X = ( Z + 1 ) % 3;
    int Y = ( Z + 2 ) % 3;

    // Use Shoelace formula to calculate signed area.
    // https://en.wikipedia.org/wiki/Shoelace_formula
    double signedArea = 0.0;
    for ( size_t i = 0; i < polygon.size(); ++i )
    {
        signedArea += ( polygon[( i + 1 ) % polygon.size()][X] + polygon[i][X] ) *
                      ( polygon[( i + 1 ) % polygon.size()][Y] - polygon[i][Y] );
    }

    return ( planeNormal[Z] > 0 ) ? signedArea : -signedArea;
}

class ClipperInterface
{
public:
    static ClipperLib::IntPoint toClipperPoint( const cvf::Vec3d& cvfPoint )
    {
        int xInt = cvfPoint.x() * clipperConversionFactor;
        int yInt = cvfPoint.y() * clipperConversionFactor;
        return ClipperLib::IntPoint( xInt, yInt, 0 );
    }

    static cvf::Vec3d fromClipperPoint( const ClipperLib::IntPoint& clipPoint )
    {
        return cvf::Vec3d( clipPoint.X, clipPoint.Y, 0.0 ) / clipperConversionFactor;
    }
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    static std::vector<std::vector<cvf::Vec3d>>
        subtractAndSimplifyPolygons( const std::vector<cvf::Vec3d>&              sourcePolygon,
                                     const std::vector<std::vector<cvf::Vec3d>>& polygonsToSubtract )
    {
        ClipperLib::Paths unionOfPolygonsToSubtract;
        {
            ClipperLib::Clipper unionator;

            for ( const auto& path : polygonsToSubtract )
            {
                ClipperLib::Path polyToSubtractPath;
                for ( const auto& v : path )
                {
                    polyToSubtractPath.push_back( toClipperPoint( v ) );
                }

                unionator.AddPath( polyToSubtractPath, ClipperLib::ptSubject, true );
            }

            unionator.Execute( ClipperLib::ctUnion,
                               unionOfPolygonsToSubtract,
                               ClipperLib::pftEvenOdd,
                               ClipperLib::pftEvenOdd );
        }

        ClipperLib::Path sourcePolygonPath;

        for ( const auto& v : sourcePolygon )
        {
            sourcePolygonPath.push_back( toClipperPoint( v ) );
        }

        ClipperLib::Clipper subtractor;

        subtractor.AddPath( sourcePolygonPath, ClipperLib::ptSubject, true );
        subtractor.AddPaths( unionOfPolygonsToSubtract, ClipperLib::ptClip, true );

        ClipperLib::Paths subtractionResultPaths;
        subtractor.Execute( ClipperLib::ctDifference,
                            subtractionResultPaths,
                            ClipperLib::pftEvenOdd,
                            ClipperLib::pftEvenOdd );

        ClipperLib::CleanPolygons( subtractionResultPaths, 3 );

        std::vector<std::vector<cvf::Vec3d>> clippedPolygons;

        // Convert back to std::vector<std::vector<cvf::Vec3d> >

        for ( ClipperLib::Path pathInSol : subtractionResultPaths )
        {
            std::vector<cvf::Vec3d> clippedPolygon;
            for ( ClipperLib::IntPoint IntPosition : pathInSol )
            {
                clippedPolygon.push_back( fromClipperPoint( IntPosition ) );
            }

            clippedPolygons.push_back( clippedPolygon );
        }

        return clippedPolygons;
    }

private:
    static double clipperConversionFactor;
};

double ClipperInterface::clipperConversionFactor = 100; // For transform to clipper int

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfaceIntersectionGeometryGenerator::calculateArrays()
{
    if ( m_triangleVxes->size() ) return;
    if ( m_hexGrid.isNull() ) return;

    std::vector<cvf::Vec3f> outputTriangleVertices;

    MeshLinesAccumulator meshAcc( m_hexGrid.p() );

    cvf::BoundingBox gridBBox = m_hexGrid->boundingBox();

    m_usedSurfaceData = m_surfaceInView->surface()->surfaceData();

    const std::vector<cvf::Vec3d>& nativeVertices        = m_usedSurfaceData->vertices();
    const std::vector<unsigned>&   nativeTriangleIndices = m_usedSurfaceData->triangleIndices();
    cvf::Vec3d                     displayModelOffset    = m_hexGrid->displayOffset();

    for ( size_t ntVxIdx = 0; ntVxIdx < nativeTriangleIndices.size(); ntVxIdx += 3 )
    {
        cvf::Vec3d p0 = nativeVertices[nativeTriangleIndices[ntVxIdx + 0]];
        cvf::Vec3d p1 = nativeVertices[nativeTriangleIndices[ntVxIdx + 1]];
        cvf::Vec3d p2 = nativeVertices[nativeTriangleIndices[ntVxIdx + 2]];

        cvf::BoundingBox triangleBBox;
        triangleBBox.add( p0 );
        triangleBBox.add( p1 );
        triangleBBox.add( p2 );

        cvf::Vec3d maxHeightVec;

        std::vector<size_t> triIntersectedCellCandidates;
        m_hexGrid->findIntersectingCells( triangleBBox, &triIntersectedCellCandidates );

        cvf::Plane plane;
        plane.setFromPoints( p0, p1, p2 );

        std::vector<caf::HexGridIntersectionTools::ClipVx> hexPlaneCutTriangleVxes;
        hexPlaneCutTriangleVxes.reserve( 5 * 3 );

        std::vector<int> cellFaceForEachTriangleEdge;
        cellFaceForEachTriangleEdge.reserve( 5 * 3 );

        std::array<cvf::Vec3d, 8> cellCorners;
        std::array<size_t, 8>     cornerIndices;

        size_t startOfGeneratedTrianglesForNativeTriangle = outputTriangleVertices.size();

        for ( size_t ticIdx = 0; ticIdx < triIntersectedCellCandidates.size(); ++ticIdx )
        {
            size_t globalCellIdx = triIntersectedCellCandidates[ticIdx];

            if ( !m_hexGrid->useCell( globalCellIdx ) ) continue;

            m_hexGrid->cellCornerVertices( globalCellIdx, &cellCorners[0] );
            m_hexGrid->cellCornerIndices( globalCellIdx, &cornerIndices[0] );

            hexPlaneCutTriangleVxes.clear();
            int triangleCount = caf::HexGridIntersectionTools::planeHexIntersectionMC( plane,
                                                                                       &cellCorners[0],
                                                                                       &cornerIndices[0],
                                                                                       &hexPlaneCutTriangleVxes,
                                                                                       &cellFaceForEachTriangleEdge );

            if ( triangleCount == 0 ) continue;

            std::vector<cvf::Vec3d> cellCutTriangles;
            for ( const auto& clipVx : hexPlaneCutTriangleVxes )
            {
                cellCutTriangles.push_back( clipVx.vx );
            }

            std::vector<cvf::Vec3d> clippedTriangleVxes;
            std::vector<int>        cellFaceForEachClippedTriangleEdge;

            caf::HexGridIntersectionTools::clipPlanarTrianglesWithInPlaneTriangle( cellCutTriangles,
                                                                                   cellFaceForEachTriangleEdge,
                                                                                   p0,
                                                                                   p1,
                                                                                   p2,
                                                                                   &clippedTriangleVxes,
                                                                                   &cellFaceForEachClippedTriangleEdge );

            size_t clippedTriangleCount = clippedTriangleVxes.size() / 3;

            for ( uint clippTrIdx = 0; clippTrIdx < clippedTriangleCount; ++clippTrIdx )
            {
                uint triVxIdx = clippTrIdx * 3;

                // Accumulate triangle vertices

                cvf::Vec3d point0( clippedTriangleVxes[triVxIdx + 0] - displayModelOffset );
                cvf::Vec3d point1( clippedTriangleVxes[triVxIdx + 1] - displayModelOffset );
                cvf::Vec3d point2( clippedTriangleVxes[triVxIdx + 2] - displayModelOffset );

                outputTriangleVertices.emplace_back( point0 );
                outputTriangleVertices.emplace_back( point1 );
                outputTriangleVertices.emplace_back( point2 );

                // Accumulate mesh lines

                meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge, triVxIdx + 0, globalCellIdx, point0, point1 );
                meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge, triVxIdx + 1, globalCellIdx, point1, point2 );
                meshAcc.accumulateMeshLines( cellFaceForEachClippedTriangleEdge, triVxIdx + 2, globalCellIdx, point2, point0 );

                // Mapping to cell index

                m_triangleToCellIdxMap.push_back( globalCellIdx );

                // Interpolation from nodes
                for ( int i = 0; i < 3; ++i )
                {
                    cvf::Vec3d cvx = clippedTriangleVxes[triVxIdx + i];

                    std::array<double, 8> cornerWeights = caf::HexInterpolator::vertexWeights( cellCorners, cvx );
                    m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights( cornerIndices, cornerWeights ) );
                }
            }
        }

        // Add triangles for the part of the native triangle outside any gridcells

        if ( startOfGeneratedTrianglesForNativeTriangle == outputTriangleVertices.size() )
        {
            // No triangles created, use the complete native triangle
            outputTriangleVertices.push_back( cvf::Vec3f( p0 - displayModelOffset ) );
            outputTriangleVertices.push_back( cvf::Vec3f( p1 - displayModelOffset ) );
            outputTriangleVertices.push_back( cvf::Vec3f( p2 - displayModelOffset ) );

            m_triangleToCellIdxMap.push_back( cvf::UNDEFINED_SIZE_T );

            m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights() );
            m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights() );
            m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights() );
        }
        else
        {
            // Use area to check if the native triangle was completely covered by the intersection triangles
            double nativeTriangleArea         = 0.0;
            double intersectionArea           = 0.0;
            double minSignificantTriangleArea = 0.0;
            {
                std::vector<cvf::Vec3f> nativeTriangle = {cvf::Vec3f( p0 - displayModelOffset ),
                                                          cvf::Vec3f( p1 - displayModelOffset ),
                                                          cvf::Vec3f( p2 - displayModelOffset )};

                nativeTriangleArea         = closestAxisSignedAreaPlanarPolygon( plane.normal(), nativeTriangle );
                minSignificantTriangleArea = 1e-4 * nativeTriangleArea;

                std::vector<cvf::Vec3f> intersectionTriangle( 3 );

                for ( size_t tvxIdx = startOfGeneratedTrianglesForNativeTriangle; tvxIdx < outputTriangleVertices.size();
                      tvxIdx += 3 )
                {
                    std::copy( outputTriangleVertices.begin() + tvxIdx,
                               outputTriangleVertices.begin() + tvxIdx + 3,
                               intersectionTriangle.begin() );
                    intersectionArea += closestAxisSignedAreaPlanarPolygon( plane.normal(), intersectionTriangle );
                }

                // If we have covered enough, do not try to create triangles for the rest

                if ( ( nativeTriangleArea - intersectionArea ) < minSignificantTriangleArea ) continue;
            }

            // Subtract the created triangles from the native triangle
            // We need to transform the triangles into x/y plane to do the polygon operation

            // Find the local CS for the native triangle in display coords

            cvf::Mat4d nativeTriangleCS;
            {
                cvf::Vec3d ez    = plane.normal().getNormalized();
                cvf::Vec3d ex    = ( p1 - p0 ).getNormalized();
                cvf::Vec3d ey    = ez ^ ex;
                nativeTriangleCS = cvf::Mat4d::fromCoordSystemAxes( &ex, &ey, &ez );
                nativeTriangleCS.setTranslation( p0 - displayModelOffset );
            }

            cvf::Mat4d invNativeTriangleCS = nativeTriangleCS.getInverted();

            std::vector<std::vector<cvf::Vec3d>> polygonsToSubtract;
            for ( size_t tvxIdx = startOfGeneratedTrianglesForNativeTriangle; tvxIdx < outputTriangleVertices.size();
                  tvxIdx += 3 )
            {
                std::vector<cvf::Vec3d> triangle =
                    {cvf::Vec3d( outputTriangleVertices[tvxIdx + 0] ).getTransformedPoint( invNativeTriangleCS ),
                     cvf::Vec3d( outputTriangleVertices[tvxIdx + 1] ).getTransformedPoint( invNativeTriangleCS ),
                     cvf::Vec3d( outputTriangleVertices[tvxIdx + 2] ).getTransformedPoint( invNativeTriangleCS )};
                polygonsToSubtract.push_back( triangle );
            }

            std::vector<cvf::Vec3d> nativeTrianglePoly =
                {( cvf::Vec3d( cvf::Vec3f( p0 - displayModelOffset ) ) ).getTransformedPoint( invNativeTriangleCS ),
                 ( cvf::Vec3d( cvf::Vec3f( p1 - displayModelOffset ) ) ).getTransformedPoint( invNativeTriangleCS ),
                 ( cvf::Vec3d( cvf::Vec3f( p2 - displayModelOffset ) ) ).getTransformedPoint( invNativeTriangleCS )};

            std::vector<std::vector<cvf::Vec3d>> remainingPolygons;

            remainingPolygons = ClipperInterface::subtractAndSimplifyPolygons( nativeTrianglePoly, polygonsToSubtract );

            // Check for holes in solution
            bool hasHoles = false;

            for ( const auto& remainingPolygon : remainingPolygons )
            {
                double area = closestAxisSignedAreaPlanarPolygon( cvf::Vec3d::Z_AXIS, remainingPolygon );
                if ( area < -minSignificantTriangleArea )
                {
                    hasHoles = true;
                }
            }

            if ( hasHoles ) continue; // Cant tesselate polygons with holes

            // Add the remains

            for ( const auto& remainingPolygon : remainingPolygons )
            {
                if ( remainingPolygon.empty() ) continue;

                cvf::EarClipTesselator tess;
                tess.setNormal( plane.normal() );
                tess.setMinTriangleArea( minSignificantTriangleArea );
                cvf::Vec3dArray cvfNodes( remainingPolygon );
                tess.setGlobalNodeArray( cvfNodes );

                std::vector<size_t> polyIndexes;
                for ( size_t idx = 0; idx < remainingPolygon.size(); ++idx )
                {
                    polyIndexes.push_back( idx );
                }
                tess.setPolygonIndices( polyIndexes );

                std::vector<size_t> triangleIndices;
                bool                isTesselationOk = tess.calculateTriangles( &triangleIndices );

                if ( !isTesselationOk )
                {
                    // continue;
                    // CVF_ASSERT( false );
                }

                double tesselatedArea = 0;

                for ( size_t idx = 0; idx < triangleIndices.size(); idx += 3 )
                {
                    cvf::Vec3f tp1(
                        ( remainingPolygon[triangleIndices[idx + 0]] ).getTransformedPoint( nativeTriangleCS ) );
                    cvf::Vec3f tp2(
                        ( remainingPolygon[triangleIndices[idx + 1]] ).getTransformedPoint( nativeTriangleCS ) );
                    cvf::Vec3f tp3(
                        ( remainingPolygon[triangleIndices[idx + 2]] ).getTransformedPoint( nativeTriangleCS ) );

                    outputTriangleVertices.push_back( tp1 );
                    outputTriangleVertices.push_back( tp2 );
                    outputTriangleVertices.push_back( tp3 );

                    std::vector<cvf::Vec3f> nativeTriangle = {tp1, tp2, tp3};

                    tesselatedArea += closestAxisSignedAreaPlanarPolygon( plane.normal(), nativeTriangle );

                    m_triangleToCellIdxMap.push_back( cvf::UNDEFINED_SIZE_T );

                    m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights() );
                    m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights() );
                    m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights() );
                }

                if ( ( tesselatedArea - 20 * minSignificantTriangleArea ) > ( nativeTriangleArea - intersectionArea ) )
                {
                    double overlapArea = tesselatedArea - ( nativeTriangleArea - intersectionArea );

                    RiaLogging::debug( "Surface intersection triangularization overlap detected : " +
                                       QString::number( overlapArea ) );

                    // CVF_ASSERT( false );
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

    CVF_ASSERT( m_triangleVxes.notNull() );

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
