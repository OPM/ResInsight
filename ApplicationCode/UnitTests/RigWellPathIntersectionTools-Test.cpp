/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RigWellPathIntersectionTools.h"

#include "cvfStructGrid.h"


TEST(RigWellPathIntersectionTools, TestCalculateMainAxisVector)
{
    std::array<cvf::Vec3d, 8> hexCorners = {
        cvf::Vec3d(0.0,  0.0,  0.0),
        cvf::Vec3d(10.0, 0.0,  0.0),
        cvf::Vec3d(10.0, 10.0, 0.0),
        cvf::Vec3d(0.0,  10.0, 0.0),
        cvf::Vec3d(0.0,  0.0,  10.0),
        cvf::Vec3d(10.0, 0.0,  10.0),
        cvf::Vec3d(10.0, 10.0, 10.0),
        cvf::Vec3d(0.0,  10.0, 10.0),
    };

    cvf::Vec3d iMainAxisVector = RigWellPathIntersectionTools::calculateCellMainAxisVector(hexCorners, cvf::StructGridInterface::NEG_I, cvf::StructGridInterface::POS_I);
    EXPECT_EQ(10, iMainAxisVector.x());
    EXPECT_EQ(0,  iMainAxisVector.y());
    EXPECT_EQ(0,  iMainAxisVector.z());

    cvf::Vec3d jMainAxisVector = RigWellPathIntersectionTools::calculateCellMainAxisVector(hexCorners, cvf::StructGridInterface::NEG_J, cvf::StructGridInterface::POS_J);
    EXPECT_EQ(0 , jMainAxisVector.x());
    EXPECT_EQ(10, jMainAxisVector.y());
    EXPECT_EQ(0,  jMainAxisVector.z());

    cvf::Vec3d kMainAxisVector = RigWellPathIntersectionTools::calculateCellMainAxisVector(hexCorners, cvf::StructGridInterface::NEG_K, cvf::StructGridInterface::POS_K);
    EXPECT_EQ(0,  kMainAxisVector.x());
    EXPECT_EQ(0,  kMainAxisVector.y());
    EXPECT_EQ(10, kMainAxisVector.z());
}

TEST(RigWellPathIntersectionTools, TestCalculateMainAxisVectors)
{
    std::array<cvf::Vec3d, 8> hexCorners = {
        cvf::Vec3d(0.0,  0.0,  0.0),
        cvf::Vec3d(10.0, 0.0,  0.0),
        cvf::Vec3d(10.0, 10.0, 0.0),
        cvf::Vec3d(0.0,  10.0, 0.0),
        cvf::Vec3d(0.0,  0.0,  10.0),
        cvf::Vec3d(10.0, 0.0,  10.0),
        cvf::Vec3d(10.0, 10.0, 10.0),
        cvf::Vec3d(0.0,  10.0, 10.0),
    };


    cvf::Vec3d iMainAxisVector;
    cvf::Vec3d jMainAxisVector;
    cvf::Vec3d kMainAxisVector;
    RigWellPathIntersectionTools::calculateCellMainAxisVectors(hexCorners, &iMainAxisVector, &jMainAxisVector, &kMainAxisVector);

    EXPECT_EQ(10, iMainAxisVector.x());
    EXPECT_EQ(0,  iMainAxisVector.y());
    EXPECT_EQ(0,  iMainAxisVector.z());

    EXPECT_EQ(0 , jMainAxisVector.x());
    EXPECT_EQ(10, jMainAxisVector.y());
    EXPECT_EQ(0,  jMainAxisVector.z());

    EXPECT_EQ(0,  kMainAxisVector.x());
    EXPECT_EQ(0,  kMainAxisVector.y());
    EXPECT_EQ(10, kMainAxisVector.z());
}

TEST(RigWellPathIntersectionTools, TestCalculateDirectionInCellThroughCellCenters)
{
    std::array<cvf::Vec3d, 8> hexCorners = {
        cvf::Vec3d(0.0,  0.0,  0.0),
        cvf::Vec3d(10.0, 0.0,  0.0),
        cvf::Vec3d(10.0, 10.0, 0.0),
        cvf::Vec3d(0.0,  10.0, 0.0),
        cvf::Vec3d(0.0,  0.0,  10.0),
        cvf::Vec3d(10.0, 0.0,  10.0),
        cvf::Vec3d(10.0, 10.0, 10.0),
        cvf::Vec3d(0.0,  10.0, 10.0),
    };

    {
        cvf::Vec3d startPoint(0, 5, 5);
        cvf::Vec3d endPoint(10, 5, 5);
        WellPathCellDirection direction = RigWellPathIntersectionTools::calculateDirectionInCell(hexCorners, startPoint, endPoint);
        EXPECT_EQ(POS_I, direction);
    }

    {
        cvf::Vec3d startPoint(5, 0, 5);
        cvf::Vec3d endPoint(5, 10, 5);
        WellPathCellDirection direction = RigWellPathIntersectionTools::calculateDirectionInCell(hexCorners, startPoint, endPoint);
        EXPECT_EQ(POS_J, direction);
    }

    {
        cvf::Vec3d startPoint(5, 5, 0);
        cvf::Vec3d endPoint(5, 5, 10);
        WellPathCellDirection direction = RigWellPathIntersectionTools::calculateDirectionInCell(hexCorners, startPoint, endPoint);
        EXPECT_EQ(POS_K, direction);
    }
}

TEST(RigWellPathIntersectionTools, TestCalculateJDirectionThroughCells)
{
    std::array<cvf::Vec3d, 8> hexCorners = {
        cvf::Vec3d(402380, 7.21315e+06, -4219.43),
        cvf::Vec3d(402475, 7.21311e+06, -4226.57),
        cvf::Vec3d(402510, 7.2132e+06, -4226.75),
        cvf::Vec3d(402414, 7.21324e+06, -4222.8),
        cvf::Vec3d(402385, 7.21314e+06, -4232.71),
        cvf::Vec3d(402480, 7.2131e+06, -4239.88),
        cvf::Vec3d(402515, 7.2132e+06, -4239.94),
        cvf::Vec3d(402419, 7.21323e+06, -4235.84),
    };

    cvf::Vec3d startPoint(402428, 7.21323e+06, -4223.35);
    cvf::Vec3d endPoint(402399, 7.21314e+06, -4224.48);
    WellPathCellDirection direction = RigWellPathIntersectionTools::calculateDirectionInCell(hexCorners, startPoint, endPoint);

    EXPECT_EQ(NEG_J, direction);
}
