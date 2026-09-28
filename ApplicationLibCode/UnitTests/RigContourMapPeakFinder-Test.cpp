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

#include "ContourMap/RigContourMapPeakFinder.h"

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
TEST( RigContourMapPeakFinderTest, SinglePeak )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, 0.0 );
    z[2 + 2 * nx] = 10.0; // center cell

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks = 5;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, peaks.size() );
    EXPECT_EQ( 2, peaks[0].i );
    EXPECT_EQ( 2, peaks[0].j );
    EXPECT_DOUBLE_EQ( 10.0, peaks[0].z );
}

//--------------------------------------------------------------------------------------------------
/// Two separated peaks, both should be reported when far enough apart and maxPeaks allows it.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, TwoSeparatedPeaks )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[1] = 5.0;
    z[8] = 8.0;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks    = 10;
    settings.minDistance = 1.0;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 2u, peaks.size() );
    // Ranked by prominence/z descending: highest peak first.
    EXPECT_EQ( 8, peaks[0].i );
    EXPECT_EQ( 1, peaks[1].i );
}

//--------------------------------------------------------------------------------------------------
/// Two nearby peaks: with a large minimum distance only the more prominent one should survive.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, MinimumDistanceSuppressesNearbyPeak )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[4] = 5.0;
    z[5] = 8.0;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks    = 10;
    settings.minDistance = 5.0;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, peaks.size() );
    EXPECT_EQ( 5, peaks[0].i );
    EXPECT_DOUBLE_EQ( 8.0, peaks[0].z );
}

//--------------------------------------------------------------------------------------------------
/// maxPeaks limits the number of returned peaks to the most significant ones.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, MaxPeaksLimitsResultCount )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[1] = 3.0;
    z[4] = 5.0;
    z[7] = 8.0;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks    = 2;
    settings.minDistance = 0.0;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 2u, peaks.size() );
    EXPECT_EQ( 7, peaks[0].i );
    EXPECT_EQ( 4, peaks[1].i );
}

//--------------------------------------------------------------------------------------------------
/// minProminence filters out low-significance shoulders/noise.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, MinimumProminenceFiltersNoise )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[4] = 0.5; // low prominence "noise"
    z[8] = 8.0;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks      = 10;
    settings.minProminence = 1.0;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, peaks.size() );
    EXPECT_EQ( 8, peaks[0].i );
}

//--------------------------------------------------------------------------------------------------
/// excludeEdgePeaks drops peaks touching the grid border.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, ExcludeEdgePeaks )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, 0.0 );
    z[0]          = 8.0; // corner cell, touches the edge
    z[2 + 2 * nx] = 5.0; // interior cell

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks         = 10;
    settings.excludeEdgePeaks = true;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, peaks.size() );
    EXPECT_EQ( 2, peaks[0].i );
    EXPECT_EQ( 2, peaks[0].j );
}

//--------------------------------------------------------------------------------------------------
/// Undefined (NaN) cells are ignored and must not be returned as peaks.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, UndefinedCellsAreIgnored )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, nanValue() );
    z[2 + 2 * nx] = 5.0;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks = 10;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, peaks.size() );
    EXPECT_EQ( 2, peaks[0].i );
    EXPECT_EQ( 2, peaks[0].j );
}

//--------------------------------------------------------------------------------------------------
/// World-space x/y coordinates should reflect origin and cell sizes.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, WorldCoordinatesUseOriginAndCellSize )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, 0.0 );
    z[3 + 2 * nx] = 5.0;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks = 10;

    const double originX = 100.0;
    const double originY = 200.0;
    const double dx      = 10.0;
    const double dy      = 20.0;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, dx, dy, settings, originX, originY );

    ASSERT_EQ( 1u, peaks.size() );
    EXPECT_DOUBLE_EQ( originX + 3 * dx, peaks[0].x );
    EXPECT_DOUBLE_EQ( originY + 2 * dy, peaks[0].y );
}

//--------------------------------------------------------------------------------------------------
/// rankByProminence = false ranks candidates by raw z value instead of prominence. The data has a high
/// peak with a high saddle next to the global maximum, and a lower but isolated (more prominent) peak,
/// so the two ranking modes select different second peaks.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, RankByZValue )
{
    const int nx = 10;
    const int ny = 1;

    std::vector<double> z( nx * ny, 0.0 );
    z[1] = 9.0; // high z, low prominence (0.5) due to the high saddle at i=2
    z[2] = 8.5;
    z[3] = 10.0; // global maximum
    z[7] = 5.0; // lower z, but isolated so higher prominence (5)

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks = 2;

    settings.rankByProminence = false;
    auto peaksByZ             = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 2u, peaksByZ.size() );
    EXPECT_EQ( 3, peaksByZ[0].i );
    EXPECT_EQ( 1, peaksByZ[1].i );
    EXPECT_DOUBLE_EQ( 0.5, peaksByZ[1].prominence );

    settings.rankByProminence = true;
    auto peaksByProminence    = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 2u, peaksByProminence.size() );
    EXPECT_EQ( 3, peaksByProminence[0].i );
    EXPECT_EQ( 7, peaksByProminence[1].i );
}

//--------------------------------------------------------------------------------------------------
/// Flat steps (shoulders) on a slope and flat background cells have zero prominence and must not be
/// reported as peaks.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, FlatStepIsNotAPeak )
{
    const int nx = 7;
    const int ny = 1;

    std::vector<double> z = { 0.0, 0.0, 3.0, 3.0, 5.0, 0.0, 0.0 };

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks = 10;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    ASSERT_EQ( 1u, peaks.size() );
    EXPECT_EQ( 4, peaks[0].i );
    EXPECT_DOUBLE_EQ( 5.0, peaks[0].prominence );
}

//--------------------------------------------------------------------------------------------------
/// Each separate data region measures prominence against its own minimum, not the global minimum.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, ProminenceIsPerDataRegion )
{
    const int nx = 5;
    const int ny = 1;

    std::vector<double> z = { 100.0, 101.0, nanValue(), 5.0, 6.0 };

    auto prominence = RigContourMapPeakFinder::computeProminence( z, nx, ny );

    EXPECT_DOUBLE_EQ( 1.0, prominence[1] );
    EXPECT_DOUBLE_EQ( 1.0, prominence[4] );
    EXPECT_TRUE( std::isnan( prominence[0] ) );
    EXPECT_TRUE( std::isnan( prominence[3] ) );
}

//--------------------------------------------------------------------------------------------------
/// A negative maxPeaks must not crash and returns no peaks.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, NegativeMaxPeaksReturnsNoPeaks )
{
    const int nx = 5;
    const int ny = 5;

    std::vector<double> z( nx * ny, 0.0 );
    z[2 + 2 * nx] = 10.0;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks = -1;

    auto peaks = RigContourMapPeakFinder::findPeaks( z, nx, ny, 1.0, 1.0, settings );

    EXPECT_TRUE( peaks.empty() );
}

//--------------------------------------------------------------------------------------------------
/// An empty grid should not crash and must return no peaks.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapPeakFinderTest, EmptyGridReturnsNoPeaks )
{
    std::vector<double> z;

    RigContourMapPeakFinder::Settings settings;
    auto                              peaks = RigContourMapPeakFinder::findPeaks( z, 0, 0, 1.0, 1.0, settings );

    EXPECT_TRUE( peaks.empty() );
}
