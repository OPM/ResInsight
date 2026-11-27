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

#include "Well/RigGeoGuidedWellPlacementTools.h"
#include "Well/RigWellLogExtractor.h"

#include <set>

namespace
{
// Vertical ray through ten 10 m thick layers, from z = 0 down to z = -100. Cell index equals layer index.
std::vector<WellPathCellIntersectionInfo> layeredRay()
{
    std::vector<WellPathCellIntersectionInfo> intersections;
    for ( size_t k = 0; k < 10; ++k )
    {
        WellPathCellIntersectionInfo info{};
        info.globCellIndex = k;
        info.startPoint    = { 0.0, 0.0, -10.0 * k };
        info.endPoint      = { 0.0, 0.0, -10.0 * ( k + 1 ) };
        info.startMD       = 10.0 * k;
        info.endMD         = 10.0 * ( k + 1 );
        intersections.push_back( info );
    }
    return intersections;
}

std::function<bool( size_t )> includeCells( std::set<size_t> cells )
{
    return [cells]( size_t cellIndex ) { return cells.contains( cellIndex ); };
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigGeoGuidedWellPlacementTools, SingleZoneAndFraction )
{
    auto intervals = RigGeoGuidedWellPlacementTools::filterIntervals( layeredRay(), includeCells( { 2, 3, 4 } ) );
    ASSERT_EQ( 1u, intervals.size() );
    EXPECT_DOUBLE_EQ( -20.0, intervals[0].top );
    EXPECT_DOUBLE_EQ( -50.0, intervals[0].bottom );

    EXPECT_DOUBLE_EQ( -50.0, RigGeoGuidedWellPlacementTools::zAtFraction( intervals[0], 0.0 ) );
    EXPECT_DOUBLE_EQ( -35.0, RigGeoGuidedWellPlacementTools::zAtFraction( intervals[0], 0.5 ) );
    EXPECT_DOUBLE_EQ( -20.0, RigGeoGuidedWellPlacementTools::zAtFraction( intervals[0], 1.0 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigGeoGuidedWellPlacementTools, DisjointZonesSelectClosest )
{
    auto intervals = RigGeoGuidedWellPlacementTools::filterIntervals( layeredRay(), includeCells( { 1, 2, 7 } ) );
    ASSERT_EQ( 2u, intervals.size() );
    EXPECT_DOUBLE_EQ( -10.0, intervals[0].top );
    EXPECT_DOUBLE_EQ( -30.0, intervals[0].bottom );
    EXPECT_DOUBLE_EQ( -70.0, intervals[1].top );
    EXPECT_DOUBLE_EQ( -80.0, intervals[1].bottom );

    // Inside a zone
    auto inside = RigGeoGuidedWellPlacementTools::closestInterval( intervals, -75.0 );
    ASSERT_TRUE( inside.has_value() );
    EXPECT_DOUBLE_EQ( -70.0, inside->top );

    // Between the zones, closer to the upper one
    auto between = RigGeoGuidedWellPlacementTools::closestInterval( intervals, -40.0 );
    ASSERT_TRUE( between.has_value() );
    EXPECT_DOUBLE_EQ( -10.0, between->top );

    // Below all zones
    auto below = RigGeoGuidedWellPlacementTools::closestInterval( intervals, -200.0 );
    ASSERT_TRUE( below.has_value() );
    EXPECT_DOUBLE_EQ( -80.0, below->bottom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigGeoGuidedWellPlacementTools, GapAlongRaySplitsZone )
{
    // A gap in the grid between layer 2 and 3 splits otherwise consecutive filter cells
    auto intersections = layeredRay();
    for ( size_t k = 3; k < intersections.size(); ++k )
    {
        intersections[k].startMD += 5.0;
        intersections[k].endMD += 5.0;
    }

    auto intervals = RigGeoGuidedWellPlacementTools::filterIntervals( intersections, includeCells( { 2, 3 } ) );
    EXPECT_EQ( 2u, intervals.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigGeoGuidedWellPlacementTools, NoHit )
{
    auto intervals = RigGeoGuidedWellPlacementTools::filterIntervals( layeredRay(), includeCells( {} ) );
    EXPECT_TRUE( intervals.empty() );
    EXPECT_FALSE( RigGeoGuidedWellPlacementTools::closestInterval( intervals, -10.0 ).has_value() );
}
