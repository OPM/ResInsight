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

#include "RigStimPlanModelTools.h"

#include "cvfMath.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStimPlanModelTools, calculateFormationDipAlignedToAzimuth )
{
    const double epsilon = 1.0e-9;

    // Formation direction vectors for a bed with 20 degree dip. The orientation of the vector
    // (+/-) depends on the fracture normal, which follows the well drilling direction. The result
    // must be independent of that orientation, and positive when the bed descends toward the
    // fracture azimuth direction.
    const double omega = 20.0;
    const double c     = std::cos( cvf::Math::toRadians( omega ) );
    const double s     = std::sin( cvf::Math::toRadians( omega ) );

    // Azimuth 90 (east). Bed dipping down toward east: positive dip.
    EXPECT_NEAR( omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( c, 0.0, -s ), 90.0 ), epsilon );
    // Same bed, opposite vector orientation (well drilled the other way): same result.
    EXPECT_NEAR( omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( -c, 0.0, s ), 90.0 ), epsilon );
    // Bed dipping down toward west: negative dip.
    EXPECT_NEAR( -omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( c, 0.0, s ), 90.0 ), epsilon );
    EXPECT_NEAR( -omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( -c, 0.0, -s ), 90.0 ), epsilon );

    // Azimuth 270 (west). Same bed dipping toward east now descends away from the azimuth direction.
    EXPECT_NEAR( -omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( c, 0.0, -s ), 270.0 ), epsilon );
    EXPECT_NEAR( -omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( -c, 0.0, s ), 270.0 ), epsilon );

    // Azimuth 0 (north). Bed dipping down toward north: positive dip.
    EXPECT_NEAR( omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( 0.0, c, -s ), 0.0 ), epsilon );
    EXPECT_NEAR( omega, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( 0.0, -c, s ), 0.0 ), epsilon );

    // Horizontal formation direction: zero dip for any azimuth.
    EXPECT_NEAR( 0.0, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( 1.0, 0.0, 0.0 ), 90.0 ), epsilon );
    EXPECT_NEAR( 0.0, RigStimPlanModelTools::calculateFormationDipAlignedToAzimuth( cvf::Vec3d( 0.0, 1.0, 0.0 ), 45.0 ), epsilon );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStimPlanModelTools, calculateFormationDipFromHorizontal )
{
    const double epsilon = 1.0e-9;

    const double omega = 20.0;
    const double c     = std::cos( cvf::Math::toRadians( omega ) );
    const double s     = std::sin( cvf::Math::toRadians( omega ) );

    // Result is non-negative and independent of the vector orientation.
    EXPECT_NEAR( omega, RigStimPlanModelTools::calculateFormationDipFromHorizontal( cvf::Vec3d( c, 0.0, -s ) ), epsilon );
    EXPECT_NEAR( omega, RigStimPlanModelTools::calculateFormationDipFromHorizontal( cvf::Vec3d( -c, 0.0, s ) ), epsilon );
    EXPECT_NEAR( omega, RigStimPlanModelTools::calculateFormationDipFromHorizontal( cvf::Vec3d( c, 0.0, s ) ), epsilon );

    EXPECT_NEAR( 0.0, RigStimPlanModelTools::calculateFormationDipFromHorizontal( cvf::Vec3d( 1.0, 0.0, 0.0 ) ), epsilon );
}
