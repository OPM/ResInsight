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

#include "RicFeatureExecutionRunner.h"
#include "RicFeatureSweepDenylist.h"

#include "RimProject.h"

#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Executes each feature in this process against synthetic selections, asserting that execution does
/// not crash and leaves a valid project.
///
/// DISABLED by default, and it cannot be made a gate. Many features assume the framework only invokes
/// them in a valid GUI selection context (e.g. a right-click on a specific object). Executing them
/// with a synthetic selection can violate that assumption and trip a CVF_ASSERT / CAF_ASSERT, which
/// aborts the process rather than throwing - so a single offender ends the whole run and hides every
/// feature after it.
///
/// Prefer FeatureSubprocessExecutionTest (RicFeatureSubprocessSweep-Test.cpp), which runs each
/// feature in its own child process and therefore isolates aborts and hangs. This in-process variant
/// is kept because it is much faster when you already know the feature under test is well behaved:
///   --gtest_also_run_disabled_tests --gtest_filter=*FeatureExecutionTest*
//--------------------------------------------------------------------------------------------------
class FeatureExecutionTest : public ::testing::TestWithParam<std::string>
{
public:
    static void TearDownTestSuite() { RicFeatureExecutionRunner::releaseModel(); }
};

TEST_P( FeatureExecutionTest, DISABLED_ExecuteWhenEnabledDoesNotCrash )
{
    const std::string& commandId = GetParam();

    if ( isFeatureDenylisted( commandId ) )
    {
        GTEST_SKIP() << "Denylisted: " << featureSweepDenylist().at( commandId );
    }

    EXPECT_TRUE( RicFeatureExecutionRunner::executeFeature( commandId ) ) << commandId << " left the project in an invalid state";

    EXPECT_TRUE( RimProject::current() != nullptr );
}

INSTANTIATE_TEST_SUITE_P( AllFeatures,
                          FeatureExecutionTest,
                          ::testing::ValuesIn( RicFeatureExecutionRunner::allRegisteredFeatureIds() ),
                          []( const testing::TestParamInfo<std::string>& info ) { return info.param; } );
