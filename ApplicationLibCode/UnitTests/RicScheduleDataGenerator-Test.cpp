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

#include "Commands/CompletionExportCommands/RicScheduleDataGenerator.h"

#include "RigCompletionData.h"

#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"

#include <memory>

namespace
{
RigCompletionData makeCompletion( const QString& wellName, size_t globalCellIndex, double md, const RimPerforationInterval* source )
{
    RigCompletionData completion( wellName, RigCompletionDataGridCell( globalCellIndex, nullptr ), md );
    completion.setSourcePdmObject( source );
    return completion;
}

RimPerforationInterval* addPerforation( RimWellPath& well )
{
    auto* interval = new RimPerforationInterval;
    well.perforationIntervalCollection()->appendPerforation( interval );
    return interval;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Completions produced for a well's mainbore and its lateral can be interleaved by measured depth
/// (see #14963). sortCompletionsMainBoreFirst() must restore mainbore-first ordering while keeping
/// the relative order of connections belonging to the same well.
//--------------------------------------------------------------------------------------------------
TEST( RicScheduleDataGeneratorTest, MainBoreConnectionsSortBeforeLateralConnections )
{
    RimWellPath mainBore;
    RimWellPath lateral;
    lateral.connectWellPaths( &mainBore, 10.0 );

    auto* mainBorePerf1 = addPerforation( mainBore );
    auto* mainBorePerf2 = addPerforation( mainBore );
    auto* lateralPerf1  = addPerforation( lateral );
    auto* lateralPerf2  = addPerforation( lateral );

    // Deliberately interleaved by measured depth, as produced by the plain MD-based sort.
    std::vector<RigCompletionData> compdata = { makeCompletion( "WELL", 1, 15.0, lateralPerf1 ),
                                                makeCompletion( "WELL", 2, 20.0, mainBorePerf1 ),
                                                makeCompletion( "WELL", 3, 25.0, lateralPerf2 ),
                                                makeCompletion( "WELL", 4, 30.0, mainBorePerf2 ) };

    RicScheduleDataGenerator::sortCompletionsMainBoreFirst( { &mainBore }, compdata );

    ASSERT_EQ( 4u, compdata.size() );
    EXPECT_EQ( mainBorePerf1, compdata[0].sourcePdmObject() );
    EXPECT_EQ( mainBorePerf2, compdata[1].sourcePdmObject() );
    EXPECT_EQ( lateralPerf1, compdata[2].sourcePdmObject() );
    EXPECT_EQ( lateralPerf2, compdata[3].sourcePdmObject() );
}

//--------------------------------------------------------------------------------------------------
/// A connection whose owning well path cannot be determined (e.g. no source object) must not be
/// dropped; it is sorted after all connections with a known well.
//--------------------------------------------------------------------------------------------------
TEST( RicScheduleDataGeneratorTest, ConnectionsWithUnknownWellAreSortedLast )
{
    RimWellPath mainBore;
    auto*       mainBorePerf = addPerforation( mainBore );

    std::vector<RigCompletionData> compdata = { makeCompletion( "WELL", 1, 5.0, nullptr ), makeCompletion( "WELL", 2, 10.0, mainBorePerf ) };

    RicScheduleDataGenerator::sortCompletionsMainBoreFirst( { &mainBore }, compdata );

    ASSERT_EQ( 2u, compdata.size() );
    EXPECT_EQ( mainBorePerf, compdata[0].sourcePdmObject() );
    EXPECT_EQ( nullptr, compdata[1].sourcePdmObject() );
}

//--------------------------------------------------------------------------------------------------
/// Several laterals on the same mainbore are ordered depth-first, matching
/// RimWellPath::allWellPathLaterals().
//--------------------------------------------------------------------------------------------------
TEST( RicScheduleDataGeneratorTest, MultipleLateralsAreOrderedDepthFirst )
{
    RimWellPath mainBore;
    RimWellPath lateral1;
    RimWellPath lateral2;
    lateral1.connectWellPaths( &mainBore, 10.0 );
    lateral2.connectWellPaths( &mainBore, 20.0 );

    auto* mainBorePerf = addPerforation( mainBore );
    auto* lateral1Perf = addPerforation( lateral1 );
    auto* lateral2Perf = addPerforation( lateral2 );

    std::vector<RigCompletionData> compdata = { makeCompletion( "WELL", 1, 5.0, lateral2Perf ),
                                                makeCompletion( "WELL", 2, 10.0, lateral1Perf ),
                                                makeCompletion( "WELL", 3, 1.0, mainBorePerf ) };

    RicScheduleDataGenerator::sortCompletionsMainBoreFirst( { &mainBore }, compdata );

    ASSERT_EQ( 3u, compdata.size() );
    EXPECT_EQ( mainBorePerf, compdata[0].sourcePdmObject() );
    EXPECT_EQ( lateral1Perf, compdata[1].sourcePdmObject() );
    EXPECT_EQ( lateral2Perf, compdata[2].sourcePdmObject() );
}
