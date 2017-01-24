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
TEST(RigCellGeometryTools, createPolygonTest)
{
    cvf::Vec3d a  = cvf::Vec3d(1, 1, 1);
    cvf::Vec3d b  = cvf::Vec3d(1, 3.14159265359, 1);
    cvf::Vec3d b2 = cvf::Vec3d(1, 3.1415926536, 1);
    cvf::Vec3d c  = cvf::Vec3d(5, 5, 1);
    cvf::Vec3d d  = cvf::Vec3d(-2, 8, 1);

    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;
    intersectionLineSegments.push_back({a, b});
    intersectionLineSegments.push_back({b2,c});
    intersectionLineSegments.push_back({c, d});
    intersectionLineSegments.push_back({a, d});

    std::vector<std::vector<cvf::Vec3d>> polygons;

    RigCellGeometryTools::createPolygonFromLineSegments(intersectionLineSegments, polygons);

    EXPECT_EQ(polygons.size(), 1);
    EXPECT_EQ(polygons[0].size(), 5);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigCellGeometryTools, createMultiplePolygonTest)
{
    cvf::Vec3d a1 = cvf::Vec3d(5, 4, 1);
    cvf::Vec3d b1 = cvf::Vec3d(6, 3, 1);
    cvf::Vec3d c1 = cvf::Vec3d(6, 4, 1);

    cvf::Vec3d a2 = cvf::Vec3d(2, 1, 1);
    cvf::Vec3d b2 = cvf::Vec3d(1, 3, 1);
    cvf::Vec3d c2 = cvf::Vec3d(1, 5, 1);



    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;
    intersectionLineSegments.push_back({ a1, b1 });
    intersectionLineSegments.push_back({ b1, c1 });

    intersectionLineSegments.push_back({ a2, b2 });
    intersectionLineSegments.push_back({ b2, c2 });

    std::vector<std::vector<cvf::Vec3d>> polygons;

    RigCellGeometryTools::createPolygonFromLineSegments(intersectionLineSegments, polygons);

    EXPECT_EQ(polygons.size(), 2);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigCellGeometryTools, planeHexCellIntersectionTest)
{
    cvf::Vec3d hexCorners[8];
    hexCorners[0] = cvf::Vec3d(0, 0, 0);
    hexCorners[1] = cvf::Vec3d(1, 0, 0);
    hexCorners[2] = cvf::Vec3d(0, 1, 0);
    hexCorners[3] = cvf::Vec3d(0, 0, 1);
    hexCorners[4] = cvf::Vec3d(0, 1, 1);
    hexCorners[5] = cvf::Vec3d(1, 1, 0);
    hexCorners[6] = cvf::Vec3d(1, 0, 1);
    hexCorners[7] = cvf::Vec3d(1, 1, 1);

    std::list<std::pair<cvf::Vec3d, cvf::Vec3d > > intersectionLineSegments;
    bool isCellIntersected = false;
    cvf::Plane fracturePlane;
    
    fracturePlane.setFromPointAndNormal(cvf::Vec3d(0.5, 0.5, 0.5), cvf::Vec3d(1, 0, 0) );
    isCellIntersected = RigCellGeometryTools::planeHexCellIntersection(hexCorners, fracturePlane, intersectionLineSegments);
    EXPECT_TRUE(isCellIntersected);

    fracturePlane.setFromPointAndNormal(cvf::Vec3d(1.5, 1.5, 1.5), cvf::Vec3d(1, 0, 0));
    isCellIntersected = RigCellGeometryTools::planeHexCellIntersection(hexCorners, fracturePlane, intersectionLineSegments);
    EXPECT_FALSE(isCellIntersected);

}






