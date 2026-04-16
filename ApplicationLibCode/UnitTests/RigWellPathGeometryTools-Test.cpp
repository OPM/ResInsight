/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "Well/RigWellPathGeometryTools.h"

#include <algorithm>
#include <complex>
#include <limits>
#include <vector>

#define TOLERANCE 1.0e-7
#define SOLVER_TOLERANCE

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathGeometryTools, VerticalPath )
{
    std::vector<double> mdValues      = { 100, 500, 1000 };
    std::vector<double> tvdValues     = { 100, 500, 1000 };
    std::vector<double> fullTVDValues = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };
    std::vector<double> fullMDValues  = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTVDValues );

    EXPECT_EQ( fullTVDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < fullTVDValues.size(); ++i )
    {
        EXPECT_NEAR( fullTVDValues[i], fullMDValues[i], TOLERANCE );
    }
}

TEST( RigWellPathGeometryTools, LinearPath )
{
    std::vector<double> mdValues      = { 100, 500, 1000 };
    std::vector<double> tvdValues     = { 50, 250, 500 };
    std::vector<double> fullTVDValues = { 50, 100, 150, 200, 250, 300, 350, 400, 450, 500 };

    std::vector<double> fullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTVDValues );

    EXPECT_EQ( fullTVDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < fullTVDValues.size(); ++i )
    {
        EXPECT_NEAR( 2.0 * fullTVDValues[i], fullMDValues[i], TOLERANCE );
    }
}

TEST( RigWellPathGeometryTools, LinearPathStartingAtZero )
{
    std::vector<double> mdValues      = { 0, 100, 500, 1000 };
    std::vector<double> tvdValues     = { 0, 50, 250, 500 };
    std::vector<double> fullTVDValues = { 0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500 };

    std::vector<double> fullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTVDValues );

    EXPECT_EQ( fullTVDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < fullTVDValues.size(); ++i )
    {
        EXPECT_NEAR( 2.0 * fullTVDValues[i], fullMDValues[i], TOLERANCE );
    }
}

TEST( RigWellPathGeometryTools, ShortLinearPathStartingAtZero )
{
    std::vector<double> mdValues         = { 0, 1000.0 };
    std::vector<double> tvdValues        = { 2000.0, 4000 };
    std::vector<double> fullTVDValues    = { 0, 2000.0, 3000, 4000.0 };
    std::vector<double> expectedMDValues = { 0.0, 0.0, 500, 1000.0 };

    std::vector<double> fullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTVDValues );

    EXPECT_EQ( fullTVDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < fullTVDValues.size(); ++i )
    {
        EXPECT_NEAR( expectedMDValues[i], fullMDValues[i], TOLERANCE );
    }
}

double quadraticFunction( double x )
{
    return 0.0015 * std::pow( x, 2 ) - 0.25 * x + 100;
}

TEST( RigWellPathGeometryTools, QuadraticPath )
{
    std::vector<double> mdValues = { 100, 300, 600, 1000 };
    std::vector<double> tvdValues;
    for ( double md : mdValues )
    {
        tvdValues.push_back( quadraticFunction( md ) );
    }
    std::vector<double> fullMDValues = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };
    std::vector<double> fullTvdValues;
    for ( double md : fullMDValues )
    {
        fullTvdValues.push_back( quadraticFunction( md ) );
    }

    std::vector<double> estimatedFullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTvdValues );
    EXPECT_EQ( estimatedFullMDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < estimatedFullMDValues.size(); ++i )
    {
        if ( std::find( mdValues.begin(), mdValues.end(), estimatedFullMDValues[i] ) != mdValues.end() )
        {
            EXPECT_NEAR( fullMDValues[i], estimatedFullMDValues[i], TOLERANCE );
        }
    }
}

double cubicFunction( double x )
{
    return 0.000012 * std::pow( x, 3 ) - 0.0175 * std::pow( x, 2 ) + 7 * x;
}

TEST( RigWellPathGeometryTools, CubicPath )
{
    std::vector<double> mdValues = { 100, 300, 700, 1000 };
    std::vector<double> tvdValues;
    for ( double md : mdValues )
    {
        tvdValues.push_back( cubicFunction( md ) );
    }
    std::vector<double> fullMDValues = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };
    std::vector<double> fullTvdValues;
    for ( double md : fullMDValues )
    {
        fullTvdValues.push_back( cubicFunction( md ) );
    }

    std::vector<double> estimatedFullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTvdValues );
    EXPECT_EQ( estimatedFullMDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < estimatedFullMDValues.size(); ++i )
    {
        if ( std::find( mdValues.begin(), mdValues.end(), estimatedFullMDValues[i] ) != mdValues.end() )
        {
            EXPECT_NEAR( fullMDValues[i], estimatedFullMDValues[i], TOLERANCE );
        }
    }
}

