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

#include <iostream>
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

            // Same expression as in RigHexIntersectionTools::lineHexCellIntersection, so the well path point is
            // bit-identical to the triangle apex
            cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
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
/// The vertical ray of the point in cell test can go through a face centre, a triangle edge or a cell edge, where it hits
/// several triangles at the same point. Each of these is one crossing of the cell surface.
//--------------------------------------------------------------------------------------------------
TEST( RigHexIntersectionTools, isPointInCellRayThroughTriangleEdges )
{
    const std::array<cvf::Vec3d, 8> unitCube = { cvf::Vec3d( 0, 0, 0 ),
                                                 cvf::Vec3d( 1, 0, 0 ),
                                                 cvf::Vec3d( 1, 1, 0 ),
                                                 cvf::Vec3d( 0, 1, 0 ),
                                                 cvf::Vec3d( 0, 0, 1 ),
                                                 cvf::Vec3d( 1, 0, 1 ),
                                                 cvf::Vec3d( 1, 1, 1 ),
                                                 cvf::Vec3d( 0, 1, 1 ) };

    // Face centre, and the two diagonals the faces are split along
    EXPECT_TRUE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 0.5, 0.5, 0.5 ), unitCube ) );
    EXPECT_TRUE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 0.3, 0.3, 0.5 ), unitCube ) );
    EXPECT_TRUE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 0.7, 0.3, 0.5 ), unitCube ) );
    EXPECT_TRUE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 0.2, 0.6, 0.5 ), unitCube ) );

    EXPECT_FALSE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 0.5, 0.5, 1.5 ), unitCube ) );
    EXPECT_FALSE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 0.3, 0.3, 1.5 ), unitCube ) );
    EXPECT_FALSE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 0.5, 0.5, -0.5 ), unitCube ) );
    EXPECT_FALSE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 1.5, 0.5, 0.5 ), unitCube ) );

    // The top is moved 0.5 in X, so the ray from the point goes through the edge between the bottom face and the I+ face
    std::array<cvf::Vec3d, 8> shearedCube = unitCube;
    for ( size_t i = 4; i < 8; ++i )
    {
        shearedCube[i].x() += 0.5;
    }
    EXPECT_TRUE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 1.0, 0.5, 0.8 ), shearedCube ) );
    EXPECT_FALSE( RigHexIntersectionTools::isPointInCell( cvf::Vec3d( 1.0, 0.5, 1.2 ), shearedCube ) );

    // Distorted cells at UTM coordinates, with the ray through the bottom face centre and a triangle edge of the bottom face
    std::mt19937                           rng( 15153 );
    std::uniform_real_distribution<double> easting( 400000.0, 600000.0 );
    std::uniform_real_distribution<double> northing( 6000000.0, 7000000.0 );
    std::uniform_real_distribution<double> depth( -4000.0, -1000.0 );

    for ( int hexIdx = 0; hexIdx < 500; ++hexIdx )
    {
        std::array<cvf::Vec3d, 8> hexCorners = createDistortedHex( rng, cvf::Vec3d( easting( rng ), northing( rng ), depth( rng ) ) );

        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices( cvf::StructGridInterface::NEG_K, faceVertexIndices );
        const cvf::Vec3d bottomCenter   = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                               hexCorners[faceVertexIndices[1]],
                                                                               hexCorners[faceVertexIndices[2]],
                                                                               hexCorners[faceVertexIndices[3]] );
        const cvf::Vec3d onTriangleEdge = 0.5 * ( bottomCenter + hexCorners[faceVertexIndices[0]] );

        for ( const auto& onBottomFace : { bottomCenter, onTriangleEdge } )
        {
            EXPECT_TRUE( RigHexIntersectionTools::isPointInCell( onBottomFace + cvf::Vec3d( 0, 0, 3.0 ), hexCorners ) );
            EXPECT_FALSE( RigHexIntersectionTools::isPointInCell( onBottomFace - cvf::Vec3d( 0, 0, 3.0 ), hexCorners ) );
        }
    }
}
