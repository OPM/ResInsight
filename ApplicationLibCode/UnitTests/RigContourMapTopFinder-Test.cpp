/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "ContourMap/RigContourMapTopFinder.h"

#include <cmath>
#include <limits>

namespace
{
double nanValue()
{
    return std::numeric_limits<double>::quiet_NaN();
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Single, isolated peak in an otherwise flat grid.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, SinglePeak )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, 0.0 );
    z[2 + 2 * nx] = 10.0; // center cell

    RigContourMapTopFinder::Settings settings;
    settings.maxTops = 5;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, tops.size() );
    EXPECT_EQ( 2, tops[0].i );
    EXPECT_EQ( 2, tops[0].j );
    EXPECT_DOUBLE_EQ( 10.0, tops[0].z );
}

//--------------------------------------------------------------------------------------------------
/// Two separated peaks, both should be reported when far enough apart and maxTops allows it.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, TwoSeparatedPeaks )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[1] = 5.0;
    z[8] = 8.0;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops     = 10;
    settings.minDistance = 1.0;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 2u, tops.size() );
    // Ranked by prominence/z descending: highest peak first.
    EXPECT_EQ( 8, tops[0].i );
    EXPECT_EQ( 1, tops[1].i );
}

//--------------------------------------------------------------------------------------------------
/// Two nearby peaks: with a large minimum distance only the more prominent one should survive.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, MinimumDistanceSuppressesNearbyPeak )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[4] = 5.0;
    z[5] = 8.0;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops     = 10;
    settings.minDistance = 5.0;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, tops.size() );
    EXPECT_EQ( 5, tops[0].i );
    EXPECT_DOUBLE_EQ( 8.0, tops[0].z );
}

//--------------------------------------------------------------------------------------------------
/// maxTops limits the number of returned tops to the most significant ones.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, MaxTopsLimitsResultCount )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[1] = 3.0;
    z[4] = 5.0;
    z[7] = 8.0;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops     = 2;
    settings.minDistance = 0.0;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 2u, tops.size() );
    EXPECT_EQ( 7, tops[0].i );
    EXPECT_EQ( 4, tops[1].i );
}

//--------------------------------------------------------------------------------------------------
/// minProminence filters out low-significance shoulders/noise.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, MinimumProminenceFiltersNoise )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[4] = 0.5; // low prominence "noise"
    z[8] = 8.0;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops       = 10;
    settings.minProminence = 1.0;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, tops.size() );
    EXPECT_EQ( 8, tops[0].i );
}

//--------------------------------------------------------------------------------------------------
/// excludeEdgeTops drops peaks touching the grid border.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, ExcludeEdgeTops )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, 0.0 );
    z[0]          = 8.0; // corner cell, touches the edge
    z[2 + 2 * nx] = 5.0; // interior cell

    RigContourMapTopFinder::Settings settings;
    settings.maxTops         = 10;
    settings.excludeEdgeTops = true;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, tops.size() );
    EXPECT_EQ( 2, tops[0].i );
    EXPECT_EQ( 2, tops[0].j );
}

//--------------------------------------------------------------------------------------------------
/// Undefined (NaN) cells are ignored and must not be returned as tops.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, UndefinedCellsAreIgnored )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, nanValue() );
    z[2 + 2 * nx] = 5.0;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops = 10;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, tops.size() );
    EXPECT_EQ( 2, tops[0].i );
    EXPECT_EQ( 2, tops[0].j );
}

//--------------------------------------------------------------------------------------------------
/// World-space x/y coordinates should reflect origin and cell sizes.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, WorldCoordinatesUseOriginAndCellSize )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, 0.0 );
    z[3 + 2 * nx] = 5.0;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops = 10;

    const double originX = 100.0;
    const double originY = 200.0;
    const double dx      = 10.0;
    const double dy      = 20.0;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, dx, dy, settings, originX, originY );

    ASSERT_EQ( 1u, tops.size() );
    EXPECT_DOUBLE_EQ( originX + 3 * dx, tops[0].x );
    EXPECT_DOUBLE_EQ( originY + 2 * dy, tops[0].y );
}

//--------------------------------------------------------------------------------------------------
/// rankByProminence = false ranks candidates by raw z value instead of prominence.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, RankByZValue )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[2] = 6.0; // higher z, lower prominence (shallow local bump)
    z[3] = 4.0;
    z[8] = 5.5; // lower z, but isolated so higher prominence

    RigContourMapTopFinder::Settings settings;
    settings.maxTops          = 1;
    settings.rankByProminence = false;

    auto tops = RigContourMapTopFinder::findTops( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, tops.size() );
    EXPECT_EQ( 2, tops[0].i );
    EXPECT_DOUBLE_EQ( 6.0, tops[0].z );
}

//--------------------------------------------------------------------------------------------------
/// An empty grid should not crash and must return no tops.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapTopFinderTest, EmptyGridReturnsNoTops )
{
    std::vector<double> z;

    RigContourMapTopFinder::Settings settings;
    auto                             tops = RigContourMapTopFinder::findTops( z, 0, 0, 1.0, 1.0, settings );

    EXPECT_TRUE( tops.empty() );
}
