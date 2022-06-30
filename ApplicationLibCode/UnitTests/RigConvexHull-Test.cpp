/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RigConvexHull.h"

#include "cvfVector3.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigConvexHullTests, SimpleExample )
{
    std::vector<cvf::Vec3d> points = { cvf::Vec3d( 0.0, 0.0, 0.0 ),
                                       cvf::Vec3d( 0.0, 1.0, 0.0 ),
                                       cvf::Vec3d( 1.0, 1.0, 0.0 ),
                                       cvf::Vec3d( 1.0, 0.0, 0.0 ),
                                       cvf::Vec3d( 1.0, 1.0, 0.0 ),
                                       cvf::Vec3d( 0.5, 0.5, 0.0 ),
                                       cvf::Vec3d( 0.25, 0.25, 0.0 ),
                                       cvf::Vec3d( 0.5, 1.25, 0.0 ),
                                       cvf::Vec3d( 0.5, 0.75, 0.0 ) };

    std::vector<cvf::Vec3d> convexHull = RigConvexHull::compute2d( points );

    std::vector<cvf::Vec3d> expectedPoints = { cvf::Vec3d( 0, 0, 0 ),
                                               cvf::Vec3d( 0, 1, 0 ),
                                               cvf::Vec3d( 0.5, 1.25, 0 ),
                                               cvf::Vec3d( 1, 1, 0 ),
                                               cvf::Vec3d( 1, 0, 0 ) };

    EXPECT_EQ( 5u, convexHull.size() );
    for ( size_t i = 0; i < convexHull.size(); i++ )
    {
        EXPECT_EQ( expectedPoints[i].x(), convexHull[i].x() );
        EXPECT_EQ( expectedPoints[i].y(), convexHull[i].y() );
    }
}
