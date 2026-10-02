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

#include "RiuWorkflowGraphLayout.h"

using Edges = std::vector<std::pair<QString, QString>>;

//--------------------------------------------------------------------------------------------------
TEST( RiuWorkflowGraphLayout, diamond )
{
    const auto layout =
        RiuWorkflowGraphLayout::computeLayers( { "a", "b", "c", "d" }, Edges{ { "a", "b" }, { "a", "c" }, { "b", "d" }, { "c", "d" } } );

    ASSERT_EQ( 3u, layout.layers.size() );
    EXPECT_EQ( QStringList( { "a" } ), layout.layers[0] );
    EXPECT_EQ( QStringList( { "b", "c" } ), layout.layers[1] );
    EXPECT_EQ( QStringList( { "d" } ), layout.layers[2] );
}

//--------------------------------------------------------------------------------------------------
TEST( RiuWorkflowGraphLayout, nonTopologicalInput )
{
    const auto layout =
        RiuWorkflowGraphLayout::computeLayers( { "d", "c", "b", "a" }, Edges{ { "c", "d" }, { "b", "c" }, { "a", "b" }, { "a", "d" } } );

    EXPECT_EQ( 0, layout.ranks.value( "a" ) );
    EXPECT_EQ( 1, layout.ranks.value( "b" ) );
    EXPECT_EQ( 2, layout.ranks.value( "c" ) );
    EXPECT_EQ( 3, layout.ranks.value( "d" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RiuWorkflowGraphLayout, disconnectedNodes )
{
    const auto layout = RiuWorkflowGraphLayout::computeLayers( { "z", "a", "m" }, Edges{ { "a", "unknown" } } );

    ASSERT_EQ( 1u, layout.layers.size() );
    EXPECT_EQ( QStringList( { "a", "m", "z" } ), layout.layers[0] );
}

//--------------------------------------------------------------------------------------------------
TEST( RiuWorkflowGraphLayout, cycle )
{
    const auto layout = RiuWorkflowGraphLayout::computeLayers( { "start", "a", "b", "end" },
                                                               Edges{ { "start", "a" }, { "a", "b" }, { "b", "a" }, { "b", "end" } } );

    EXPECT_EQ( 4, layout.ranks.size() );
    EXPECT_EQ( 0, layout.ranks.value( "start" ) );
    EXPECT_LT( layout.ranks.value( "a" ), layout.ranks.value( "b" ) );
    EXPECT_LT( layout.ranks.value( "b" ), layout.ranks.value( "end" ) );

    int nodeCount = 0;
    for ( const auto& layer : layout.layers )
        nodeCount += layer.size();
    EXPECT_EQ( 4, nodeCount );
}

//--------------------------------------------------------------------------------------------------
TEST( RiuWorkflowGraphLayout, deterministicOrder )
{
    const Edges edges{ { "root", "x" }, { "root", "y" }, { "other", "w" }, { "x", "end" } };
    const auto  first  = RiuWorkflowGraphLayout::computeLayers( { "root", "other", "x", "y", "w", "end" }, edges );
    const auto  second = RiuWorkflowGraphLayout::computeLayers( { "end", "w", "y", "x", "other", "root" }, edges );

    EXPECT_EQ( first.layers, second.layers );
    ASSERT_EQ( 3u, first.layers.size() );
    EXPECT_EQ( QStringList( { "other", "root" } ), first.layers[0] );
    // Children of "other" (position 0) come before children of "root" (position 1)
    EXPECT_EQ( QStringList( { "w", "x", "y" } ), first.layers[1] );
}