TEST( RigWellPathGeometryTools, CubicPathPoorSampling )
{
    std::vector<double> mdValues = { 100, 300, 600, 1000 };
    std::vector<double> tvdValues;
    for ( double md : mdValues )
    {
        tvdValues.push_back( cubicFunction( md ) );
    }
    std::vector<double> fullMDValues = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };
    std::vector<double> fullTvdValues;
    for ( double md : fullMDValues )
    {
        fullTvdValues.push_back( cubicFunction( md ) );
    }

    std::vector<double> estimatedFullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTvdValues );
    EXPECT_EQ( estimatedFullMDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < estimatedFullMDValues.size(); ++i )
    {
        if ( std::find( mdValues.begin(), mdValues.end(), estimatedFullMDValues[i] ) != mdValues.end() )
        {
            EXPECT_NEAR( fullMDValues[i], estimatedFullMDValues[i], TOLERANCE );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// When infinity is passed as a TVD value (e.g. for invalid grid cells), the function must not
/// crash and must return a result vector of the same size as the input.
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathGeometryTools, InfinityTvdValues_OutputSizeMatchesInput )
{
    const double inf = std::numeric_limits<double>::infinity();

    std::vector<double> mdValues  = { 100, 500, 1000 };
    std::vector<double> tvdValues = { 100, 500, 1000 };

    // All entries are infinity
    std::vector<double> allInfTvd = { inf, inf, inf };
    std::vector<double> result    = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, allInfTvd );
    EXPECT_EQ( result.size(), allInfTvd.size() );
}

//--------------------------------------------------------------------------------------------------
/// Valid TVD entries that are mixed with infinity entries must still yield correct MD values.
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathGeometryTools, InfinityTvdValues_ValidEntriesStillInterpolatedCorrectly )
{
    const double inf = std::numeric_limits<double>::infinity();

    // Vertical well: MD == TVD
    std::vector<double> mdValues  = { 100, 500, 1000 };
    std::vector<double> tvdValues = { 100, 500, 1000 };

    // Mix of infinity and valid TVD values
    std::vector<double> mixedTvd = { 100.0, inf, 500.0, inf, 1000.0 };
    std::vector<double> result   = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, mixedTvd );

    EXPECT_EQ( result.size(), mixedTvd.size() );

    // Valid entries at indices 0, 2, 4 should interpolate correctly (MD == TVD for a vertical well)
    EXPECT_NEAR( result[0], 100.0, TOLERANCE );
    EXPECT_NEAR( result[2], 500.0, TOLERANCE );
    EXPECT_NEAR( result[4], 1000.0, TOLERANCE );
}

//--------------------------------------------------------------------------------------------------
/// A single infinity entry in an otherwise valid list must not corrupt subsequent results.
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathGeometryTools, InfinityTvdValues_SingleInfinityDoesNotCorruptOtherResults )
{
    const double inf = std::numeric_limits<double>::infinity();

    // Vertical well: MD == TVD
    std::vector<double> mdValues  = { 100, 500, 1000 };
    std::vector<double> tvdValues = { 100, 500, 1000 };

    std::vector<double> tvdWithInf = { 200.0, inf, 800.0 };
    std::vector<double> result     = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, tvdWithInf );

    EXPECT_EQ( result.size(), tvdWithInf.size() );
    EXPECT_NEAR( result[0], 200.0, TOLERANCE );
    // inf TVD at index 1 is linearly interpolated between neighbors: (200 + 800) / 2 = 500
    EXPECT_NEAR( result[1], 500.0, TOLERANCE );
    EXPECT_NEAR( result[2], 800.0, TOLERANCE );
}

//--------------------------------------------------------------------------------------------------
/// N consecutive infinity TVD entries must be distributed evenly across the MD interval defined
/// by the nearest valid neighbors.
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathGeometryTools, InfinityTvdValues_ConsecutiveInfinityDistributedEvenly )
{
    const double inf = std::numeric_limits<double>::infinity();

    // Vertical well: MD == TVD
    std::vector<double> mdValues  = { 100, 500, 1000 };
    std::vector<double> tvdValues = { 100, 500, 1000 };

    // Three consecutive inf entries between TVD 200 and TVD 800
    std::vector<double> tvdWithInf = { 200.0, inf, inf, inf, 800.0 };
    std::vector<double> result     = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, tvdWithInf );

    EXPECT_EQ( result.size(), tvdWithInf.size() );
    EXPECT_NEAR( result[0], 200.0, TOLERANCE );
    // Three inf entries divide [200, 800] into four equal steps of 150 each
    EXPECT_NEAR( result[1], 350.0, TOLERANCE );
    EXPECT_NEAR( result[2], 500.0, TOLERANCE );
    EXPECT_NEAR( result[3], 650.0, TOLERANCE );
    EXPECT_NEAR( result[4], 800.0, TOLERANCE );
}
