/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>

#include "RigHexIntersectionTools.h"
#include "Well/RigWellLogExtractionTools.h"

#include "cvfGeometryTools.h"
#include "cvfStructGrid.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, planeHexCellIntersectionTest )
{
    std::array<cvf::Vec3d, 8> hexCorners;
    hexCorners[0] = cvf::Vec3d( 0, 0, 0 );
    hexCorners[1] = cvf::Vec3d( 1, 0, 0 );
    hexCorners[2] = cvf::Vec3d( 0, 1, 0 );
    hexCorners[3] = cvf::Vec3d( 0, 0, 1 );
    hexCorners[4] = cvf::Vec3d( 0, 1, 1 );
    hexCorners[5] = cvf::Vec3d( 1, 1, 0 );
    hexCorners[6] = cvf::Vec3d( 1, 0, 1 );
    hexCorners[7] = cvf::Vec3d( 1, 1, 1 );

    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;
    bool                                         isCellIntersected = false;
    cvf::Plane                                   fracturePlane;

    fracturePlane.setFromPointAndNormal( cvf::Vec3d( 0.5, 0.5, 0.5 ), cvf::Vec3d( 1, 0, 0 ) );
    isCellIntersected = RigHexIntersectionTools::planeHexCellIntersection( hexCorners, fracturePlane, intersectionLineSegments );
    EXPECT_TRUE( isCellIntersected );

    fracturePlane.setFromPointAndNormal( cvf::Vec3d( 1.5, 1.5, 1.5 ), cvf::Vec3d( 1, 0, 0 ) );
    isCellIntersected = RigHexIntersectionTools::planeHexCellIntersection( hexCorners, fracturePlane, intersectionLineSegments );
    EXPECT_FALSE( isCellIntersected );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, DISABLED_planeHexCellIntersectionPerformanceTest )
{
    std::array<cvf::Vec3d, 8> hexCorners;
    hexCorners[0] = cvf::Vec3d( 0, 0, 0 );
    hexCorners[1] = cvf::Vec3d( 1, 0, 0 );
    hexCorners[2] = cvf::Vec3d( 0, 1, 0 );
    hexCorners[3] = cvf::Vec3d( 0, 0, 1 );
    hexCorners[4] = cvf::Vec3d( 0, 1, 1 );
    hexCorners[5] = cvf::Vec3d( 1, 1, 0 );
    hexCorners[6] = cvf::Vec3d( 1, 0, 1 );
    hexCorners[7] = cvf::Vec3d( 1, 1, 1 );

    bool       isCellIntersected = false;
    cvf::Plane fracturePlaneIntersecting;
    cvf::Plane fracturePlaneNotIntersecting;

    fracturePlaneNotIntersecting.setFromPointAndNormal( cvf::Vec3d( 1.5, 1.5, 1.5 ), cvf::Vec3d( 1, 0, 0 ) );
    fracturePlaneIntersecting.setFromPointAndNormal( cvf::Vec3d( 0.5, 0.5, 0.5 ), cvf::Vec3d( 1, 0, 0 ) );

    QElapsedTimer timeTotal;
    timeTotal.start();

    for ( int run = 0; run < 5; run++ )
    {
        QElapsedTimer timeLocal;
        timeLocal.start();

        for ( int i = 0; i < 2000000; i++ )
        {
            std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;

            isCellIntersected =
                RigHexIntersectionTools::planeHexCellIntersection( hexCorners, fracturePlaneIntersecting, intersectionLineSegments );
            isCellIntersected =
                RigHexIntersectionTools::planeHexCellIntersection( hexCorners, fracturePlaneNotIntersecting, intersectionLineSegments );
        }

        qDebug() << "Time rim elapsed: " << timeLocal.elapsed();
    }

    qDebug() << "Time total elapsed: " << timeTotal.elapsed() << " intersected: " << static_cast<int>( isCellIntersected );
}

