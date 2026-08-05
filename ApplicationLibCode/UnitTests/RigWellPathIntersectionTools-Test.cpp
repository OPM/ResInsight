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

#include "Well/RigWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellPathIntersectionTools.h"

#include "cvfVector3.h"

#include <limits>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathTest, FindWellPathCoordsIncludingIntersectionPoint )
{
    RigWellPath wellPathGeometry;
    {
        std::vector<cvf::Vec3d> wellPathPoints;
        std::vector<double>     mdValues;

        wellPathPoints.push_back( cvf::Vec3d( 0.0, 0.0, 0.0 ) );
        wellPathPoints.push_back( cvf::Vec3d( 0.0, 1.0, 0.0 ) );
        wellPathPoints.push_back( cvf::Vec3d( 0.0, 2.0, 0.0 ) );
        wellPathPoints.push_back( cvf::Vec3d( 0.0, 3.0, 0.0 ) );
        wellPathPoints.push_back( cvf::Vec3d( 0.0, 4.0, 0.0 ) );

        mdValues.push_back( 0.0 );
        mdValues.push_back( 1.0 );
        mdValues.push_back( 2.0 );
        mdValues.push_back( 3.0 );
        mdValues.push_back( 4.0 );

        wellPathGeometry.setWellPathPoints( wellPathPoints, mdValues );
    }

    // Before first MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint( -1.0 );
        EXPECT_EQ( 5u, wellPathPoints.size() );
    }

    // Identical to first MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint( 0.0 );
        EXPECT_EQ( 5u, wellPathPoints.size() );
    }

    // Identical to second MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint( 1.0 );
        EXPECT_EQ( 5u, wellPathPoints.size() );
    }

    // Between first and second MD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint( 0.3 );
        EXPECT_EQ( 6u, wellPathPoints.size() );
    }

    // Identical to lastMD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint( 4.0 );
        EXPECT_EQ( 5u, wellPathPoints.size() );
    }

    // Larger than lastMD
    {
        auto wellPathPoints = wellPathGeometry.wellPathPointsIncludingInterpolatedIntersectionPoint( 10.0 );
        EXPECT_EQ( 5u, wellPathPoints.size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathIntersectionToolsTest, BuildContinuousIntersectionsIsSortedByMeasuredDepth )
{
    auto createIntersection = []( size_t globCellIndex, double startMD, double endMD )
    {
        WellPathCellIntersectionInfo intersection;
        intersection.globCellIndex               = globCellIndex;
        intersection.startPoint                  = cvf::Vec3d( 0.0, 0.0, -startMD );
        intersection.endPoint                    = cvf::Vec3d( 0.0, 0.0, -endMD );
        intersection.startMD                     = startMD;
        intersection.endMD                       = endMD;
        intersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;
        intersection.intersectedCellFaceIn       = cvf::StructGridInterface::NEG_K;
        intersection.intersectedCellFaceOut      = cvf::StructGridInterface::POS_K;
        return intersection;
    };

    // Two cells, then a gap where the well path is outside the grid, then a third cell
    std::vector<WellPathCellIntersectionInfo> intersections;
    intersections.push_back( createIntersection( 0, 100.0, 110.0 ) );
    intersections.push_back( createIntersection( 1, 110.0, 120.0 ) );
    intersections.push_back( createIntersection( 2, 200.0, 210.0 ) );

    auto continuousIntersections = RigWellPathIntersectionTools::buildContinuousIntersections( intersections, nullptr );

    ASSERT_EQ( 4u, continuousIntersections.size() );

    // The gap intersection is inserted between the second and third cell
    EXPECT_EQ( std::numeric_limits<size_t>::max(), continuousIntersections[2].globCellIndex );
    EXPECT_DOUBLE_EQ( 120.0, continuousIntersections[2].startMD );
    EXPECT_DOUBLE_EQ( 200.0, continuousIntersections[2].endMD );

    for ( size_t i = 1; i < continuousIntersections.size(); i++ )
    {
        EXPECT_GE( continuousIntersections[i].startMD, continuousIntersections[i - 1].startMD );
        EXPECT_GE( continuousIntersections[i].endMD, continuousIntersections[i - 1].endMD );
    }
}
