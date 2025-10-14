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

#include "gtest/gtest.h"

#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "cvfArrayWrapperConst.h"
#include "cvfArrayWrapperToEdit.h"

#include "cvfBoundingBoxTree.h"
#include "cvfGeometryTools.h"

#include <array>
#include <cmath>

using namespace cvf;

template <typename NodeArrayType, typename NodeType, typename IndexType>
NodeType quadNormal( ArrayWrapperConst<NodeArrayType, NodeType> nodeCoords, const IndexType cubeFaceIndices[4] )
{
    return ( nodeCoords[cubeFaceIndices[2]] - nodeCoords[cubeFaceIndices[0]] ) ^
           ( nodeCoords[cubeFaceIndices[3]] - nodeCoords[cubeFaceIndices[1]] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> createVertices()
{
    std::vector<cvf::Vec3d> vxs;
    vxs.resize( 14, cvf::Vec3d::ZERO );

    vxs[0]  = cvf::Vec3d( 0, 0, 0 );
    vxs[1]  = cvf::Vec3d( 1, 0, 0 );
    vxs[2]  = cvf::Vec3d( 1, 1, 0 );
    vxs[3]  = cvf::Vec3d( 0, 1, 0 );
    vxs[4]  = cvf::Vec3d( -0.4, -0.2, 0.0 );
    vxs[5]  = cvf::Vec3d( 0.4, 0.6, 0.0 );
    vxs[6]  = cvf::Vec3d( 0.8, 0.2, 0.0 );
    vxs[7]  = cvf::Vec3d( 0.0, -0.6, 0.0 );
    vxs[8]  = cvf::Vec3d( 1.0, 1.2, 0.0 );
    vxs[9]  = cvf::Vec3d( 1.4, 0.8, 0.0 );
    vxs[10] = cvf::Vec3d( 0.4, -0.2, 0.0 );
    vxs[11] = cvf::Vec3d( 1.2, 0.6, 0.0 );
    vxs[12] = cvf::Vec3d( 1.6, 0.2, 0.0 );
    vxs[13] = cvf::Vec3d( 0.8, -0.6, 0.0 );

    return vxs;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::array<cvf::uint, 4>> getCubeFaces()
{
    std::vector<std::array<cvf::uint, 4>> cubeFaces;

    cubeFaces.resize( 4 );
    cubeFaces[0] = { 0, 1, 2, 3 };
    cubeFaces[1] = { 4, 5, 6, 7 };
    cubeFaces[2] = { 5, 8, 9, 6 };
    cubeFaces[3] = { 10, 11, 12, 13 };

    return cubeFaces;
}

std::ostream& operator<<( std::ostream& stream, std::vector<cvf::uint> v )
{
    for ( size_t i = 0; i < v.size(); ++i )
    {
        stream << v[i] << " ";
    }
    return stream;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( CellFaceIntersectionTst, Intersection1 )
{
    std::vector<cvf::Vec3d> nodes = createVertices();

    std::vector<cvf::Vec3d> additionalVertices;

    std::vector<std::vector<cvf::uint>> overlapPolygons;
    auto                                faces = getCubeFaces();

    EdgeIntersectStorage<cvf::uint> edgeIntersectionStorage;
    edgeIntersectionStorage.setVertexCount( nodes.size() );
    {
        std::vector<cvf::uint> polygon;
        bool                   isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                      &additionalVertices,
                                                                      &edgeIntersectionStorage,
                                                                      wrapArrayConst( &nodes ),
                                                                      faces[0].data(),
                                                                      faces[1].data(),
                                                                      1e-6 );

        EXPECT_EQ( (size_t)5, polygon.size() );
        EXPECT_EQ( (size_t)2, additionalVertices.size() );
        EXPECT_TRUE( isOk );
        overlapPolygons.push_back( polygon );
        std::cout << polygon << std::endl;
    }

    {
        std::vector<cvf::uint> polygon;
        bool                   isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                      &additionalVertices,
                                                                      &edgeIntersectionStorage,
                                                                      wrapArrayConst( &nodes ),
                                                                      faces[0].data(),
                                                                      faces[2].data(),
                                                                      1e-6 );

        EXPECT_EQ( (size_t)5, polygon.size() );
        EXPECT_EQ( (size_t)4, additionalVertices.size() );
        EXPECT_TRUE( isOk );
        overlapPolygons.push_back( polygon );
        std::cout << polygon << std::endl;
    }

    {
        std::vector<cvf::uint> polygon;
        bool                   isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                      &additionalVertices,
                                                                      &edgeIntersectionStorage,
                                                                      wrapArrayConst( &nodes ),
                                                                      faces[0].data(),
                                                                      faces[3].data(),
                                                                      1e-6 );

        EXPECT_EQ( (size_t)3, polygon.size() );
        EXPECT_EQ( (size_t)6, additionalVertices.size() );
        EXPECT_TRUE( isOk );
        overlapPolygons.push_back( polygon );
        std::cout << polygon << std::endl;
    }

    nodes.insert( nodes.end(), additionalVertices.begin(), additionalVertices.end() );
    std::vector<cvf::uint> basePolygon;
    basePolygon.insert( basePolygon.begin(), faces[0].data(), &( faces[0].data()[4] ) );

    for ( cvf::uint vxIdx = 0; vxIdx < nodes.size(); ++vxIdx )
    {
        GeometryTools::insertVertexInPolygon( &basePolygon, wrapArrayConst( &nodes ), vxIdx, 1e-6 );
    }

    EXPECT_EQ( (size_t)8, basePolygon.size() );
    std::cout << "Bp: " << basePolygon << std::endl;

    for ( size_t pIdx = 0; pIdx < overlapPolygons.size(); ++pIdx )
    {
        for ( cvf::uint vxIdx = 0; vxIdx < nodes.size(); ++vxIdx )
        {
            GeometryTools::insertVertexInPolygon( &overlapPolygons[pIdx], wrapArrayConst( &nodes ), vxIdx, 1e-6 );
        }

        if ( pIdx == 0 )
        {
            EXPECT_EQ( (size_t)5, overlapPolygons[pIdx].size() );
        }
        if ( pIdx == 1 )
        {
            EXPECT_EQ( (size_t)5, overlapPolygons[pIdx].size() );
        }
        if ( pIdx == 2 )
        {
            EXPECT_EQ( (size_t)4, overlapPolygons[pIdx].size() );
        }

        std::cout << "Op" << pIdx << ":" << overlapPolygons[pIdx] << std::endl;
    }

    Vec3d             normal = quadNormal( wrapArrayConst( &nodes ), faces[0].data() );
    std::vector<bool> faceOverlapPolygonWindingSameAsCubeFaceFlags;
    faceOverlapPolygonWindingSameAsCubeFaceFlags.resize( overlapPolygons.size(), true );

    {
        std::vector<cvf::uint> freeFacePolygon;
        bool                   hasHoles = false;

        std::vector<std::vector<cvf::uint>*> overlapPolygonPtrs;
        for ( size_t pIdx = 0; pIdx < overlapPolygons.size(); ++pIdx )
        {
            overlapPolygonPtrs.push_back( &( overlapPolygons[pIdx] ) );
        }

        GeometryTools::calculatePartiallyFreeCubeFacePolygon( wrapArrayConst( &nodes ),
                                                              wrapArrayConst( &basePolygon ),
                                                              normal,
                                                              overlapPolygonPtrs,
                                                              faceOverlapPolygonWindingSameAsCubeFaceFlags,
                                                              &freeFacePolygon,
                                                              &hasHoles );

        EXPECT_EQ( (size_t)4, freeFacePolygon.size() );
        EXPECT_FALSE( hasHoles );
        std::cout << "FF1: " << freeFacePolygon << std::endl;
    }

    {
        std::vector<cvf::uint> freeFacePolygon;
        bool                   hasHoles = false;

        std::vector<std::vector<cvf::uint>*> overlapPolygonPtrs;
        for ( size_t pIdx = 0; pIdx < 1; ++pIdx )
        {
            overlapPolygonPtrs.push_back( &( overlapPolygons[pIdx] ) );
        }

        GeometryTools::calculatePartiallyFreeCubeFacePolygon( wrapArrayConst( &nodes ),
                                                              wrapArrayConst( &basePolygon ),
                                                              normal,
                                                              overlapPolygonPtrs,
                                                              faceOverlapPolygonWindingSameAsCubeFaceFlags,
                                                              &freeFacePolygon,
                                                              &hasHoles );

        EXPECT_EQ( (size_t)9, freeFacePolygon.size() );
        EXPECT_FALSE( hasHoles );

        std::cout << "FF2: " << freeFacePolygon << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

TEST( CellFaceIntersectionTst, Intersection )
{
    std::vector<cvf::Vec3d> additionalVertices;
    cvf::Vec3dArray         nodes;
    std::vector<size_t>     polygon;

    cvf::Array<size_t> ids;
    size_t             cv1CubeFaceIndices[4] = { 0, 1, 2, 3 };
    size_t             cv2CubeFaceIndices[4] = { 4, 5, 6, 7 };

    nodes.resize( 8 );
    nodes.setAll( cvf::Vec3d( 0, 0, 0 ) );
    EdgeIntersectStorage<size_t> edgeIntersectionStorage;
    edgeIntersectionStorage.setVertexCount( nodes.size() );

    // Face 1
    nodes[0] = cvf::Vec3d( 0, 0, 0 );
    nodes[1] = cvf::Vec3d( 1, 0, 0 );
    nodes[2] = cvf::Vec3d( 1, 1, 0 );
    nodes[3] = cvf::Vec3d( 0, 1, 0 );
    // Face 2
    nodes[4] = cvf::Vec3d( 0, 0, 0 );
    nodes[5] = cvf::Vec3d( 1, 0, 0 );
    nodes[6] = cvf::Vec3d( 1, 1, 0 );
    nodes[7] = cvf::Vec3d( 0, 1, 0 );

    bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                  &additionalVertices,
                                                                  &edgeIntersectionStorage,
                                                                  wrapArrayConst( &nodes ),
                                                                  cv1CubeFaceIndices,
                                                                  cv2CubeFaceIndices,
                                                                  1e-6 );
    EXPECT_EQ( (size_t)4, polygon.size() );
    EXPECT_EQ( (size_t)0, additionalVertices.size() );
    EXPECT_TRUE( isOk );

    // Face 1
    nodes[0] = cvf::Vec3d( 0, 0, 0 );
    nodes[1] = cvf::Vec3d( 1, 0, 0 );
    nodes[2] = cvf::Vec3d( 1, 1, 0 );
    nodes[3] = cvf::Vec3d( 0, 1, 0 );
    // Face 2
    nodes[4] = cvf::Vec3d( 0.5, -0.25, 0 );
    nodes[5] = cvf::Vec3d( 1.25, 0.5, 0 );
    nodes[6] = cvf::Vec3d( 0.5, 1.25, 0 );
    nodes[7] = cvf::Vec3d( -0.25, 0.5, 0 );
    polygon.clear();

    isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                             &additionalVertices,
                                                             &edgeIntersectionStorage,
                                                             wrapArrayConst( &nodes ),
                                                             cv1CubeFaceIndices,
                                                             cv2CubeFaceIndices,
                                                             1e-6 );
    EXPECT_EQ( (size_t)8, polygon.size() );
    EXPECT_EQ( (size_t)8, additionalVertices.size() );
    EXPECT_TRUE( isOk );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( CellFaceIntersectionTst, FreeFacePolygon )
{
    std::vector<cvf::Vec3d> additionalVertices;
    cvf::Vec3dArray         nodes;
    std::vector<size_t>     polygon;

    cvf::Array<size_t> ids;
    size_t             cv1CubeFaceIndices[4] = { 0, 1, 2, 3 };
    size_t             cv2CubeFaceIndices[4] = { 4, 5, 6, 7 };

    nodes.resize( 8 );
    nodes.setAll( cvf::Vec3d( 0, 0, 0 ) );
    EdgeIntersectStorage<size_t> edgeIntersectionStorage;
    edgeIntersectionStorage.setVertexCount( nodes.size() );

    // Face 1
    nodes[0] = cvf::Vec3d( 0, 0, 0 );
    nodes[1] = cvf::Vec3d( 1, 0, 0 );
    nodes[2] = cvf::Vec3d( 1, 1, 0 );
    nodes[3] = cvf::Vec3d( 0, 1, 0 );
    // Face 2
    nodes[4] = cvf::Vec3d( 0, 0, 0 );
    nodes[5] = cvf::Vec3d( 1, 0, 0 );
    nodes[6] = cvf::Vec3d( 1, 1, 0 );
    nodes[7] = cvf::Vec3d( 0, 1, 0 );

    bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                  &additionalVertices,
                                                                  &edgeIntersectionStorage,
                                                                  wrapArrayConst( &nodes ),
                                                                  cv1CubeFaceIndices,
                                                                  cv2CubeFaceIndices,
                                                                  1e-6 );
    EXPECT_EQ( (size_t)4, polygon.size() );
    EXPECT_EQ( (size_t)0, additionalVertices.size() );
    EXPECT_TRUE( isOk );

    std::vector<bool>                 faceOverlapPolygonWinding;
    std::vector<std::vector<size_t>*> faceOverlapPolygons;
    faceOverlapPolygons.push_back( &polygon );
    faceOverlapPolygonWinding.push_back( true );

    std::vector<size_t> partialFacePolygon;
    bool                hasHoles = false;
    GeometryTools::calculatePartiallyFreeCubeFacePolygon( wrapArrayConst( &nodes ),
                                                          wrapArrayConst( cv1CubeFaceIndices, 4 ),
                                                          Vec3d( 0, 0, 1 ),
                                                          faceOverlapPolygons,
                                                          faceOverlapPolygonWinding,
                                                          &partialFacePolygon,
                                                          &hasHoles );

    // Face 1
    nodes[0] = cvf::Vec3d( 0, 0, 0 );
    nodes[1] = cvf::Vec3d( 1, 0, 0 );
    nodes[2] = cvf::Vec3d( 1, 1, 0 );
    nodes[3] = cvf::Vec3d( 0, 1, 0 );
    // Face 2
    nodes[4] = cvf::Vec3d( 0.5, -0.25, 0 );
    nodes[5] = cvf::Vec3d( 1.25, 0.5, 0 );
    nodes[6] = cvf::Vec3d( 0.5, 1.25, 0 );
    nodes[7] = cvf::Vec3d( -0.25, 0.5, 0 );
    polygon.clear();

    isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                             &additionalVertices,
                                                             &edgeIntersectionStorage,
                                                             wrapArrayConst( &nodes ),
                                                             cv1CubeFaceIndices,
                                                             cv2CubeFaceIndices,
                                                             1e-6 );
    EXPECT_EQ( (size_t)8, polygon.size() );
    EXPECT_EQ( (size_t)8, additionalVertices.size() );
    EXPECT_TRUE( isOk );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( CellFaceIntersectionTst, PolygonAreaNormal3D )
{
    // Test special cases with zero area

    {
        std::vector<cvf::Vec3d> vxs;

        cvf::Vec3d area = GeometryTools::polygonAreaNormal3D( vxs );
        EXPECT_TRUE( area == cvf::Vec3d::ZERO );
    }

    {
        std::vector<cvf::Vec3d> vxs;
        vxs.push_back( { 0, 0, 0 } );

        cvf::Vec3d area = GeometryTools::polygonAreaNormal3D( vxs );
        EXPECT_TRUE( area == cvf::Vec3d::ZERO );
    }

    {
        std::vector<cvf::Vec3d> vxs;
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 1 } );

        cvf::Vec3d area = GeometryTools::polygonAreaNormal3D( vxs );
        EXPECT_TRUE( area == cvf::Vec3d::ZERO );
    }

    // Three points

    {
        std::vector<cvf::Vec3d> vxs;
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 1 } );
        vxs.push_back( { 0, 1, 1 } );

        cvf::Vec3d area = GeometryTools::polygonAreaNormal3D( vxs );
        EXPECT_DOUBLE_EQ( -0.5, area.x() );
        EXPECT_DOUBLE_EQ( 0.0, area.y() );
        EXPECT_DOUBLE_EQ( 0.0, area.z() );
    }

    // four identical points

    {
        std::vector<cvf::Vec3d> vxs;
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 0 } );

        cvf::Vec3d area = GeometryTools::polygonAreaNormal3D( vxs );
        EXPECT_TRUE( area == cvf::Vec3d::ZERO );
    }

    // Square of four points

    {
        std::vector<cvf::Vec3d> vxs;
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 1 } );
        vxs.push_back( { 0, 1, 1 } );
        vxs.push_back( { 0, 1, 0 } );

        cvf::Vec3d area = GeometryTools::polygonAreaNormal3D( vxs );
        EXPECT_DOUBLE_EQ( -1.0, area.x() );
        EXPECT_DOUBLE_EQ( 0.0, area.y() );
        EXPECT_DOUBLE_EQ( 0.0, area.z() );
    }

    // Square of four points + one point in center of square

    {
        std::vector<cvf::Vec3d> vxs;
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 1 } );
        vxs.push_back( { 0, 1, 1 } );
        vxs.push_back( { 0, 1, 0 } );

        vxs.push_back( { 0, 0.5, 0.5 } ); // center of square

        cvf::Vec3d area = GeometryTools::polygonAreaNormal3D( vxs );
        EXPECT_DOUBLE_EQ( -0.75, area.x() );
        EXPECT_DOUBLE_EQ( 0.0, area.y() );
        EXPECT_DOUBLE_EQ( 0.0, area.z() );
    }

    // Area (float)

    {
        std::vector<cvf::Vec3f> vxs;
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 2 } );
        vxs.push_back( { 0, 2, 2 } );
        vxs.push_back( { 0, 2, 0 } );

        auto area = GeometryTools::polygonArea( vxs );
        EXPECT_FLOAT_EQ( 4.0, area );
    }

    // Area (double)

    {
        std::vector<cvf::Vec3d> vxs;
        vxs.push_back( { 0, 0, 0 } );
        vxs.push_back( { 0, 0, 2 } );
        vxs.push_back( { 0, 2, 2 } );
        vxs.push_back( { 0, 2, 0 } );

        auto area = GeometryTools::polygonArea( vxs );
        EXPECT_DOUBLE_EQ( 4.0, area );
    }
}