//--------------------------------------------------------------------------------------------------
/// Hex corners in ResInsight corner order, slightly distorted, at UTM-like coordinates
//--------------------------------------------------------------------------------------------------
static std::array<cvf::Vec3d, 8> createDistortedHex( std::mt19937& rng, const cvf::Vec3d& origin )
{
    const double dx = 100.0;
    const double dy = 100.0;
    const double dz = 10.0;

    std::array<cvf::Vec3d, 8> hexCorners = { origin + cvf::Vec3d( 0, 0, 0 ),
                                             origin + cvf::Vec3d( dx, 0, 0 ),
                                             origin + cvf::Vec3d( dx, dy, 0 ),
                                             origin + cvf::Vec3d( 0, dy, 0 ),
                                             origin + cvf::Vec3d( 0, 0, dz ),
                                             origin + cvf::Vec3d( dx, 0, dz ),
                                             origin + cvf::Vec3d( dx, dy, dz ),
                                             origin + cvf::Vec3d( 0, dy, dz ) };

    std::uniform_real_distribution<double> jitter( -5.0, 5.0 );
    for ( auto& corner : hexCorners )
    {
        corner += cvf::Vec3d( jitter( rng ), jitter( rng ), 0.1 * jitter( rng ) );
    }

    return hexCorners;
}

//--------------------------------------------------------------------------------------------------
/// Count the face centres where neither of the two well path segments sharing the point reports the crossing
//--------------------------------------------------------------------------------------------------
static int countMissedFaceCenterCrossings( double parametricTolerance, int hexCount )
{
    std::mt19937                           rng( 15088 );
    std::uniform_real_distribution<double> easting( 400000.0, 600000.0 );
    std::uniform_real_distribution<double> northing( 6000000.0, 7000000.0 );
    std::uniform_real_distribution<double> depth( -4000.0, -1000.0 );

    int missed = 0;

    for ( int hexIdx = 0; hexIdx < hexCount; ++hexIdx )
    {
        std::array<cvf::Vec3d, 8> hexCorners = createDistortedHex( rng, cvf::Vec3d( easting( rng ), northing( rng ), depth( rng ) ) );

        cvf::Vec3d cellCenter = cvf::Vec3d::ZERO;
        for ( const auto& corner : hexCorners )
            cellCenter += corner;
        cellCenter *= 1.0 / 8.0;

        for ( int face = 0; face < 6; ++face )
        {
            cvf::ubyte faceVertexIndices[4];
            cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ), faceVertexIndices );

            // Same face center as in RigHexIntersectionTools::lineHexCellIntersection, so the well path point is
            // bit-identical to the triangle apex
            cvf::Vec3d faceCenter = RigHexIntersectionTools::canonicalFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                                  hexCorners[faceVertexIndices[1]],
                                                                                  hexCorners[faceVertexIndices[2]],
                                                                                  hexCorners[faceVertexIndices[3]] );

            cvf::Vec3d outsidePoint = faceCenter + ( faceCenter - cellCenter );

            std::vector<HexIntersectionInfo> intersections;
            RigHexIntersectionTools::lineHexCellIntersection( outsidePoint, faceCenter, hexCorners, 0, &intersections, parametricTolerance );
            RigHexIntersectionTools::lineHexCellIntersection( faceCenter, cellCenter, hexCorners, 0, &intersections, parametricTolerance );

            bool found = false;
            for ( const auto& intersection : intersections )
            {
                if ( intersection.m_face == static_cast<cvf::StructGridInterface::FaceType>( face ) &&
                     intersection.m_intersectionPoint.pointDistance( faceCenter ) < 1e-6 )
                    found = true;
            }

            if ( !found ) ++missed;
        }
    }

    return missed;
}

//--------------------------------------------------------------------------------------------------
/// #15088: a well path vertex on a cell face centre must be reported by one of the two segments sharing it
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, lineHexCellIntersectionAtFaceCenters )
{
    const int hexCount = 500;

    int missedExact = countMissedFaceCenterCrossings( 0.0, hexCount );
    std::cout << "Face centre crossings missed with parametric tolerance 0: " << missedExact << " of " << 6 * hexCount << std::endl;

    int missedWithTolerance = countMissedFaceCenterCrossings( RigWellLogExtractionTools::intersectionParametricTolerance, hexCount );
    EXPECT_EQ( 0, missedWithTolerance );
}

