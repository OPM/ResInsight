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

#include "RigWellPath.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigWellPathTest, FindWellPathCoordsIncludingIntersectionPoint)
{
    RigWellPath wellPathGeometry;
    {
        std::vector<cvf::Vec3d> wellPathPoints;
        std::vector<double> mdValues;

        wellPathPoints.push_back(cvf::Vec3d(0.0, 0.0, 0.0));
        wellPathPoints.push_back(cvf::Vec3d(0.0, 1.0, 0.0));
        wellPathPoints.push_back(cvf::Vec3d(0.0, 2.0, 0.0));
        wellPathPoints.push_back(cvf::Vec3d(0.0, 3.0, 0.0));
        wellPathPoints.push_back(cvf::Vec3d(0.0, 4.0, 0.0));

        mdValues.push_back(0.0);
        mdValues.push_back(1.0);
        mdValues.push_back(2.0);
        mdValues.push_back(3.0);
        mdValues.push_back(4.0);


        wellPathGeometry.m_wellPathPoints = wellPathPoints;
        wellPathGeometry.m_measuredDepths = mdValues;
    }

    // Before first MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint(-1.0);
        EXPECT_EQ(5u, wellPathPoints.size());
    }

    // Identical to first MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint(0.0);
        EXPECT_EQ(5u, wellPathPoints.size());
    }

    // Identical to second MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint(1.0);
        EXPECT_EQ(5u, wellPathPoints.size());
    }

    // Between first and second MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint(0.3);
        EXPECT_EQ(6u, wellPathPoints.size());
    }

    // Identical to lastMD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint(4.0);
        EXPECT_EQ(5u, wellPathPoints.size());
    }

    // Larger than lastMD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint(10.0);
        EXPECT_EQ(5u, wellPathPoints.size());
    }
}