TEST( EarClipTesselator, ErrorTest )
{
    std::vector<cvf::Vec3d> remainingPolygon{
        cvf::Vec3d( 44.66, 20.17, 0 ),
        cvf::Vec3d( 78.08, 35.26, 0 ),
        cvf::Vec3d( 93.97, 35.83, 0 ),
        cvf::Vec3d( 144.95, 44.42, 0 ),
        cvf::Vec3d( 172.59, 39.73, 0 ),
        cvf::Vec3d( 227.27, 24.01, 0 ),
        cvf::Vec3d( 217.46, 45.72, 0 ),
        cvf::Vec3d( 178.5, 57.61, 0 ),
        cvf::Vec3d( 141.33, 63.82, 0 ),
        cvf::Vec3d( 0, 0, 0 ),
        cvf::Vec3d( 63.77, 0, 0 ),
    };

    double nativeTriangleArea = 21266;

    cvf::EarClipTesselator tess;
    tess.setNormal( cvf::Vec3d::Z_AXIS );
    tess.setMinTriangleArea( 1e-3 * nativeTriangleArea );
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

    // CVF_ASSERT( isTesselationOk );
    if ( !isTesselationOk )
    {
        // continue;
    }
}

//--------------------------------------------------------------------------------------------------
/// Test robust geometry calculation functions
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsErrorHandling, RobustCalculations )
{
    // Test with degenerate polygons
    std::vector<cvf::Vec3d> degeneratePolygon = { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 0, 0, 0 ) };

    cvf::Vec3d center = GeometryTools::computePolygonCenter( degeneratePolygon );
    EXPECT_TRUE( center.isZero() );

    double area = GeometryTools::polygonArea( degeneratePolygon );
    EXPECT_DOUBLE_EQ( 0.0, area );

    // Test with collinear points
    std::vector<cvf::Vec3d> collinearPolygon = { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 1, 0, 0 ), cvf::Vec3d( 2, 0, 0 ) };

    area = GeometryTools::polygonArea( collinearPolygon );
    EXPECT_DOUBLE_EQ( 0.0, area );
}

