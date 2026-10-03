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

#include "RimGridView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "WellPath/RimWellPathInView.h"
#include "WellPath/RimWellPathInViewCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// Exercises the RicToggleItems*Feature family against the children of a selected collection.
///
/// These features are tree-driven: with a single object selected they toggle the objectToggleField
/// of that object's tree children (see RicToggleItemsFeatureImpl). Because the feature-test
/// main window project tree is not populated with the model built here, the tree is supplied by
/// RiaFeatureTestTreeView, which registers a
/// headless project tree view via RiaFeatureCommandContext.
///
/// Well path visibility lives per-view: the per-view RimWellPathInViewCollection mirrors the
/// global well path collection, and its RimWellPathInView items have the checkable
/// objectToggleField. The global well path collection no longer exposes a per-well toggle (see
/// the removal of RimWellPath::objectToggleField()), so the well path in-view collection under
/// the Eclipse view is used as the selected object here.
//--------------------------------------------------------------------------------------------------
class RicToggleItemsFeatureTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        FeatureTestModel model = RiaFeatureTestModelBuilder::combinedModel();
        m_eclipseView          = model.eclipseView;

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

        m_wellPathInViewCollection = m_eclipseView ? m_eclipseView->wellPathInViewCollection() : nullptr;
    }

    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }

    void setCheckedStateForAllWellPathsInView( bool checked )
    {
        for ( RimWellPathInView* wellPathInView : m_wellPathInViewCollection->allWellPathsInView() )
        {
            wellPathInView->setCheckState( checked );
        }
    }

    RimGridView*                 m_eclipseView              = nullptr;
    RimWellPathCollection*       m_wellPathCollection       = nullptr;
    RimWellPathInViewCollection* m_wellPathInViewCollection = nullptr;
};

TEST_F( RicToggleItemsFeatureTest, ToggleOffHidesAllChildWellPaths )
{
    ASSERT_TRUE( m_wellPathInViewCollection != nullptr );
    ASSERT_EQ( 2u, m_wellPathInViewCollection->allWellPathsInView().size() );
    setCheckedStateForAllWellPathsInView( true );

    RiaFeatureTestTreeView treeView;
    caf::SelectionManager::instance()->setSelectedItem( m_wellPathInViewCollection );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicToggleItemsOffFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    for ( RimWellPathInView* wellPathInView : m_wellPathInViewCollection->allWellPathsInView() )
    {
        EXPECT_FALSE( wellPathInView->isChecked() );
    }
}

TEST_F( RicToggleItemsFeatureTest, ToggleOnShowsAllChildWellPaths )
{
    ASSERT_TRUE( m_wellPathInViewCollection != nullptr );
    setCheckedStateForAllWellPathsInView( false );

    RiaFeatureTestTreeView treeView;
    caf::SelectionManager::instance()->setSelectedItem( m_wellPathInViewCollection );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicToggleItemsOnFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    for ( RimWellPathInView* wellPathInView : m_wellPathInViewCollection->allWellPathsInView() )
    {
        EXPECT_TRUE( wellPathInView->isChecked() );
    }
}

TEST_F( RicToggleItemsFeatureTest, ToggleFlipsEachChildShowState )
{
    ASSERT_TRUE( m_wellPathInViewCollection != nullptr );

    const std::vector<RimWellPathInView*> wellPathsInView = m_wellPathInViewCollection->allWellPathsInView();
    ASSERT_EQ( 2u, wellPathsInView.size() );

    // Give the two well paths opposite states, then assert each is individually flipped.
    wellPathsInView[0]->setCheckState( true );
    wellPathsInView[1]->setCheckState( false );

    RiaFeatureTestTreeView treeView;
    caf::SelectionManager::instance()->setSelectedItem( m_wellPathInViewCollection );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicToggleItemsFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_FALSE( wellPathsInView[0]->isChecked() );
    EXPECT_TRUE( wellPathsInView[1]->isChecked() );
}
