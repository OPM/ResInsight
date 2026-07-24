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
#include "RiaFeatureTestTreeView.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// Exercises the RicToggleItems*Feature family against the children of a selected collection.
///
/// These features are tree-driven: with a single object selected they toggle the objectToggleField
/// of that object's tree children (see RicToggleItemsFeatureImpl). Because the feature-test
/// executable has no RiuMainWindow, the tree is supplied by RiaFeatureTestTreeView, which registers a
/// headless project tree view via RiaFeatureCommandContext.
///
/// The well path collection is used as the selected object: its tree children are the well paths,
/// each of which has a boolean objectToggleField (RimWellPath::showWellPath).
//--------------------------------------------------------------------------------------------------
class RicToggleItemsFeatureTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        RiaFeatureTestModelBuilder::wellPath();

        RimOilField* oilField = RimProject::current()->activeOilField();
        m_wellPathCollection  = ( oilField != nullptr ) ? oilField->wellPathCollection() : nullptr;

        if ( m_wellPathCollection != nullptr )
        {
            // A single-child collection is enough, but add a second so the toggle must touch more
            // than one field.
            auto* secondWellPath = new RimWellPath;
            secondWellPath->setName( "TestWellPath2" );
            m_wellPathCollection->addWellPath( secondWellPath );
        }
    }

    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }

    void setShowStateForAllWellPaths( bool show )
    {
        for ( RimWellPath* wellPath : m_wellPathCollection->allWellPaths() )
        {
            wellPath->setShowWellPath( show );
        }
    }

    RimWellPathCollection* m_wellPathCollection = nullptr;
};

TEST_F( RicToggleItemsFeatureTest, ToggleOffHidesAllChildWellPaths )
{
    ASSERT_TRUE( m_wellPathCollection != nullptr );
    ASSERT_EQ( 2u, m_wellPathCollection->allWellPaths().size() );
    setShowStateForAllWellPaths( true );

    RiaFeatureTestTreeView treeView;
    caf::SelectionManager::instance()->setSelectedItem( m_wellPathCollection );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicToggleItemsOffFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    for ( RimWellPath* wellPath : m_wellPathCollection->allWellPaths() )
    {
        EXPECT_FALSE( wellPath->showWellPath() );
    }
}

TEST_F( RicToggleItemsFeatureTest, ToggleOnShowsAllChildWellPaths )
{
    ASSERT_TRUE( m_wellPathCollection != nullptr );
    setShowStateForAllWellPaths( false );

    RiaFeatureTestTreeView treeView;
    caf::SelectionManager::instance()->setSelectedItem( m_wellPathCollection );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicToggleItemsOnFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    for ( RimWellPath* wellPath : m_wellPathCollection->allWellPaths() )
    {
        EXPECT_TRUE( wellPath->showWellPath() );
    }
}

TEST_F( RicToggleItemsFeatureTest, ToggleFlipsEachChildShowState )
{
    ASSERT_TRUE( m_wellPathCollection != nullptr );

    const std::vector<RimWellPath*> wellPaths = m_wellPathCollection->allWellPaths();
    ASSERT_EQ( 2u, wellPaths.size() );

    // Give the two well paths opposite states, then assert each is individually flipped.
    wellPaths[0]->setShowWellPath( true );
    wellPaths[1]->setShowWellPath( false );

    RiaFeatureTestTreeView treeView;
    caf::SelectionManager::instance()->setSelectedItem( m_wellPathCollection );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicToggleItemsFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_FALSE( wellPaths[0]->showWellPath() );
    EXPECT_TRUE( wellPaths[1]->showWellPath() );
}