//--------------------------------------------------------------------------------------------------
/// Test simple error handling cases
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsErrorHandling, BasicErrorHandling )
{
    // Test empty polygon center calculation
    std::vector<cvf::Vec3d> emptyPolygon;
    cvf::Vec3d              center = GeometryTools::computePolygonCenter( emptyPolygon );
    EXPECT_TRUE( center.isZero() );

    // Test empty polygon area calculation
    double area = GeometryTools::polygonArea( emptyPolygon );
    EXPECT_DOUBLE_EQ( 0.0, area );

    // Test simple geometric functions with valid inputs
    std::vector<cvf::Vec3d> triangle = { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 1, 0, 0 ), cvf::Vec3d( 0, 1, 0 ) };

    center = GeometryTools::computePolygonCenter( triangle );
    EXPECT_NEAR( 1.0 / 3.0, center.x(), 1e-6 );
    EXPECT_NEAR( 1.0 / 3.0, center.y(), 1e-6 );

    area = GeometryTools::polygonArea( triangle );
    EXPECT_NEAR( 0.5, area, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Test bounds checking in EdgeIntersectStorage
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsErrorHandling, EdgeIntersectStorageBoundsCheck )
{
    EdgeIntersectStorage<cvf::uint> storage;
    storage.setVertexCount( 4 );

    // Test adding intersection with out-of-bounds index
    storage.addIntersection( 10, 11, 0, 1, 0, GeometryTools::LINES_CROSSES, 0.5, 0.5 );
    // Should return early without crashing

    cvf::uint                         intersectionIndex;
    GeometryTools::IntersectionStatus status;
    double                            fraction1, fraction2;

    // Test finding intersection with out-of-bounds index
    bool found = storage.findIntersection( 10, 11, 0, 1, &intersectionIndex, &status, &fraction1, &fraction2 );
    EXPECT_FALSE( found );
}

