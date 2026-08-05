/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "Well/RigWellPath.h"

//--------------------------------------------------------------------------------------------------
/// A well path without geometry must not throw, and must report the point as undefined
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathTest, InterpolatedPointForEmptyGeometry )
{
    RigWellPath wellPath;

    cvf::Vec3d point = wellPath.interpolatedPointAlongWellPath( 100.0 );
    EXPECT_TRUE( point.isUndefined() );

    double horizontalLength = -1.0;
    point                   = wellPath.interpolatedPointAlongWellPath( 100.0, &horizontalLength );
    EXPECT_TRUE( point.isUndefined() );
    EXPECT_DOUBLE_EQ( 0.0, horizontalLength );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathTest, InterpolatedPointAlongWellPath )
{
    std::vector<cvf::Vec3d> points = { cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( 0.0, 0.0, -100.0 ), cvf::Vec3d( 0.0, 0.0, -200.0 ) };
    std::vector<double>     mds    = { 0.0, 100.0, 200.0 };

    RigWellPath wellPath( points, mds );

    // Before, inside and after the measured depth range
    EXPECT_EQ( cvf::Vec3d( 0.0, 0.0, 0.0 ), wellPath.interpolatedPointAlongWellPath( -10.0 ) );
    EXPECT_EQ( cvf::Vec3d( 0.0, 0.0, -50.0 ), wellPath.interpolatedPointAlongWellPath( 50.0 ) );
    EXPECT_EQ( cvf::Vec3d( 0.0, 0.0, -200.0 ), wellPath.interpolatedPointAlongWellPath( 500.0 ) );
}
