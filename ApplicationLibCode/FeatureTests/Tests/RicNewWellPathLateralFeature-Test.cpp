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

#include "RiaFeatureTestModelBuilder.h"

#include "RimProject.h"
#include "RimWellPath.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicNewWellPathLateralFeature, which creates a well path lateral from a selected
/// well path. Requires the parent well path to have a real trajectory (more than two measured
/// depths), which RiaFeatureTestModelBuilder provides.
//--------------------------------------------------------------------------------------------------
class RicNewWellPathLateralFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewWellPathLateralFeatureTest, NewLateralFromSelectedWellPathIsAdded )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::wellPath();
    ASSERT_TRUE( model.wellPath != nullptr );
    ASSERT_EQ( 1u, RimProject::current()->allWellPaths().size() );

    caf::SelectionManager::instance()->setSelectedItem( model.wellPath );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewWellPathLateralFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    // The new lateral is a modeled well path added to the collection.
    EXPECT_EQ( 2u, RimProject::current()->allWellPaths().size() );
}
