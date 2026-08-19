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

#include "RifReaderOpmCommon.h"

#include "RiaEclipseUnitTools.h"

//--------------------------------------------------------------------------------------------------
/// opm-common scales MAPAXES to meter based on MAPUNITS, while the cell corner coordinates are left in the units
/// given by GRIDUNIT. Verify that the map axes are scaled back into grid units.
//--------------------------------------------------------------------------------------------------
TEST( RifReaderOpmCommon, MapAxesScaleFactor )
{
    const int gridUnitMetric  = 1;
    const int gridUnitField   = 2;
    const int gridUnitLab     = 3;
    const int gridUnitUnknown = -1;

    const double feetToMeter = RiaEclipseUnitTools::meterPerFeet();

    // MAPUNITS is missing, opm-common has not scaled the map axes
    EXPECT_DOUBLE_EQ( 1.0, RifReaderOpmCommon::mapAxesScaleFactor( "", gridUnitField ) );
    EXPECT_DOUBLE_EQ( 1.0, RifReaderOpmCommon::mapAxesScaleFactor( "", gridUnitMetric ) );

    // MAPUNITS is not recognized by opm-common, the map axes are left unscaled
    EXPECT_DOUBLE_EQ( 1.0, RifReaderOpmCommon::mapAxesScaleFactor( "FT", gridUnitField ) );

    // Same unit for map axes and grid, the scaling applied by opm-common must be undone
    EXPECT_DOUBLE_EQ( 1.0, RifReaderOpmCommon::mapAxesScaleFactor( "METRES", gridUnitMetric ) );
    EXPECT_DOUBLE_EQ( 1.0 / feetToMeter, RifReaderOpmCommon::mapAxesScaleFactor( "FEET", gridUnitField ) );
    EXPECT_DOUBLE_EQ( 100.0, RifReaderOpmCommon::mapAxesScaleFactor( "CM", gridUnitLab ) );

    // Map axes and grid have different units, convert from meter to grid unit
    EXPECT_DOUBLE_EQ( 1.0, RifReaderOpmCommon::mapAxesScaleFactor( "FEET", gridUnitMetric ) );
    EXPECT_DOUBLE_EQ( 1.0 / feetToMeter, RifReaderOpmCommon::mapAxesScaleFactor( "METRES", gridUnitField ) );

    // Unknown grid unit, undo the scaling applied by opm-common
    EXPECT_DOUBLE_EQ( 1.0 / feetToMeter, RifReaderOpmCommon::mapAxesScaleFactor( "FEET", gridUnitUnknown ) );
    EXPECT_DOUBLE_EQ( 1.0, RifReaderOpmCommon::mapAxesScaleFactor( "METRES", gridUnitUnknown ) );
}
