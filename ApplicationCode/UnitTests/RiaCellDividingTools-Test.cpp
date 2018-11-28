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

#include "RiaCellDividingTools.h"
#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
/// Helper
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8> createRegularCellCoords(cvf::Vec3d refPt, double xLen, double yLen, double zLen)
{
    std::array<cvf::Vec3d, 8> coords = {
        refPt + cvf::Vec3d(0, 0, 0),
        refPt + cvf::Vec3d(xLen, 0, 0),
        refPt + cvf::Vec3d(xLen, yLen, 0),
        refPt + cvf::Vec3d(0, yLen, 0),
        refPt + cvf::Vec3d(0, 0, zLen),
        refPt + cvf::Vec3d(xLen, 0, zLen),
        refPt + cvf::Vec3d(xLen, yLen, zLen),
        refPt + cvf::Vec3d(0, yLen, zLen),
    };
    return coords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaCellDividingTools, flowDistanceCubicMainCell_AreaPointInCenter)
{          
    std::array<cvf::Vec3d, 8> mainCellCorners = createRegularCellCoords(cvf::Vec3d(10, 10, 10), 10, 10, 10);
    cvf::Vec3d point(15, 15, 15);

    double dist = RiaCellDividingTools::computeFlowDistance(mainCellCorners, point);

    double expectedDist =
        (
            (cvf::Vec3d(12.5, 12.5, 12.5) - point).length() + (cvf::Vec3d(17.5, 12.5, 12.5) - point).length() +
            (cvf::Vec3d(12.5, 17.5, 12.5) - point).length() + (cvf::Vec3d(17.5, 17.5, 12.5) - point).length() +
            (cvf::Vec3d(12.5, 12.5, 17.5) - point).length() + (cvf::Vec3d(17.5, 12.5, 17.5) - point).length() +
            (cvf::Vec3d(12.5, 17.5, 17.5) - point).length() + (cvf::Vec3d(17.5, 17.5, 17.5) - point).length()
        ) / 8;

    EXPECT_NEAR(expectedDist, dist, 1e-6);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RiaCellDividingTools, flowDistanceCubicMainCell_AreaPointNearCorner)
{
    std::array<cvf::Vec3d, 8> mainCellCorners = createRegularCellCoords(cvf::Vec3d(10, 10, 10), 10, 10, 10);
    cvf::Vec3d                point(11, 11, 11);

    double dist = RiaCellDividingTools::computeFlowDistance(mainCellCorners, point);

    double expectedDist = ((cvf::Vec3d(12.5, 12.5, 12.5) - point).length() + (cvf::Vec3d(17.5, 12.5, 12.5) - point).length() +
                           (cvf::Vec3d(12.5, 17.5, 12.5) - point).length() + (cvf::Vec3d(17.5, 17.5, 12.5) - point).length() +
                           (cvf::Vec3d(12.5, 12.5, 17.5) - point).length() + (cvf::Vec3d(17.5, 12.5, 17.5) - point).length() +
                           (cvf::Vec3d(12.5, 17.5, 17.5) - point).length() + (cvf::Vec3d(17.5, 17.5, 17.5) - point).length()) /
                          8;

    EXPECT_NEAR(expectedDist, dist, 1e-6);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RiaCellDividingTools, flowDistanceHighMainCell_AreaPointNearLowerCorner)
{
    std::array<cvf::Vec3d, 8> mainCellCorners = createRegularCellCoords(cvf::Vec3d(10, 10, 10), 10, 10, 100);
    cvf::Vec3d                point(11, 11, 11);

    double dist = RiaCellDividingTools::computeFlowDistance(mainCellCorners, point);

    double expectedDist = ((cvf::Vec3d(12.5, 12.5, 35) - point).length() + (cvf::Vec3d(17.5, 12.5, 35) - point).length() +
                           (cvf::Vec3d(12.5, 17.5, 35) - point).length() + (cvf::Vec3d(17.5, 17.5, 35) - point).length() +
                           (cvf::Vec3d(12.5, 12.5, 85) - point).length() + (cvf::Vec3d(17.5, 12.5, 85) - point).length() +
                           (cvf::Vec3d(12.5, 17.5, 85) - point).length() + (cvf::Vec3d(17.5, 17.5, 85) - point).length()) /
                          8;

    EXPECT_NEAR(expectedDist, dist, 1e-6);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
  TEST(RigCellGeometryTools, lgrNodesTest)
{
    std::array<cvf::Vec3d, 8> mainCellCorners = createRegularCellCoords(cvf::Vec3d(10, 10, 10), 36, 18, 18);
    auto                      coords          = RiaCellDividingTools::createHexCornerCoords(mainCellCorners, 6, 3, 3);
    

}
