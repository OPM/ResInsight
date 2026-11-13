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

#include "RigMdRange.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMdRange, ConstructorOrdersStartAndEnd )
{
    const RigMdRange range( 200.0, 100.0 );
    EXPECT_DOUBLE_EQ( 100.0, range.start() );
    EXPECT_DOUBLE_EQ( 200.0, range.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMdRange, MergedIsEmptyForNoRanges )
{
    EXPECT_FALSE( RigMdRange::merged( {} ).has_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMdRange, MergedSpansGapsAndIgnoresInputOrder )
{
    const auto single = RigMdRange::merged( { RigMdRange( 100.0, 200.0 ) } );
    ASSERT_TRUE( single.has_value() );
    EXPECT_DOUBLE_EQ( 100.0, single->start() );
    EXPECT_DOUBLE_EQ( 200.0, single->end() );

    const auto merged = RigMdRange::merged( { RigMdRange( 500.0, 600.0 ), RigMdRange( 100.0, 200.0 ), RigMdRange( 150.0, 250.0 ) } );
    ASSERT_TRUE( merged.has_value() );
    EXPECT_DOUBLE_EQ( 100.0, merged->start() );
    EXPECT_DOUBLE_EQ( 600.0, merged->end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMdRange, Overlaps )
{
    const RigMdRange range( 100.0, 200.0 );

    EXPECT_TRUE( range.overlaps( RigMdRange( 150.0, 160.0 ) ) ); // inside
    EXPECT_TRUE( range.overlaps( RigMdRange( 50.0, 120.0 ) ) ); // partly before
    EXPECT_TRUE( range.overlaps( RigMdRange( 180.0, 300.0 ) ) ); // partly after
    EXPECT_TRUE( range.overlaps( RigMdRange( 50.0, 300.0 ) ) ); // covers the range
    EXPECT_TRUE( range.overlaps( RigMdRange( 160.0, 150.0 ) ) ); // reversed interval

    EXPECT_FALSE( range.overlaps( RigMdRange( 0.0, 50.0 ) ) );
    EXPECT_FALSE( range.overlaps( RigMdRange( 250.0, 300.0 ) ) );
    EXPECT_FALSE( range.overlaps( RigMdRange( 50.0, 100.0 ) ) ); // touching the start
    EXPECT_FALSE( range.overlaps( RigMdRange( 200.0, 250.0 ) ) ); // touching the end
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMdRange, PointRangeIsOverlappedByContainingInterval )
{
    const RigMdRange point( 100.0, 100.0 );

    EXPECT_TRUE( point.overlaps( RigMdRange( 90.0, 110.0 ) ) );
    EXPECT_TRUE( point.overlaps( RigMdRange( 100.0, 110.0 ) ) );
    EXPECT_FALSE( point.overlaps( RigMdRange( 110.0, 120.0 ) ) );
    EXPECT_TRUE( RigMdRange( 90.0, 110.0 ).overlaps( point ) ); // symmetric
    EXPECT_FALSE( RigMdRange( 110.0, 120.0 ).overlaps( point ) );
}
