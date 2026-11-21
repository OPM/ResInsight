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

#include "RimEclipseCase.h"
#include "RimEclipseView.h"

#include "RiuMainWindow.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Removing the dock widget of a view makes another view the current dock widget, which selects it in the
/// project tree. This rebuilds the property editor, which is not safe while the removed view is being
/// destructed. ~Rim3dView therefore blocks view selection while removing its dock widget.
//--------------------------------------------------------------------------------------------------
class RimDockWindowControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
        ASSERT_TRUE( model.eclipseCase != nullptr );

        // The neighbour is the original view, the removed view is the newly created and current one.
        caf::SelectionManager::instance()->setSelectedItem( model.eclipseCase );
        caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewViewFeature" );
        ASSERT_TRUE( feature != nullptr );
        feature->actionTriggered( false );

        const auto views = model.eclipseCase->reservoirViews();
        ASSERT_EQ( 2u, views.size() );
        m_neighbourView = views[0];
        m_removedView   = views[1];

        if ( !m_neighbourView->dockWidget() || !m_removedView->dockWidget() )
        {
            GTEST_SKIP() << "No dock widgets are available for the views";
        }

        caf::SelectionManager::instance()->clearAll();
    }

    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }

    RimEclipseView* m_neighbourView = nullptr;
    RimEclipseView* m_removedView   = nullptr;
};

TEST_F( RimDockWindowControllerTest, RemoveFromDockSelectsNeighbourView )
{
    m_removedView->removeWindowFromDock();

    EXPECT_EQ( m_neighbourView, caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>() );
}

TEST_F( RimDockWindowControllerTest, RemoveFromDockWithBlockedSelectionDoesNotSelectNeighbourView )
{
    m_removedView->removeWindowFromDock( true );

    EXPECT_EQ( nullptr, caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>() );
    EXPECT_FALSE( RiuMainWindow::instance()->isBlockingViewSelectionOnSubWindowActivated() );
}
