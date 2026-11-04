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

#include "RiuGroupNetworkLayout.h"

using NodeParents = std::vector<std::pair<QString, QString>>;

//--------------------------------------------------------------------------------------------------
TEST( RiuGroupNetworkLayout, parentCentredOnChildren )
{
    const auto positions = RiuGroupNetworkLayout::computePositions(
        NodeParents{ { "FIELD", "" }, { "A", "FIELD" }, { "B", "FIELD" }, { "P1", "A" }, { "P2", "A" }, { "P3", "B" } } );

    ASSERT_EQ( 6, positions.size() );
    EXPECT_EQ( 0, positions["FIELD"].column );
    EXPECT_EQ( 1, positions["A"].column );
    EXPECT_EQ( 2, positions["P3"].column );

    EXPECT_DOUBLE_EQ( 0.0, positions["P1"].row );
    EXPECT_DOUBLE_EQ( 1.0, positions["P2"].row );
    EXPECT_DOUBLE_EQ( 2.0, positions["P3"].row );
    EXPECT_DOUBLE_EQ( 0.5, positions["A"].row );
    EXPECT_DOUBLE_EQ( 2.0, positions["B"].row );
    EXPECT_DOUBLE_EQ( 1.25, positions["FIELD"].row );
}

//--------------------------------------------------------------------------------------------------
TEST( RiuGroupNetworkLayout, childrenBeforeParentsAndUnknownParents )
{
    // A child listed before its parent is still placed below it. An unknown parent gives a root.
    const auto positions =
        RiuGroupNetworkLayout::computePositions( NodeParents{ { "P1", "A" }, { "A", "FIELD" }, { "FIELD", "" }, { "X", "MISSING" } } );

    ASSERT_EQ( 4, positions.size() );
    EXPECT_EQ( 2, positions["P1"].column );
    EXPECT_EQ( 0, positions["X"].column );
    EXPECT_DOUBLE_EQ( 0.0, positions["P1"].row );
    EXPECT_DOUBLE_EQ( 1.0, positions["X"].row );
}

//--------------------------------------------------------------------------------------------------
TEST( RiuGroupNetworkLayout, cycleIsLeftOut )
{
    const auto positions = RiuGroupNetworkLayout::computePositions( NodeParents{ { "FIELD", "" }, { "A", "B" }, { "B", "A" } } );

    EXPECT_EQ( 1, positions.size() );
    EXPECT_TRUE( positions.contains( "FIELD" ) );
}
