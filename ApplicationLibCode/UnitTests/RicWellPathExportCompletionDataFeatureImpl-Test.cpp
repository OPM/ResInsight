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

#include "Commands/CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "RigCompletionData.h"

#include <algorithm>
#include <optional>

namespace
{
RigCompletionData makeCompletion( double startMD, double endMD, std::optional<int> completionNumber )
{
    RigCompletionData completion( "W", RigCompletionDataGridCell( 0, nullptr ), startMD );
    completion.setDepthRange( startMD, endMD );
    if ( completionNumber ) completion.setCompletionNumber( *completionNumber );
    return completion;
}

std::optional<int> numberForAllOrders( std::vector<RigCompletionData> completions )
{
    std::vector<size_t> order( completions.size() );
    for ( size_t i = 0; i < order.size(); ++i )
        order[i] = i;

    std::optional<int> first;
    bool               firstSet = false;
    do
    {
        std::vector<RigCompletionData> permuted;
        for ( size_t i : order )
            permuted.push_back( completions[i] );

        auto number = RicWellPathExportCompletionDataFeatureImpl::completionNumberForCell( permuted );
        if ( !firstSet )
        {
            first    = number;
            firstSet = true;
        }
        EXPECT_EQ( first, number );
    } while ( std::next_permutation( order.begin(), order.end() ) );

    return first;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A cell cut by two perforation intervals gets the number of the interval with the largest overlap,
/// regardless of the order the contributions arrive in.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberUsesLargestOverlap )
{
    EXPECT_EQ( 2, numberForAllOrders( { makeCompletion( 100.0, 101.0, 1 ), makeCompletion( 101.0, 105.0, 2 ) } ) );
    EXPECT_EQ( 1, numberForAllOrders( { makeCompletion( 100.0, 104.0, 1 ), makeCompletion( 104.0, 105.0, 2 ) } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberTieGoesToLowestNumber )
{
    EXPECT_EQ( 3, numberForAllOrders( { makeCompletion( 100.0, 102.0, 5 ), makeCompletion( 102.0, 104.0, 3 ) } ) );
}

//--------------------------------------------------------------------------------------------------
/// The same interval can cross a cell more than once; its lengths are summed.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberSumsOverlapPerNumber )
{
    EXPECT_EQ( 1,
               numberForAllOrders(
                   { makeCompletion( 100.0, 102.0, 1 ), makeCompletion( 102.0, 105.0, 2 ), makeCompletion( 110.0, 112.0, 1 ) } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberIgnoresContributionsWithoutNumber )
{
    EXPECT_EQ( 4, numberForAllOrders( { makeCompletion( 100.0, 110.0, std::nullopt ), makeCompletion( 110.0, 111.0, 4 ) } ) );
    EXPECT_EQ( std::nullopt, numberForAllOrders( { makeCompletion( 100.0, 110.0, std::nullopt ) } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportCompletionDataFeatureImpl, CompletionNumberEmptyCompletionsGivesNoNumber )
{
    EXPECT_EQ( std::nullopt, RicWellPathExportCompletionDataFeatureImpl::completionNumberForCell( {} ) );
}