//--------------------------------------------------------------------------------------------------
/// Test computePolygonCenter template function
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsTemplate, ComputePolygonCenter )
{
    // Test with Vec3d
    std::vector<cvf::Vec3d> polygon = { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 2, 0, 0 ), cvf::Vec3d( 2, 2, 0 ), cvf::Vec3d( 0, 2, 0 ) };

    cvf::Vec3d center = GeometryTools::computePolygonCenter( polygon );
    EXPECT_DOUBLE_EQ( 1.0, center.x() );
    EXPECT_DOUBLE_EQ( 1.0, center.y() );
    EXPECT_DOUBLE_EQ( 0.0, center.z() );

    // Test with empty polygon
    std::vector<cvf::Vec3d> emptyPolygon;
    cvf::Vec3d              emptyCenter = GeometryTools::computePolygonCenter( emptyPolygon );
    EXPECT_TRUE( emptyCenter.isZero() );

    // Test with single point
    std::vector<cvf::Vec3d> singlePoint  = { cvf::Vec3d( 5, 3, 1 ) };
    cvf::Vec3d              singleCenter = GeometryTools::computePolygonCenter( singlePoint );
    EXPECT_DOUBLE_EQ( 5.0, singleCenter.x() );
    EXPECT_DOUBLE_EQ( 3.0, singleCenter.y() );
    EXPECT_DOUBLE_EQ( 1.0, singleCenter.z() );
}