//--------------------------------------------------------------------------------------------------
/// The parametric tolerance does not change the result for segments clearly inside or outside the cell, and a
/// segment ending just short of a face is only accepted with a tolerance, with the point kept on the segment
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, lineHexCellIntersectionParametricTolerance )
{
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( 0, 0, 0 ),
                                             cvf::Vec3d( 1, 0, 0 ),
                                             cvf::Vec3d( 1, 1, 0 ),
                                             cvf::Vec3d( 0, 1, 0 ),
                                             cvf::Vec3d( 0, 0, 1 ),
                                             cvf::Vec3d( 1, 0, 1 ),
                                             cvf::Vec3d( 1, 1, 1 ),
                                             cvf::Vec3d( 0, 1, 1 ) };

    const double tolerance = RigWellLogExtractionTools::intersectionParametricTolerance;

    {
        std::vector<HexIntersectionInfo> exact;
        std::vector<HexIntersectionInfo> withTolerance;
        RigHexIntersectionTools::lineHexCellIntersection( cvf::Vec3d( 0.5, 0.5, -1 ), cvf::Vec3d( 0.5, 0.5, 2 ), hexCorners, 0, &exact );
        RigHexIntersectionTools::lineHexCellIntersection( cvf::Vec3d( 0.5, 0.5, -1 ),
                                                          cvf::Vec3d( 0.5, 0.5, 2 ),
                                                          hexCorners,
                                                          0,
                                                          &withTolerance,
                                                          tolerance );

        ASSERT_EQ( 2u, exact.size() );
        ASSERT_EQ( 2u, withTolerance.size() );
        for ( size_t i = 0; i < exact.size(); ++i )
        {
            EXPECT_EQ( exact[i].m_face, withTolerance[i].m_face );
            EXPECT_EQ( exact[i].m_isIntersectionEntering, withTolerance[i].m_isIntersectionEntering );
            EXPECT_EQ( exact[i].m_intersectionPoint, withTolerance[i].m_intersectionPoint );
        }
    }

    {
        std::vector<HexIntersectionInfo> exact;
        std::vector<HexIntersectionInfo> withTolerance;
        RigHexIntersectionTools::lineHexCellIntersection( cvf::Vec3d( 0.5, 0.5, -1 ), cvf::Vec3d( 0.5, 0.5, -0.1 ), hexCorners, 0, &exact );
        RigHexIntersectionTools::lineHexCellIntersection( cvf::Vec3d( 0.5, 0.5, -1 ),
                                                          cvf::Vec3d( 0.5, 0.5, -0.1 ),
                                                          hexCorners,
                                                          0,
                                                          &withTolerance,
                                                          tolerance );

        EXPECT_EQ( 0u, exact.size() );
        EXPECT_EQ( 0u, withTolerance.size() );
    }

    {
        const cvf::Vec3d p1( 0.5, 0.5, -1 );
        const cvf::Vec3d p2( 0.5, 0.5, -1e-12 );

        std::vector<HexIntersectionInfo> exact;
        std::vector<HexIntersectionInfo> withTolerance;
        RigHexIntersectionTools::lineHexCellIntersection( p1, p2, hexCorners, 0, &exact );
        RigHexIntersectionTools::lineHexCellIntersection( p1, p2, hexCorners, 0, &withTolerance, tolerance );

        EXPECT_EQ( 0u, exact.size() );
        ASSERT_EQ( 1u, withTolerance.size() );
        EXPECT_LT( withTolerance[0].m_intersectionPoint.pointDistance( p2 ), 1e-12 );
    }
}

