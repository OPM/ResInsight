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

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Executes RicDeleteWellPathFeature against a selected well path and asserts the well path is
/// removed from the project and dropped from the selection.
///
/// This is a curated deep test: it drives a feature end-to-end through the selection system with a
/// valid selection context and asserts the resulting model change.
//--------------------------------------------------------------------------------------------------
class RicDeleteWellPathFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicDeleteWellPathFeatureTest, DeleteSelectedWellPathRemovesItFromProject )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::wellPath();
    ASSERT_TRUE( model.wellPath != nullptr );
    ASSERT_EQ( 1u, RimProject::current()->allWellPaths().size() );

    caf::SelectionManager::instance()->setSelectedItem( model.wellPath );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicDeleteWellPathFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( 0u, RimProject::current()->allWellPaths().size() );

    // The deleted object must no longer be part of the selection (dangling-pointer cleanup).
    EXPECT_TRUE( caf::SelectionManager::instance()->selectedItems().empty() );
}

TEST_F( RicDeleteWellPathFeatureTest, DeleteSelectedWellPathKeepsOtherWellPaths )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::wellPath();
    ASSERT_TRUE( model.wellPath != nullptr );

    // Add a second well path so the feature must delete only the selected one.
    RimOilField* oilField = RimProject::current()->activeOilField();
    ASSERT_TRUE( oilField != nullptr && oilField->wellPathCollection() != nullptr );

    auto* otherWellPath = new RimWellPath;
    otherWellPath->setName( "OtherWellPath" );
    oilField->wellPathCollection()->addWellPath( otherWellPath );

    ASSERT_EQ( 2u, RimProject::current()->allWellPaths().size() );

    caf::SelectionManager::instance()->setSelectedItem( model.wellPath );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicDeleteWellPathFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    const std::vector<RimWellPath*> remaining = RimProject::current()->allWellPaths();
    ASSERT_EQ( 1u, remaining.size() );
    EXPECT_EQ( otherWellPath, remaining.front() );
}

TEST_F( RicDeleteWellPathFeatureTest, EmptySelectionIsNotExecutable )
{
    RiaFeatureTestModelBuilder::wellPath();
    caf::SelectionManager::instance()->clearAll();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicDeleteWellPathFeature" );
    ASSERT_TRUE( feature != nullptr );

    // With nothing selected the feature must report itself disabled and leave the project untouched.
    EXPECT_FALSE( feature->canFeatureBeExecuted() );
    EXPECT_EQ( 1u, RimProject::current()->allWellPaths().size() );
}