//--------------------------------------------------------------------------------------------------
/// Test interpolateQuad template function
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsTemplate, InterpolateQuad )
{
    cvf::Vec3d v1( 0, 0, 0 );
    cvf::Vec3d v2( 1, 0, 0 );
    cvf::Vec3d v3( 1, 1, 0 );
    cvf::Vec3d v4( 0, 1, 0 );

    // Test interpolation at corners
    cvf::Vec3d center( 0.5, 0.5, 0 );
    double     result = GeometryTools::interpolateQuad( v1, 10.0, v2, 20.0, v3, 30.0, v4, 40.0, center );
    EXPECT_DOUBLE_EQ( 25.0, result ); // Average of corner values

    // Test interpolation at v1 position
    result = GeometryTools::interpolateQuad( v1, 10.0, v2, 20.0, v3, 30.0, v4, 40.0, v1 );
    EXPECT_DOUBLE_EQ( 10.0, result );

    // Test with float values
    float floatResult = GeometryTools::interpolateQuad( v1, 1.0f, v2, 2.0f, v3, 3.0f, v4, 4.0f, center );
    EXPECT_FLOAT_EQ( 2.5f, floatResult );
}

//--------------------------------------------------------------------------------------------------
/// Test geometric edge cases with simple functions
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsEdgeCases, GeometricEdgeCases )
{
    // Test polygon with repeated vertices
    std::vector<cvf::Vec3d> repeatedVertices = { cvf::Vec3d( 0, 0, 0 ),
                                                 cvf::Vec3d( 1, 0, 0 ),
                                                 cvf::Vec3d( 1, 0, 0 ), // Repeated
                                                 cvf::Vec3d( 1, 1, 0 ),
                                                 cvf::Vec3d( 0, 1, 0 ) };

    cvf::Vec3d center = GeometryTools::computePolygonCenter( repeatedVertices );
    EXPECT_GT( center.x(), 0.0 );
    EXPECT_GT( center.y(), 0.0 );

    // Test very small polygon
    std::vector<cvf::Vec3d> smallPolygon = { cvf::Vec3d( 0, 0, 0 ),
                                             cvf::Vec3d( 1e-10, 0, 0 ),
                                             cvf::Vec3d( 1e-10, 1e-10, 0 ),
                                             cvf::Vec3d( 0, 1e-10, 0 ) };

    double area = GeometryTools::polygonArea( smallPolygon );
    EXPECT_GT( area, 0.0 );
    EXPECT_LT( area, 1e-15 );
}

