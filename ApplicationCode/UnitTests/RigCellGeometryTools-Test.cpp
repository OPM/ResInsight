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

#include "RigCellGeometryTools.h"
#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, calculateCellVolumeTest )
{
    cvf::BoundingBox bbox( cvf::Vec3d( 1.0, -2.0, 5.0 ), cvf::Vec3d( 500.0, 3.0, 1500.0 ) );
    cvf::Vec3d       extent     = bbox.extent();
    double           bboxVolume = extent.x() * extent.y() * extent.z();

    std::array<cvf::Vec3d, 8> cornerVertices;
    bbox.cornerVertices( cornerVertices.data() );

    // This is a cuboid. The result should be exact
    EXPECT_DOUBLE_EQ( bboxVolume, RigCellGeometryTools::calculateCellVolume( cornerVertices ) );

    // Distort it by adding a tetrahedron to the volume
    cornerVertices[1].x() += bbox.extent().x() / 3.0;
    cornerVertices[2].x() += bbox.extent().x() / 3.0;

    double extraVolume = 0.5 * extent.z() * bbox.extent().x() / 3.0 * extent.y();

    EXPECT_DOUBLE_EQ( bboxVolume + extraVolume, RigCellGeometryTools::calculateCellVolume( cornerVertices ) );

    // The overlap with the original bounding box should just yield the original bounding box
    cvf::BoundingBox          overlapBoundingBox;
    std::array<cvf::Vec3d, 8> overlapVertices;
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices, bbox, &overlapVertices, &overlapBoundingBox );

    EXPECT_DOUBLE_EQ( bboxVolume, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );

    cvf::Vec3d overlapExtent     = overlapBoundingBox.extent();
    double     overlapBBoxVolume = overlapExtent.x() * overlapExtent.y() * overlapExtent.z();
    EXPECT_DOUBLE_EQ( bboxVolume, overlapBBoxVolume );

    // Shift bounding box by half the original extent in x-direction.
    // It should now contain the full tetrahedron + half the original bounding box
    std::array<cvf::Vec3d, 8> tetrahedronBoxVertices;
    bbox.cornerVertices( tetrahedronBoxVertices.data() );
    cvf::BoundingBox tetrahedronBBox;
    for ( cvf::Vec3d& corner : tetrahedronBoxVertices )
    {
        corner.x() += 0.5 * bbox.extent().x();
        tetrahedronBBox.add( corner );
    }
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices,
                                                             tetrahedronBBox,
                                                             &overlapVertices,
                                                             &overlapBoundingBox );

    EXPECT_DOUBLE_EQ( bboxVolume * 0.5 + extraVolume, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );

    // Shift it the rest of the original extent in x-direction.
    // The bounding box should now contain only the tetrahedron.
    tetrahedronBBox = cvf::BoundingBox();
    for ( cvf::Vec3d& corner : tetrahedronBoxVertices )
    {
        corner.x() += 0.5 * bbox.extent().x();
        tetrahedronBBox.add( corner );
    }
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices,
                                                             tetrahedronBBox,
                                                             &overlapVertices,
                                                             &overlapBoundingBox );

    EXPECT_DOUBLE_EQ( extraVolume, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );

    // Expand original bounding box to be much larger than the hex
    bbox.expand( 2000 );
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices, bbox, &overlapVertices, &overlapBoundingBox );
    EXPECT_DOUBLE_EQ( bboxVolume + extraVolume, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, calculateCellVolumeTest2 )
{
    cvf::BoundingBox          bbox( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( 100.0, 100.0, 100.0 ) );
    std::array<cvf::Vec3d, 8> cornerVertices;
    bbox.cornerVertices( cornerVertices.data() );

    cornerVertices[5].z() = cornerVertices[1].z();
    cornerVertices[6].z() = cornerVertices[2].z();

    double totalCellVolume = 0.5 * 100.0 * 100.0 * 100.0;
    EXPECT_DOUBLE_EQ( totalCellVolume, RigCellGeometryTools::calculateCellVolume( cornerVertices ) );
    cvf::BoundingBox innerBBox( cvf::Vec3d( 25.0, 25.0, -10.0 ), cvf::Vec3d( 75.0, 75.0, 110.0 ) );

    double expectedOverlap = 50 * 50 * 25 + 0.5 * 50 * 50 * 50;

    cvf::BoundingBox          overlapBoundingBox;
    std::array<cvf::Vec3d, 8> overlapVertices;
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices, innerBBox, &overlapVertices, &overlapBoundingBox );
    EXPECT_DOUBLE_EQ( expectedOverlap, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );

    cvf::BoundingBox smallerInnerBBox( cvf::Vec3d( 25.0, 25.0, -10.0 ), cvf::Vec3d( 75.0, 75.0, 25.0 ) );
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices,
                                                             smallerInnerBBox,
                                                             &overlapVertices,
                                                             &overlapBoundingBox );
    EXPECT_DOUBLE_EQ( 50 * 50 * 25, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );

    cvf::BoundingBox smallerBBox( cvf::Vec3d( 50.0, 50.0, 0.0 ), cvf::Vec3d( 100.0, 100.0, 100.0 ) );
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices, smallerBBox, &overlapVertices, &overlapBoundingBox );
    double tipVolume = 50 * 50 * 50 * 0.5;
    EXPECT_DOUBLE_EQ( tipVolume, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );

    cvf::BoundingBox smallerBBox2( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( 50.0, 50.0, 100.0 ) );
    RigCellGeometryTools::estimateHexOverlapWithBoundingBox( cornerVertices,
                                                             smallerBBox2,
                                                             &overlapVertices,
                                                             &overlapBoundingBox );
    double expectedVolume = ( totalCellVolume - 2 * tipVolume ) * 0.5;
    EXPECT_DOUBLE_EQ( expectedVolume, RigCellGeometryTools::calculateCellVolume( overlapVertices ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, createPolygonTest )
{
    cvf::Vec3d a  = cvf::Vec3d( 1, 1, 1 );
    cvf::Vec3d b  = cvf::Vec3d( 1, 3.14159265359, 1 );
    cvf::Vec3d b2 = cvf::Vec3d( 1, 3.1415926536, 1 );
    cvf::Vec3d c  = cvf::Vec3d( 5, 5, 1 );
    cvf::Vec3d d  = cvf::Vec3d( -2, 8, 1 );

    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;
    intersectionLineSegments.push_back( {a, b} );
    intersectionLineSegments.push_back( {b2, c} );
    intersectionLineSegments.push_back( {c, d} );
    intersectionLineSegments.push_back( {a, d} );

    std::vector<std::vector<cvf::Vec3d>> polygons;

    RigCellGeometryTools::createPolygonFromLineSegments( intersectionLineSegments, polygons );

    EXPECT_EQ( polygons.size(), (size_t)1 );
    EXPECT_EQ( polygons[0].size(), (size_t)5 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, createMultiplePolygonTest )
{
    cvf::Vec3d a1 = cvf::Vec3d( 5, 4, 1 );
    cvf::Vec3d b1 = cvf::Vec3d( 6, 3, 1 );
    cvf::Vec3d c1 = cvf::Vec3d( 6, 4, 1 );

    cvf::Vec3d a2 = cvf::Vec3d( 2, 1, 1 );
    cvf::Vec3d b2 = cvf::Vec3d( 1, 3, 1 );
    cvf::Vec3d c2 = cvf::Vec3d( 1, 5, 1 );

    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;
    intersectionLineSegments.push_back( {a1, b1} );
    intersectionLineSegments.push_back( {b1, c1} );

    intersectionLineSegments.push_back( {a2, b2} );
    intersectionLineSegments.push_back( {b2, c2} );

    std::vector<std::vector<cvf::Vec3d>> polygons;

    RigCellGeometryTools::createPolygonFromLineSegments( intersectionLineSegments, polygons );

    EXPECT_EQ( polygons.size(), (size_t)2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, createPolygonTestRealCase )
{
    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;

    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900632511830, 20.000000000000000, -0.011799300447865143 ),
                                         cvf::Vec3d( 13.498900632515129, 20.000000000000000, -0.011874744050458887 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900633056895, 20.000000000000000, -0.024268930302180504 ),
                                         cvf::Vec3d( 13.498900632515127, 20.000000000000000, -0.011874744050458887 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900632511830, 20.000000000000000, -0.011799300447865143 ),
                                         cvf::Vec3d( 13.498900631970063, 20.000000000000000, 0.00059488709383226715 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900195597713, 10.000000000000000, -0.016369358494277231 ),
                                         cvf::Vec3d( 13.498900195056242, 10.000000000000000, -0.0039819325234285293 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900195597711, 10.000000000000000, -0.016369358494277241 ),
                                         cvf::Vec3d( 13.498900195600806, 10.000000000000000, -0.016440172032184591 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900196142280, 10.000000000000000, -0.028827609381707260 ),
                                         cvf::Vec3d( 13.498900195600807, 10.000000000000000, -0.016440172032184591 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900196142280, 10.000000000000000, -0.028827609381707260 ),
                                         cvf::Vec3d( 13.498900414562046, 14.999139949621291, -0.026549475935230070 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900414637365, 15.000863944878075, -0.026548599914369600 ),
                                         cvf::Vec3d( 13.498900633056895, 20.000000000000000, -0.024268930302180504 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900414637365, 15.000863944878075, -0.026548599914369600 ),
                                         cvf::Vec3d( 13.498900414562046, 14.999139949621291, -0.026549475935230073 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900413511738, 14.999966890459870, -0.0016942968682188385 ),
                                         cvf::Vec3d( 13.498900413514638, 15.000033259475705, -0.0016942630763332898 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900631970063, 20.000000000000000, 0.00059488709383226715 ),
                                         cvf::Vec3d( 13.498900413514637, 15.000033259475705, -0.0016942630763332901 )} );
    intersectionLineSegments.push_back( {cvf::Vec3d( 13.498900413511738, 14.999966890459870, -0.0016942968682188385 ),
                                         cvf::Vec3d( 13.498900195056240, 10.000000000000000, -0.0039819325234285319 )} );

    std::vector<std::vector<cvf::Vec3d>> polygons;

    RigCellGeometryTools::createPolygonFromLineSegments( intersectionLineSegments, polygons );

    EXPECT_EQ( polygons.size(), (size_t)1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, findCellAverageZTest )
{
    std::array<cvf::Vec3d, 8> hexCorners;

    hexCorners[0] = cvf::Vec3d( 0, 0, 0 );
    hexCorners[1] = cvf::Vec3d( 1, 0, 0 );
    hexCorners[2] = cvf::Vec3d( 1, 1, 0 );
    hexCorners[3] = cvf::Vec3d( 0, 1, 0 );

    hexCorners[4] = cvf::Vec3d( 0, 0, 1 );
    hexCorners[5] = cvf::Vec3d( 1, 0, 1 );
    hexCorners[6] = cvf::Vec3d( 1, 1, 1 );
    hexCorners[7] = cvf::Vec3d( 0, 1, 1 );

    cvf::Vec3d localX;
    cvf::Vec3d localY;
    cvf::Vec3d localZ;

    RigCellGeometryTools::findCellLocalXYZ( hexCorners, localX, localY, localZ );

    EXPECT_DOUBLE_EQ( localX[0], 1 );
    EXPECT_DOUBLE_EQ( localX[1], 0 );
    EXPECT_DOUBLE_EQ( localX[2], 0 );

    EXPECT_DOUBLE_EQ( localY[0], 0 );
    EXPECT_DOUBLE_EQ( localY[1], 1 );
    EXPECT_DOUBLE_EQ( localY[2], 0 );

    EXPECT_DOUBLE_EQ( localZ[0], 0 );
    EXPECT_DOUBLE_EQ( localZ[1], 0 );
    EXPECT_DOUBLE_EQ( localZ[2], 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, lengthCalcTest )
{
    std::vector<cvf::Vec3d> polygonExample;
    polygonExample.push_back( cvf::Vec3d( 0.00, 0.00, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 0.00, 2.50, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 1.50, 2.50, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 1.50, 0.00, 0.0 ) );

    double length = RigCellGeometryTools::polygonLengthInLocalXdirWeightedByArea( polygonExample );
    EXPECT_DOUBLE_EQ( length, 1.5 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, lengthCalcTestTriangle )
{
    std::vector<cvf::Vec3d> trianglePolygonExample;
    trianglePolygonExample.push_back( cvf::Vec3d( 0.00, 0.00, 0.0 ) );
    trianglePolygonExample.push_back( cvf::Vec3d( 2.50, 2.50, 0.0 ) );
    trianglePolygonExample.push_back( cvf::Vec3d( 2.50, 0.00, 0.0 ) );

    double length = RigCellGeometryTools::polygonLengthInLocalXdirWeightedByArea( trianglePolygonExample );
    EXPECT_GT( length, 1.7 );
    EXPECT_LT( length, 1.8 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, polylinePolygonIntersectionTest )
{
    std::vector<cvf::Vec3d> polygonExample;

    polygonExample.push_back( cvf::Vec3d( 0.00, 0.00, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 0.00, 2.50, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 1.50, 2.50, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 1.50, 0.00, 0.0 ) );

    std::vector<cvf::Vec3d> polyLine;

    polyLine.push_back( cvf::Vec3d( -1.00, 0.00, 1.0 ) );
    polyLine.push_back( cvf::Vec3d( 1.00, 2.0, 2.0 ) );
    polyLine.push_back( cvf::Vec3d( 1.00, 3.00, 3.0 ) );

    {
        std::vector<std::vector<cvf::Vec3d>> clippedLines =
            RigCellGeometryTools::clipPolylineByPolygon( polyLine, polygonExample, RigCellGeometryTools::INTERPOLATE_LINE_Z );

        EXPECT_EQ( (size_t)1, clippedLines.size() );
        EXPECT_EQ( (size_t)3, clippedLines.front().size() );
        EXPECT_EQ( 0.0, clippedLines.front()[0].x() );
        EXPECT_EQ( 1.0, clippedLines.front()[0].y() );
        EXPECT_EQ( 1.5, clippedLines.front()[0].z() );

        EXPECT_EQ( 1.0, clippedLines.front()[1].x() );
        EXPECT_EQ( 2.0, clippedLines.front()[1].y() );
        EXPECT_EQ( 2.0, clippedLines.front()[1].z() );

        EXPECT_EQ( 1.0, clippedLines.front()[2].x() );
        EXPECT_EQ( 2.5, clippedLines.front()[2].y() );
        EXPECT_EQ( 2.5, clippedLines.front()[2].z() );
    }

    {
        std::vector<std::vector<cvf::Vec3d>> clippedLines =
            RigCellGeometryTools::clipPolylineByPolygon( polyLine, polygonExample, RigCellGeometryTools::USE_HUGEVAL );

        EXPECT_EQ( (size_t)1, clippedLines.size() );
        EXPECT_EQ( (size_t)3, clippedLines.front().size() );
        EXPECT_EQ( 0.0, clippedLines.front()[0].x() );
        EXPECT_EQ( 1.0, clippedLines.front()[0].y() );
        EXPECT_EQ( HUGE_VAL, clippedLines.front()[0].z() );

        EXPECT_EQ( 1.0, clippedLines.front()[1].x() );
        EXPECT_EQ( 2.0, clippedLines.front()[1].y() );
        EXPECT_EQ( 2.0, clippedLines.front()[1].z() );

        EXPECT_EQ( 1.0, clippedLines.front()[2].x() );
        EXPECT_EQ( 2.5, clippedLines.front()[2].y() );
        EXPECT_EQ( HUGE_VAL, clippedLines.front()[2].z() );
    }

    polyLine.push_back( {-0.5, 1.5, 0.0} );

    {
        std::vector<std::vector<cvf::Vec3d>> clippedLines =
            RigCellGeometryTools::clipPolylineByPolygon( polyLine, polygonExample, RigCellGeometryTools::USE_HUGEVAL );

        EXPECT_EQ( (size_t)2, clippedLines.size() );
        EXPECT_EQ( (size_t)2, clippedLines.front().size() );
        EXPECT_EQ( (size_t)3, clippedLines.back().size() );

        EXPECT_EQ( 0.5, clippedLines.front()[0].x() );
        EXPECT_EQ( 2.5, clippedLines.front()[0].y() );
        EXPECT_EQ( HUGE_VAL, clippedLines.front()[0].z() );

        EXPECT_EQ( 0.0, clippedLines.front()[1].x() );
        EXPECT_EQ( 2.0, clippedLines.front()[1].y() );
        EXPECT_EQ( HUGE_VAL, clippedLines.front()[1].z() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCellGeometryTools, polylinePolygonIntersectionTest2 )
// Recreating bug...
{
    std::vector<cvf::Vec3d> polygonExample;

    polygonExample.push_back( cvf::Vec3d( 1.00, 0.00, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 0.00, 1.00, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( -1.00, 0.00, 0.0 ) );
    polygonExample.push_back( cvf::Vec3d( 0.00, -1.00, 0.0 ) );

    std::vector<cvf::Vec3d> polyLine1;
    polyLine1.push_back( cvf::Vec3d( 2.0, 2.0, 0.0 ) );
    polyLine1.push_back( cvf::Vec3d( 0.0, 0.0, 0.0 ) );
    polyLine1.push_back( cvf::Vec3d( -2.0, -2.0, 0.0 ) );

    std::vector<std::vector<cvf::Vec3d>> clippedLines1 =
        RigCellGeometryTools::clipPolylineByPolygon( polyLine1, polygonExample, RigCellGeometryTools::INTERPOLATE_LINE_Z );

    EXPECT_EQ( (size_t)1, clippedLines1.size() );
    EXPECT_EQ( 0.0, clippedLines1.front()[0].z() );

    std::vector<cvf::Vec3d> polyLine2;
    polyLine2.push_back( cvf::Vec3d( 2.0, 0.0, 0.0 ) );
    polyLine2.push_back( cvf::Vec3d( 0.0, 0.0, 0.0 ) );
    polyLine2.push_back( cvf::Vec3d( -2.0, 0.0, 0.0 ) );

    std::vector<std::vector<cvf::Vec3d>> clippedLines2 =
        RigCellGeometryTools::clipPolylineByPolygon( polyLine2, polygonExample, RigCellGeometryTools::INTERPOLATE_LINE_Z );
    EXPECT_EQ( (size_t)1, clippedLines2.size() );
    EXPECT_EQ( 0.0, clippedLines2.front()[0].z() );
    // Since both the line and the polygon is in the z=0 plane, the expected clipped line should be in this plane
}

#include "Completions/RigWellPathStimplanIntersector.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathStimplanIntersector, intersection )
{
    {
        cvf::Mat4d fractureXf = cvf::Mat4d::IDENTITY;
        fractureXf.setTranslation( {50.0f, 0.0f, 0.0f} );

        // std::vector<cvf::Vec3f> fracturePolygon ={ {0.0f, 0.0f, 0.0f},  {5.0f, 10.0f, 0.0f}, {10.0f, 0.0f, 0.0f} };
        double                               perforationLength = 25.0;
        std::vector<cvf::Vec3d>              wellPathPoints    = {{50.0f - 4.0f, 6.0f, 10.0f},
                                                  {50.0f + 6.0f, 6.0f, 0.0f},
                                                  {50.0f + 10.0f, 10.0f, -100.0f}};
        double                               wellRadius        = 1.5;
        std::vector<std::vector<cvf::Vec3d>> stpCellPolygons   = {
            {{0.0f, 0.0f, 0.0f}, {0.0f, 5.0f, 0.0f}, {5.0f, 5.0f, 0.0f}, {5.0f, 0.0f, 0.0f}},
            {{0.5f, 0.0f, 0.0f}, {0.5f, 5.0f, 0.0f}, {10.0f, 5.0f, 0.0f}, {10.0f, 0.0f, 0.0f}},
            {{0.0f, 5.0f, 0.0f}, {0.0f, 10.0f, 0.0f}, {5.0f, 10.0f, 0.0f}, {5.0f, 5.0f, 0.0f}},
            {{5.0f, 5.0f, 0.0f}, {5.0f, 10.0f, 0.0f}, {10.0f, 10.0f, 0.0f}, {10.0f, 5.0f, 0.0f}},
        };

        std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection> stimPlanCellIdxToIntersectionInfoMap;

        RigWellPathStimplanIntersectorTester::testCalculate( fractureXf,
                                                             wellPathPoints,
                                                             wellRadius,
                                                             perforationLength,
                                                             stpCellPolygons,
                                                             stimPlanCellIdxToIntersectionInfoMap );

        EXPECT_EQ( (size_t)2, stimPlanCellIdxToIntersectionInfoMap.size() );
        auto it = stimPlanCellIdxToIntersectionInfoMap.begin();
        EXPECT_EQ( (size_t)2, it->first );
        EXPECT_EQ( 1, it->second.endpointCount );
        ++it;
        EXPECT_EQ( (size_t)3, it->first );
        EXPECT_EQ( 1, it->second.endpointCount );
    }

    {
        cvf::Mat4d fractureXf = cvf::Mat4d::IDENTITY;

        //         std::vector<cvf::Vec3f> fracturePolygon ={ {0.0f, 0.0f, 0.0f},  {5.0f, 10.0f, 0.0f}, {10.0f, 0.0f,
        //         0.0f} };
        double                               perforationLength = 10;
        double                               wellRadius        = 1.5;
        std::vector<std::vector<cvf::Vec3d>> stpCellPolygons   = {
            {{0.0f, 0.0f, 0.0f}, {0.0f, 5.0f, 0.0f}, {5.0f, 5.0f, 0.0f}, {5.0f, 0.0f, 0.0f}},
            {{5.0f, 0.0f, 0.0f}, {5.0f, 5.0f, 0.0f}, {10.0f, 5.0f, 0.0f}, {10.0f, 0.0f, 0.0f}},
            {{0.0f, 5.0f, 0.0f}, {0.0f, 10.0f, 0.0f}, {5.0f, 10.0f, 0.0f}, {5.0f, 5.0f, 0.0f}},
            {{5.0f, 5.0f, 0.0f}, {5.0f, 10.0f, 0.0f}, {10.0f, 10.0f, 0.0f}, {10.0f, 5.0f, 0.0f}},
        };

        {
            std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection> stimPlanCellIdxToIntersectionInfoMap;
            std::vector<cvf::Vec3d> wellPathPoints = {{1.0f, 0.5f, 10.0f}, {1.0f, 1.5f, -10.0f}};

            RigWellPathStimplanIntersectorTester::testCalculate( fractureXf,
                                                                 wellPathPoints,
                                                                 wellRadius,
                                                                 perforationLength,
                                                                 stpCellPolygons,
                                                                 stimPlanCellIdxToIntersectionInfoMap );

            EXPECT_EQ( (size_t)1, stimPlanCellIdxToIntersectionInfoMap.size() );
            auto it = stimPlanCellIdxToIntersectionInfoMap.begin();
            EXPECT_EQ( (size_t)0, it->first );
            EXPECT_EQ( 2, it->second.endpointCount );
        }

        {
            std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection> stimPlanCellIdxToIntersectionInfoMap;
            std::vector<cvf::Vec3d> wellPathPoints = {{1.0f, 0.5f, 10.0f}, {1.0f, 1.0f, 0.5f}};

            RigWellPathStimplanIntersectorTester::testCalculate( fractureXf,
                                                                 wellPathPoints,
                                                                 wellRadius,
                                                                 perforationLength,
                                                                 stpCellPolygons,
                                                                 stimPlanCellIdxToIntersectionInfoMap );

            EXPECT_EQ( (size_t)1, stimPlanCellIdxToIntersectionInfoMap.size() );
            auto it = stimPlanCellIdxToIntersectionInfoMap.begin();
            EXPECT_EQ( (size_t)0, it->first );
            EXPECT_EQ( 2, it->second.endpointCount );
        }

        {
            std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection> stimPlanCellIdxToIntersectionInfoMap;
            std::vector<cvf::Vec3d> wellPathPoints = {{1.0f, 0.5f, 10.0f},
                                                      {1.0f, 1.0f, 0.5f},
                                                      {1.0f, 1.5f, -0.5f},
                                                      {1.0f, 2.0f, -10.0f}};

            RigWellPathStimplanIntersectorTester::testCalculate( fractureXf,
                                                                 wellPathPoints,
                                                                 wellRadius,
                                                                 perforationLength,
                                                                 stpCellPolygons,
                                                                 stimPlanCellIdxToIntersectionInfoMap );

            EXPECT_EQ( (size_t)1, stimPlanCellIdxToIntersectionInfoMap.size() );
            auto it = stimPlanCellIdxToIntersectionInfoMap.begin();
            EXPECT_EQ( (size_t)0, it->first );
            EXPECT_EQ( 2, it->second.endpointCount );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Test of whether we can transport edge information trough clipper clipping
// Seems as if it might be possible, but clipper will ignore the edge information
// processed by the callback if one of the edges is horizontal
//

#include "clipper/clipper.hpp"
double clipperConversionFactor2 = 10000; // For transform to clipper int

ClipperLib::IntPoint toClipperEdgePoint( const cvf::Vec3d& cvfPoint )
{
    int xInt = cvfPoint.x() * clipperConversionFactor2;
    int yInt = cvfPoint.y() * clipperConversionFactor2;
    int zInt = cvfPoint.z();
    return ClipperLib::IntPoint( xInt, yInt, zInt );
}

cvf::Vec3d fromClipperEdgePoint( const ClipperLib::IntPoint& clipPoint )
{
    double zDValue;

    if ( clipPoint.Z == std::numeric_limits<int>::max() )
    {
        zDValue = HUGE_VAL;
    }
    else
    {
        zDValue = clipPoint.Z;
    }

    return cvf::Vec3d( clipPoint.X / clipperConversionFactor2, clipPoint.Y / clipperConversionFactor2, zDValue );
}

void swapPointsIfNeedeed( ClipperLib::IntPoint*& p1, ClipperLib::IntPoint*& p2 )
{
    if ( p1->Z > p2->Z )
    {
        std::swap( p1, p2 );
    }

    if ( ( p2->Z - p1->Z ) > 1 ) // End edge of polygon
    {
        std::swap( p1, p2 ); // Swap back
    }
}

void clipperEdgeIntersectCallback( ClipperLib::IntPoint& e1bot,
                                   ClipperLib::IntPoint& e1top,
                                   ClipperLib::IntPoint& e2bot,
                                   ClipperLib::IntPoint& e2top,
                                   ClipperLib::IntPoint& pt )
{
    ClipperLib::IntPoint* e11 = &e1bot;
    ClipperLib::IntPoint* e12 = &e1top;
    ClipperLib::IntPoint* e21 = &e2bot;
    ClipperLib::IntPoint* e22 = &e2top;

    swapPointsIfNeedeed( e11, e12 );
    swapPointsIfNeedeed( e21, e22 );

    cvf::Vec3f e1( e12->X - e11->X, e12->Y - e11->Y, 0.0 );
    cvf::Vec3f e2( e22->X - e21->X, e22->Y - e21->Y, 0.0 );

    cvf::Vec3f up = e1 ^ e2;
    if ( up.z() > 0 )
    {
        pt.Z = e12->Z;
        std::cout << "E1 :" << e11->Z << " " << e12->Z << std::endl;
        std::cout << "E2 :" << e21->Z << " " << e22->Z << std::endl << std::endl;
    }
    else
    {
        pt.Z = e22->Z;
        std::cout << "E1 :" << e21->Z << " " << e22->Z << std::endl;
        std::cout << "E2 :" << e11->Z << " " << e12->Z << std::endl << std::endl;
    }
}

TEST( RigCellGeometryTools, ClipperEdgeTracking )
{
    // If the first edges of the polygons are horizontal, the edge tracking will fail.
    // Encode polygon and edge into Z coordinate of the vertex

    std::vector<cvf::Vec3d> polygon1 = {{0.0, 0.51, 1002.0}, {1.0, 0.5, 1000.0}, {0.0, 1.0, 1001.0}};
    std::vector<cvf::Vec3d> polygon2 = {{0.5, 0.01, 2002.0}, {1.0, 0.0, 2000.0}, {0.5, 1.0, 2001.0}};

    std::vector<std::vector<cvf::Vec3d>> clippedPolygons;

    // Convert to int for clipper library and store as clipper "path"
    ClipperLib::Path polygon1path;
    for ( const cvf::Vec3d& v : polygon1 )
    {
        polygon1path.push_back( toClipperEdgePoint( v ) );
    }

    ClipperLib::Path polygon2path;
    for ( const cvf::Vec3d& v : polygon2 )
    {
        polygon2path.push_back( toClipperEdgePoint( v ) );
    }

    ClipperLib::Clipper clpr;
    clpr.AddPath( polygon1path, ClipperLib::ptSubject, true );
    clpr.AddPath( polygon2path, ClipperLib::ptClip, true );

    clpr.ZFillFunction( &clipperEdgeIntersectCallback );

    ClipperLib::Paths solution;
    clpr.Execute( ClipperLib::ctIntersection, solution, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd );

    // Convert back to std::vector<std::vector<cvf::Vec3d> >
    for ( ClipperLib::Path pathInSol : solution )
    {
        std::vector<cvf::Vec3d> clippedPolygon;
        for ( ClipperLib::IntPoint IntPosition : pathInSol )
        {
            clippedPolygon.push_back( fromClipperEdgePoint( IntPosition ) );
        }
        clippedPolygons.push_back( clippedPolygon );
    }

    int pIdx = 1;
    for ( auto& polygon : clippedPolygons )
    {
        std::cout << "Polygon: " << pIdx << std::endl;
        for ( auto& point : polygon )
        {
            std::cout << "   [ " << point[0] << ", " << point[1] << ", " << point[2] << " ]" << std::endl;
        }
        pIdx++;
    }
}