// 
// //--------------------------------------------------------------------------------------------------
// /// 
// //--------------------------------------------------------------------------------------------------
// TEST(RigStatisticsMath, RankPercentiles)
// {
//     std::vector<double> values;
//     values.push_back(HUGE_VAL);
//     values.push_back(2788.2723335651900);
//     values.push_back(-22481.0927881701000);
//     values.push_back(68778.6851686236000);
//     values.push_back(-76092.8157632591000);
//     values.push_back(6391.97999909729003);
//     values.push_back(65930.1200169780000);
//     values.push_back(-27696.2320267235000);
//     values.push_back(HUGE_VAL);
//     values.push_back(HUGE_VAL);
//     values.push_back(96161.7546348456000);
//     values.push_back(73875.6716288563000);
//     values.push_back(80720.4378655615000);
//     values.push_back(-98649.8109937874000);
//     values.push_back(99372.9362079615000);
//     values.push_back(HUGE_VAL);
//     values.push_back(-57020.4389966513000);
// 
//     std::vector<double> pValPos;
//     pValPos.push_back(10);
//     pValPos.push_back(40);
//     pValPos.push_back(50);
//     pValPos.push_back(90);
//     std::vector<double> pVals = RigStatisticsMath::calculateNearestRankPercentiles(values, pValPos);
// 
//     EXPECT_DOUBLE_EQ( -76092.8157632591000, pVals[0]);
//     EXPECT_DOUBLE_EQ( 2788.2723335651900  , pVals[1]);
//     EXPECT_DOUBLE_EQ( 6391.979999097290   , pVals[2]);
//     EXPECT_DOUBLE_EQ( 96161.7546348456000 , pVals[3]);
// }
// 
// 
// 
// 
// //--------------------------------------------------------------------------------------------------
// /// 
// //--------------------------------------------------------------------------------------------------
// TEST(RigStatisticsMath, HistogramPercentiles)
// {
//     std::vector<double> values;
//     values.push_back(HUGE_VAL);
//     values.push_back(2788.2723335651900);
//     values.push_back(-22481.0927881701000);
//     values.push_back(68778.6851686236000);
//     values.push_back(-76092.8157632591000);
//     values.push_back(6391.97999909729003);
//     values.push_back(65930.1200169780000);
//     values.push_back(-27696.2320267235000);
//     values.push_back(HUGE_VAL);
//     values.push_back(HUGE_VAL);
//     values.push_back(96161.7546348456000);
//     values.push_back(73875.6716288563000);
//     values.push_back(80720.4378655615000);
//     values.push_back(-98649.8109937874000);
//     values.push_back(99372.9362079615000);
//     values.push_back(HUGE_VAL);
//     values.push_back(-57020.4389966513000);
// 
// 
//     double min, max, range, mean, stdev;
//     RigStatisticsMath::calculateBasicStatistics(values, &min, &max, NULL, &range, &mean, &stdev);
// 
//     std::vector<size_t> histogram;
//     RigHistogramCalculator histCalc(min, max, 100, &histogram);
//     histCalc.addData(values);
//     std::vector<double> pVals;
//     double p10, p50, p90;
//     p10 = histCalc.calculatePercentil(0.1);
//     p50 = histCalc.calculatePercentil(0.5);
//     p90 = histCalc.calculatePercentil(0.9);
// 
//     EXPECT_DOUBLE_EQ( -76273.240559989776, p10);
//     EXPECT_DOUBLE_EQ( 5312.1312871307755  , p50);
//     EXPECT_DOUBLE_EQ( 94818.413022321271 , p90);
// }
// 
// 
// //--------------------------------------------------------------------------------------------------
// /// 
// //--------------------------------------------------------------------------------------------------
// TEST(RigStatisticsMath, InterpolatedPercentiles)
// {
//     std::vector<double> values;
//     values.push_back(HUGE_VAL);
//     values.push_back(2788.2723335651900);
//     values.push_back(-22481.0927881701000);
//     values.push_back(68778.6851686236000);
//     values.push_back(-76092.8157632591000);
//     values.push_back(6391.97999909729003);
//     values.push_back(65930.1200169780000);
//     values.push_back(-27696.2320267235000);
//     values.push_back(HUGE_VAL);
//     values.push_back(HUGE_VAL);
//     values.push_back(96161.7546348456000);
//     values.push_back(73875.6716288563000);
//     values.push_back(80720.4378655615000);
//     values.push_back(-98649.8109937874000);
//     values.push_back(99372.9362079615000);
//     values.push_back(HUGE_VAL);
//     values.push_back(-57020.4389966513000);
// 
// 
//     std::vector<double> pValPos;
//     pValPos.push_back(10);
//     pValPos.push_back(40);
//     pValPos.push_back(50);
//     pValPos.push_back(90);
//     std::vector<double> pVals = RigStatisticsMath::calculateInterpolatedPercentiles(values, pValPos);
// 
//     EXPECT_DOUBLE_EQ( -72278.340409937548,  pVals[0]);
//     EXPECT_DOUBLE_EQ(  -2265.6006907818719, pVals[1]);
//     EXPECT_DOUBLE_EQ(   6391.9799990972897, pVals[2]);
//     EXPECT_DOUBLE_EQ(  93073.49128098879,   pVals[3]);
// }