//--------------------------------------------------------------------------------------------------
/// Test additional geometric calculations
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsEdgeCases, AdditionalGeometricTests )
{
    // Test polygon normal calculation
    std::vector<cvf::Vec3d> square = { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 1, 0, 0 ), cvf::Vec3d( 1, 1, 0 ), cvf::Vec3d( 0, 1, 0 ) };

    cvf::Vec3d areaNormal = GeometryTools::polygonAreaNormal3D( square );
    EXPECT_NEAR( 0.0, areaNormal.x(), 1e-6 );
    EXPECT_NEAR( 0.0, areaNormal.y(), 1e-6 );
    EXPECT_NEAR( 1.0, areaNormal.z(), 1e-6 ); // Positive due to opposite winding

    // Test with different winding
    std::vector<cvf::Vec3d> squareReversed = { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 0, 1, 0 ), cvf::Vec3d( 1, 1, 0 ), cvf::Vec3d( 1, 0, 0 ) };

    areaNormal = GeometryTools::polygonAreaNormal3D( squareReversed );
    EXPECT_NEAR( 0.0, areaNormal.x(), 1e-6 );
    EXPECT_NEAR( 0.0, areaNormal.y(), 1e-6 );
    EXPECT_NEAR( -1.0, areaNormal.z(), 1e-6 ); // Negative due to winding
}

