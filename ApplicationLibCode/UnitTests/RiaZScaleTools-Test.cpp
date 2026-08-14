/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaDefines.h"
#include "RiaZScaleTools.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaZScaleToolsTest, NextScaleFactor )
{
    const std::vector<double> options = { 0.1, 0.5, 1.0, 5.0, 10.0 };

    // Step from a predefined value
    EXPECT_DOUBLE_EQ( 5.0, RiaZScaleTools::nextScaleFactor( 1.0, options ) );

    // Step from a custom value between two options
    EXPECT_DOUBLE_EQ( 5.0, RiaZScaleTools::nextScaleFactor( 3.0, options ) );

    // Step from below the smallest option
    EXPECT_DOUBLE_EQ( 0.1, RiaZScaleTools::nextScaleFactor( 0.01, options ) );

    // The largest option and values above it are unchanged
    EXPECT_DOUBLE_EQ( 10.0, RiaZScaleTools::nextScaleFactor( 10.0, options ) );
    EXPECT_DOUBLE_EQ( 100.0, RiaZScaleTools::nextScaleFactor( 100.0, options ) );

    EXPECT_DOUBLE_EQ( 2.0, RiaZScaleTools::nextScaleFactor( 2.0, {} ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaZScaleToolsTest, PreviousScaleFactor )
{
    const std::vector<double> options = { 0.1, 0.5, 1.0, 5.0, 10.0 };

    // Step from a predefined value
    EXPECT_DOUBLE_EQ( 0.5, RiaZScaleTools::previousScaleFactor( 1.0, options ) );

    // Step from a custom value between two options
    EXPECT_DOUBLE_EQ( 1.0, RiaZScaleTools::previousScaleFactor( 3.0, options ) );

    // Step from above the largest option
    EXPECT_DOUBLE_EQ( 10.0, RiaZScaleTools::previousScaleFactor( 100.0, options ) );

    // The smallest option and values below it are unchanged
    EXPECT_DOUBLE_EQ( 0.1, RiaZScaleTools::previousScaleFactor( 0.1, options ) );
    EXPECT_DOUBLE_EQ( 0.01, RiaZScaleTools::previousScaleFactor( 0.01, options ) );

    EXPECT_DOUBLE_EQ( 2.0, RiaZScaleTools::previousScaleFactor( 2.0, {} ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaZScaleToolsTest, StepThroughCustomScaleFactorInBothDirections )
{
    // A custom factor between two predefined values must be visited when stepping in both directions
    const std::vector<double> options = { 1.0, 5.0, 7.0, 10.0 };

    EXPECT_DOUBLE_EQ( 7.0, RiaZScaleTools::nextScaleFactor( 5.0, options ) );
    EXPECT_DOUBLE_EQ( 10.0, RiaZScaleTools::nextScaleFactor( 7.0, options ) );
    EXPECT_DOUBLE_EQ( 7.0, RiaZScaleTools::previousScaleFactor( 10.0, options ) );
    EXPECT_DOUBLE_EQ( 5.0, RiaZScaleTools::previousScaleFactor( 7.0, options ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaZScaleToolsTest, ScaleFactorOptionsAreSortedAndUnique )
{
    RiaZScaleTools::registerScaleFactor( 7.0 );
    RiaZScaleTools::registerScaleFactor( 7.0 );
    RiaZScaleTools::registerScaleFactor( 0.3 );

    // Registering a predefined value or an invalid value must not create duplicates or new entries
    RiaZScaleTools::registerScaleFactor( 1.0 );
    RiaZScaleTools::registerScaleFactor( 0.0 );
    RiaZScaleTools::registerScaleFactor( -2.0 );

    auto options = RiaZScaleTools::scaleFactorOptions();

    EXPECT_TRUE( std::is_sorted( options.begin(), options.end() ) );
    EXPECT_EQ( options.end(), std::adjacent_find( options.begin(), options.end() ) );

    EXPECT_NE( options.end(), std::find( options.begin(), options.end(), 7.0 ) );
    EXPECT_NE( options.end(), std::find( options.begin(), options.end(), 0.3 ) );
    EXPECT_EQ( options.end(), std::find( options.begin(), options.end(), 0.0 ) );
    EXPECT_EQ( options.end(), std::find( options.begin(), options.end(), -2.0 ) );

    // All predefined values must be present
    for ( auto predefined : RiaDefines::viewScaleOptions() )
    {
        EXPECT_NE( options.end(), std::find( options.begin(), options.end(), predefined ) );
    }
}
