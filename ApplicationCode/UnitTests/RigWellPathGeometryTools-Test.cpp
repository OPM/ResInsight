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

#include "RigWellPathGeometryTools.h"

#include <algorithm>
#include <complex>
#include <vector>

#define TOLERANCE 1.0e-7
#define SOLVER_TOLERANCE

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathGeometryTools, VerticalPath )
{
    std::vector<double> mdValues      = {100, 500, 1000};
    std::vector<double> tvdValues     = {100, 500, 1000};
    std::vector<double> fullTVDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTVDValues );

    EXPECT_EQ( fullTVDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < fullTVDValues.size(); ++i )
    {
        EXPECT_NEAR( fullTVDValues[i], fullMDValues[i], TOLERANCE );
    }
}

TEST( RigWellPathGeometryTools, LinearPath )
{
    std::vector<double> mdValues      = {100, 500, 1000};
    std::vector<double> tvdValues     = {50, 250, 500};
    std::vector<double> fullTVDValues = {50, 100, 150, 200, 250, 300, 350, 400, 450, 500};

    std::vector<double> fullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTVDValues );

    EXPECT_EQ( fullTVDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < fullTVDValues.size(); ++i )
    {
        EXPECT_NEAR( 2.0 * fullTVDValues[i], fullMDValues[i], TOLERANCE );
    }
}

TEST( RigWellPathGeometryTools, LinearPathStartingAtZero )
{
    std::vector<double> mdValues      = {0, 100, 500, 1000};
    std::vector<double> tvdValues     = {0, 50, 250, 500};
    std::vector<double> fullTVDValues = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500};

    std::vector<double> fullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTVDValues );

    EXPECT_EQ( fullTVDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < fullTVDValues.size(); ++i )
    {
        EXPECT_NEAR( 2.0 * fullTVDValues[i], fullMDValues[i], TOLERANCE );
    }
}

double quadraticFunction( double x )
{
    return 0.0015 * std::pow( x, 2 ) - 0.25 * x + 100;
}

TEST( RigWellPathGeometryTools, QuadraticPath )
{
    std::vector<double> mdValues = {100, 300, 600, 1000};
    std::vector<double> tvdValues;
    for ( double md : mdValues )
    {
        tvdValues.push_back( quadraticFunction( md ) );
    }
    std::vector<double> fullMDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullTvdValues;
    for ( double md : fullMDValues )
    {
        fullTvdValues.push_back( quadraticFunction( md ) );
    }

    std::vector<double> estimatedFullMDValues =
        RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTvdValues );
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
    std::vector<double> mdValues = {100, 300, 700, 1000};
    std::vector<double> tvdValues;
    for ( double md : mdValues )
    {
        tvdValues.push_back( cubicFunction( md ) );
    }
    std::vector<double> fullMDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullTvdValues;
    for ( double md : fullMDValues )
    {
        fullTvdValues.push_back( cubicFunction( md ) );
    }

    std::vector<double> estimatedFullMDValues =
        RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTvdValues );
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
    std::vector<double> mdValues = {100, 300, 600, 1000};
    std::vector<double> tvdValues;
    for ( double md : mdValues )
    {
        tvdValues.push_back( cubicFunction( md ) );
    }
    std::vector<double> fullMDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullTvdValues;
    for ( double md : fullMDValues )
    {
        fullTvdValues.push_back( cubicFunction( md ) );
    }

    std::vector<double> estimatedFullMDValues =
        RigWellPathGeometryTools::interpolateMdFromTvd( mdValues, tvdValues, fullTvdValues );
    EXPECT_EQ( estimatedFullMDValues.size(), fullMDValues.size() );
    for ( size_t i = 0; i < estimatedFullMDValues.size(); ++i )
    {
        if ( std::find( mdValues.begin(), mdValues.end(), estimatedFullMDValues[i] ) != mdValues.end() )
        {
            EXPECT_NEAR( fullMDValues[i], estimatedFullMDValues[i], TOLERANCE );
        }
    }
}