//--------------------------------------------------------------------------------------------------
/// Test numerical precision handling
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsEdgeCases, NumericalPrecision )
{
    // Test with very small differences
    std::vector<cvf::Vec3d> nearIdenticalPoints = { cvf::Vec3d( 0, 0, 0 ),
                                                    cvf::Vec3d( 1, 0, 0 ),
                                                    cvf::Vec3d( 1.0000001, 1e-10, 0 ), // Very close to (1, 0, 0)
                                                    cvf::Vec3d( 0, 1, 0 ) };

    double area = GeometryTools::polygonArea( nearIdenticalPoints );
    EXPECT_GT( area, 0.0 );
    EXPECT_LT( area, 1.0 ); // Should be less than unit square

    // Test with extreme coordinates
    std::vector<cvf::Vec3d> extremeCoords = { cvf::Vec3d( 1e6, 1e6, 0 ),
                                              cvf::Vec3d( 1e6 + 1, 1e6, 0 ),
                                              cvf::Vec3d( 1e6 + 1, 1e6 + 1, 0 ),
                                              cvf::Vec3d( 1e6, 1e6 + 1, 0 ) };

    area = GeometryTools::polygonArea( extremeCoords );
    EXPECT_NEAR( 1.0, area, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Test with various polygon sizes and configurations
//--------------------------------------------------------------------------------------------------
TEST( GeometryToolsEdgeCases, PolygonVariations )
{
    // Test with triangle
    std::vector<cvf::Vec3d> triangleNodes = { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 1, 0, 0 ), cvf::Vec3d( 0.5, 1, 0 ) };

    cvf::Vec3d triangleCenter = GeometryTools::computePolygonCenter( triangleNodes );
    EXPECT_NEAR( 0.5, triangleCenter.x(), 1e-6 );
    EXPECT_NEAR( 1.0 / 3.0, triangleCenter.y(), 1e-6 );

    // Test with hexagon
    std::vector<cvf::Vec3d> hexagonNodes;
    for ( int i = 0; i < 6; ++i )
    {
        double angle = i * 2.0 * M_PI / 6.0;
        hexagonNodes.push_back( cvf::Vec3d( cos( angle ), sin( angle ), 0 ) );
    }

    cvf::Vec3d hexagonCenter = GeometryTools::computePolygonCenter( hexagonNodes );
    EXPECT_NEAR( 0.0, hexagonCenter.x(), 1e-6 );
    EXPECT_NEAR( 0.0, hexagonCenter.y(), 1e-6 );

    double hexagonArea = GeometryTools::polygonArea( hexagonNodes );
    EXPECT_GT( hexagonArea, 0.0 );
}
