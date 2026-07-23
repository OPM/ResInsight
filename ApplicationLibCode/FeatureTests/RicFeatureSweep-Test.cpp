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

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafFactory.h"
#include "cafSelectionManager.h"

#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// The command ids of all registered Ric*Feature commands.
///
/// gtest evaluates INSTANTIATE_TEST_SUITE_P generators lazily (via the generated _EvalGenerator_
/// function) at the start of RUN_ALL_TESTS, i.e. after the singletons have been created in main().
/// The factory is therefore fully populated when this is called, so there is no static-init-order
/// problem with reading it here.
//--------------------------------------------------------------------------------------------------
static std::vector<std::string> allRegisteredFeatureIds()
{
    return caf::Factory<caf::CmdFeature, std::string>::instance()->allKeys();
}

//--------------------------------------------------------------------------------------------------
/// All Ric*Feature command ids are registered and were instantiated by the CmdFeatureManager.
///
/// This is the M1 de-risk test: booting the feature-test executable creates a RiaGuiApplication
/// (offscreen on CI) and a caf::CmdFeatureManager, whose constructor instantiates every feature
/// registered via CAF_CMD_SOURCE_INIT. Reaching this test therefore already proves the full set of
/// features survives linking into this target and can be constructed. Here we assert the expected
/// number of registrations is present.
///
/// Note: this test deliberately does not call getCommandFeature()/refreshEnabledState() for every
/// feature, because that path invokes isCommandEnabled() with an empty selection. That behaviour is
/// exercised separately and per-feature by FeatureEnabledState.* so that a crash in one feature does
/// not mask the rest.
//--------------------------------------------------------------------------------------------------
TEST( FeatureSweep, AllFeaturesRegistered )
{
    auto* factory = caf::Factory<caf::CmdFeature, std::string>::instance();
    ASSERT_TRUE( factory != nullptr );

    std::vector<std::string> commandIds = factory->allKeys();
    EXPECT_GT( commandIds.size(), 400u ) << "Expected the full set of Ric*Feature commands to be registered";

    ASSERT_TRUE( caf::CmdFeatureManager::instance() != nullptr );
}

//--------------------------------------------------------------------------------------------------
/// Querying a feature's enabled state with an empty selection must not crash.
///
/// isCommandEnabled() is called by the framework whenever it refreshes actions (menus, toolbars,
/// keyboard-shortcut dispatch). Many features read the current selection to decide; if they
/// dereference the result of caf::SelectionManager queries without a null check, they crash when
/// nothing (or the wrong type) is selected. This parameterized test runs one case per feature so a
/// crash in one feature is reported against that feature id and does not hide the others.
//--------------------------------------------------------------------------------------------------
class FeatureEnabledStateTest : public ::testing::TestWithParam<std::string>
{
protected:
    void SetUp() override { caf::SelectionManager::instance()->clearAll(); }
    void TearDown() override { caf::SelectionManager::instance()->clearAll(); }
};

TEST_P( FeatureEnabledStateTest, EmptySelectionDoesNotCrash )
{
    const std::string& commandId = GetParam();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( commandId );
    ASSERT_TRUE( feature != nullptr );

    // Must not crash / must not throw with an empty selection.
    EXPECT_NO_FATAL_FAILURE( (void)feature->canFeatureBeExecuted() );
}

INSTANTIATE_TEST_SUITE_P( AllFeatures,
                          FeatureEnabledStateTest,
                          ::testing::ValuesIn( allRegisteredFeatureIds() ),
                          []( const testing::TestParamInfo<std::string>& info ) { return info.param; } );