//--------------------------------------------------------------------------------------------------
/// The face center does not depend on where the cyclic corner order starts or its direction, so the shared face of
/// two neighbour cells gets the same triangulation in both cells
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, canonicalFaceTriangulationIsSharedByNeighbourCells )
{
    std::mt19937                           rng( 15163 );
    std::uniform_real_distribution<double> easting( 400000.0, 600000.0 );
    std::uniform_real_distribution<double> northing( 6000000.0, 7000000.0 );
    std::uniform_real_distribution<double> depth( -4000.0, -1000.0 );

    for ( int cellIdx = 0; cellIdx < 500; ++cellIdx )
    {
        const std::array<cvf::Vec3d, 8> cellA = createDistortedHex( rng, cvf::Vec3d( easting( rng ), northing( rng ), depth( rng ) ) );

        // Neighbour in I direction sharing the POS_I face of cell A as its NEG_I face
        const cvf::Vec3d          offset( 100.0, 0.0, 0.0 );
        std::array<cvf::Vec3d, 8> cellB =
            { cellA[1], cellA[1] + offset, cellA[2] + offset, cellA[2], cellA[5], cellA[5] + offset, cellA[6] + offset, cellA[6] };

        using Triangle            = std::array<cvf::Vec3d, 3>;
        auto collectFaceTriangles = []( const std::array<cvf::Vec3d, 8>& hexCorners, cvf::StructGridInterface::FaceType face )
        {
            std::vector<Triangle> triangles;
            RigHexIntersectionTools::forEachCellSurfaceTriangle( hexCorners,
                                                                 [&]( cvf::StructGridInterface::FaceType triangleFace,
                                                                      const cvf::Vec3d&                  t0,
                                                                      const cvf::Vec3d&                  t1,
                                                                      const cvf::Vec3d&                  t2 )
                                                                 {
                                                                     if ( triangleFace == face ) triangles.push_back( { t0, t1, t2 } );
                                                                     return true;
                                                                 } );
            return triangles;
        };

        const auto trianglesA = collectFaceTriangles( cellA, cvf::StructGridInterface::POS_I );
        const auto trianglesB = collectFaceTriangles( cellB, cvf::StructGridInterface::NEG_I );

        ASSERT_EQ( 4u, trianglesA.size() );
        ASSERT_EQ( 4u, trianglesB.size() );
        ASSERT_EQ( trianglesA[0][2], trianglesB[0][2] );

        // Every triangle of the shared face in cell A has a triangle with the same vertices in cell B
        for ( const auto& triA : trianglesA )
        {
            bool found = false;
            for ( const auto& triB : trianglesB )
            {
                const bool sameEdge = ( triA[0] == triB[0] && triA[1] == triB[1] ) || ( triA[0] == triB[1] && triA[1] == triB[0] );
                if ( sameEdge && triA[2] == triB[2] ) found = true;
            }
            EXPECT_TRUE( found );
        }

        // All rotations and reversals of the cyclic corner order give the same center
        const cvf::Vec3d reference = RigHexIntersectionTools::canonicalFaceCenter( cellA[0], cellA[1], cellA[2], cellA[3] );
        for ( int start = 0; start < 4; ++start )
        {
            for ( int step : { 1, 3 } )
            {
                auto corner = [&]( int i ) { return cellA[( start + i * step ) % 4]; };
                EXPECT_EQ( reference, RigHexIntersectionTools::canonicalFaceCenter( corner( 0 ), corner( 1 ), corner( 2 ), corner( 3 ) ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// A segment through cell corners hits several faces and triangles at the same point. These hits are merged into
/// one entering and one leaving intersection, sorted along the segment.
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, lineHexCellIntersectionMergesHitsAtCorners )
{
    const cvf::Vec3d origin( 512345.678, 6712345.678, -2345.678 );

    std::array<cvf::Vec3d, 8> hexCorners = { origin + cvf::Vec3d( 0, 0, 0 ),
                                             origin + cvf::Vec3d( 10, 0, 0 ),
                                             origin + cvf::Vec3d( 10, 10, 0 ),
                                             origin + cvf::Vec3d( 0, 10, 0 ),
                                             origin + cvf::Vec3d( 0, 0, 10 ),
                                             origin + cvf::Vec3d( 10, 0, 10 ),
                                             origin + cvf::Vec3d( 10, 10, 10 ),
                                             origin + cvf::Vec3d( 0, 10, 10 ) };

    const cvf::Vec3d p1 = origin + cvf::Vec3d( -10, -10, -10 );
    const cvf::Vec3d p2 = origin + cvf::Vec3d( 20, 20, 20 );

    {
        std::vector<HexIntersectionInfo> intersections;
        int                              count = RigHexIntersectionTools::lineHexCellIntersection( p1, p2, hexCorners, 7, &intersections );

        ASSERT_EQ( 2, count );
        ASSERT_EQ( 2u, intersections.size() );
        EXPECT_TRUE( intersections[0].m_isIntersectionEntering );
        EXPECT_FALSE( intersections[1].m_isIntersectionEntering );
        EXPECT_LT( intersections[0].m_intersectionPoint.pointDistance( hexCorners[0] ), 1e-6 );
        EXPECT_LT( intersections[1].m_intersectionPoint.pointDistance( hexCorners[6] ), 1e-6 );
        EXPECT_EQ( 7u, intersections[0].m_hexIndex );
    }

    {
        // Reversed direction gives the same hits in reversed order with the opposite direction
        std::vector<HexIntersectionInfo> forward;
        std::vector<HexIntersectionInfo> reverse;
        RigHexIntersectionTools::lineHexCellIntersection( p1, p2, hexCorners, 0, &forward );
        RigHexIntersectionTools::lineHexCellIntersection( p2, p1, hexCorners, 0, &reverse );

        ASSERT_EQ( forward.size(), reverse.size() );
        for ( size_t i = 0; i < forward.size(); ++i )
        {
            const auto& f = forward[i];
            const auto& r = reverse[reverse.size() - 1 - i];
            EXPECT_NE( f.m_isIntersectionEntering, r.m_isIntersectionEntering );
            EXPECT_LT( f.m_intersectionPoint.pointDistance( r.m_intersectionPoint ), 1e-6 );
        }
    }

    {
        // A segment through a cell edge hits two faces at the same point
        std::vector<HexIntersectionInfo> intersections;
        RigHexIntersectionTools::lineHexCellIntersection( origin + cvf::Vec3d( -5, 5, -5 ),
                                                          origin + cvf::Vec3d( 15, 5, 15 ),
                                                          hexCorners,
                                                          0,
                                                          &intersections );
        ASSERT_EQ( 2u, intersections.size() );
        EXPECT_TRUE( intersections[0].m_isIntersectionEntering );
        EXPECT_FALSE( intersections[1].m_isIntersectionEntering );
    }
}

//--------------------------------------------------------------------------------------------------
/// Duplicates in the same direction are merged, but distinct crossings with an intervening crossing in the opposite
/// direction are kept even when they are closer than the merge tolerance
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, sortMergeAndAppendHits )
{
    using Hit                   = RigHexIntersectionTools::SegmentHit;
    const double tolerance      = RigHexIntersectionTools::intersectionMergeTolerance;
    const auto   face           = cvf::StructGridInterface::NEG_K;
    auto         pointAt        = []( double t ) { return cvf::Vec3d( t * 1.0e6, 0, 0 ); };
    auto         hitAt          = [&]( double t, bool isEntering ) { return Hit{ t, pointAt( t ), isEntering, face }; };
    auto         enteringValues = []( const std::vector<HexIntersectionInfo>& intersections )
    {
        std::vector<bool> values;
        for ( const auto& intersection : intersections )
            values.push_back( intersection.m_isIntersectionEntering );
        return values;
    };

    {
        // Duplicates from neighbour triangles, unsorted
        std::vector<Hit> hits = { hitAt( 0.7 + 0.5 * tolerance, false ), hitAt( 0.3 + 1.0e-12, true ), hitAt( 0.7, false ), hitAt( 0.3, true ) };
        std::vector<HexIntersectionInfo> intersections;

        EXPECT_EQ( 2, RigHexIntersectionTools::sortMergeAndAppendHits( hits, tolerance, 3, &intersections ) );
        ASSERT_EQ( 2u, intersections.size() );
        EXPECT_EQ( std::vector<bool>( { true, false } ), enteringValues( intersections ) );
        EXPECT_EQ( pointAt( 0.3 ), intersections[0].m_intersectionPoint );
        EXPECT_EQ( pointAt( 0.7 ), intersections[1].m_intersectionPoint );
        EXPECT_EQ( 3u, intersections[0].m_hexIndex );
    }

    {
        // Distinct crossings within the tolerance, alternating direction
        const double step = 0.2 * tolerance;
        std::vector<Hit> hits = { hitAt( 0.5 + 3 * step, false ), hitAt( 0.5 + step, false ), hitAt( 0.5, true ), hitAt( 0.5 + 2 * step, true ) };
        std::vector<HexIntersectionInfo> intersections;

        EXPECT_EQ( 4, RigHexIntersectionTools::sortMergeAndAppendHits( hits, tolerance, 0, &intersections ) );
        EXPECT_EQ( std::vector<bool>( { true, false, true, false } ), enteringValues( intersections ) );
    }

    {
        // A run is measured from its first hit, so a chain of close hits is not merged into one
        std::vector<Hit> hits = { hitAt( 0.1, true ), hitAt( 0.1 + 0.6 * tolerance, true ), hitAt( 0.1 + 1.2 * tolerance, true ) };
        std::vector<HexIntersectionInfo> intersections;

        EXPECT_EQ( 2, RigHexIntersectionTools::sortMergeAndAppendHits( hits, tolerance, 0, &intersections ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Timing of the line and point vs cell queries on distorted cells at UTM coordinates. Disabled by default, run with
/// --gtest_also_run_disabled_tests --gtest_filter=*IntersectionPerformance*
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, DISABLED_lineAndPointCellIntersectionPerformance )
{
    const int    hexCount      = 2000;
    const int    queriesPerHex = 50;
    const int    runCount      = 5;
    const double cellExtension = 20.0;

    std::mt19937                           rng( 15163 );
    std::uniform_real_distribution<double> easting( 400000.0, 600000.0 );
    std::uniform_real_distribution<double> northing( 6000000.0, 7000000.0 );
    std::uniform_real_distribution<double> depth( -4000.0, -1000.0 );
    std::uniform_real_distribution<double> unit( 0.0, 1.0 );

    std::vector<std::array<cvf::Vec3d, 8>>         hexes;
    std::vector<std::pair<cvf::Vec3d, cvf::Vec3d>> segments;
    for ( int hexIdx = 0; hexIdx < hexCount; ++hexIdx )
    {
        hexes.push_back( createDistortedHex( rng, cvf::Vec3d( easting( rng ), northing( rng ), depth( rng ) ) ) );

        cvf::Vec3d minCorner = hexes.back()[0];
        cvf::Vec3d maxCorner = hexes.back()[0];
        for ( const auto& corner : hexes.back() )
        {
            for ( int i = 0; i < 3; ++i )
            {
                minCorner[i] = std::min( minCorner[i], corner[i] );
                maxCorner[i] = std::max( maxCorner[i], corner[i] );
            }
        }
        minCorner -= cvf::Vec3d( cellExtension, cellExtension, 0.2 * cellExtension );
        maxCorner += cvf::Vec3d( cellExtension, cellExtension, 0.2 * cellExtension );

        auto randomPoint = [&]()
        {
            return cvf::Vec3d( minCorner.x() + unit( rng ) * ( maxCorner.x() - minCorner.x() ),
                               minCorner.y() + unit( rng ) * ( maxCorner.y() - minCorner.y() ),
                               minCorner.z() + unit( rng ) * ( maxCorner.z() - minCorner.z() ) );
        };

        for ( int q = 0; q < queriesPerHex; ++q )
        {
            segments.emplace_back( randomPoint(), randomPoint() );
        }
    }

    auto bestOf = [&]( auto&& function )
    {
        double best = std::numeric_limits<double>::infinity();
        for ( int run = 0; run < runCount; ++run )
        {
            auto start = std::chrono::steady_clock::now();
            function();
            auto stop = std::chrono::steady_clock::now();
            best      = std::min( best, std::chrono::duration<double, std::milli>( stop - start ).count() );
        }
        return best;
    };

    const size_t queryCount = segments.size();

    size_t lineHexHits = 0;
    double lineHexMs   = bestOf(
        [&]()
        {
            lineHexHits = 0;
            std::vector<HexIntersectionInfo> intersections;
            for ( size_t q = 0; q < queryCount; ++q )
            {
                intersections.clear();
                lineHexHits += RigHexIntersectionTools::lineHexCellIntersection( segments[q].first,
                                                                                 segments[q].second,
                                                                                 hexes[q / queriesPerHex],
                                                                                 q,
                                                                                 &intersections,
                                                                                 RigWellLogExtractionTools::intersectionParametricTolerance );
            }
        } );

    size_t lineIntersectsCount = 0;
    double lineIntersectsMs    = bestOf(
        [&]()
        {
            lineIntersectsCount = 0;
            for ( size_t q = 0; q < queryCount; ++q )
            {
                if ( RigHexIntersectionTools::lineIntersectsHexCell( segments[q].first, segments[q].second, hexes[q / queriesPerHex] ) )
                    ++lineIntersectsCount;
            }
        } );

    size_t pointInCellCount = 0;
    double pointInCellMs    = bestOf(
        [&]()
        {
            pointInCellCount = 0;
            for ( size_t q = 0; q < queryCount; ++q )
            {
                if ( RigHexIntersectionTools::isPointInCell( segments[q].first, hexes[q / queriesPerHex] ) ) ++pointInCellCount;
            }
        } );

    auto nsPerQuery = [&]( double ms ) { return 1.0e6 * ms / static_cast<double>( queryCount ); };

    std::cout << "Queries: " << queryCount << ", best of " << runCount << " runs" << std::endl;
    std::cout << "lineHexCellIntersection: " << lineHexMs << " ms (" << nsPerQuery( lineHexMs ) << " ns/query), hits " << lineHexHits
              << std::endl;
    std::cout << "lineIntersectsHexCell:   " << lineIntersectsMs << " ms (" << nsPerQuery( lineIntersectsMs ) << " ns/query), intersecting "
              << lineIntersectsCount << std::endl;
    std::cout << "isPointInCell:           " << pointInCellMs << " ms (" << nsPerQuery( pointInCellMs ) << " ns/query), inside "
              << pointInCellCount << std::endl;
}
