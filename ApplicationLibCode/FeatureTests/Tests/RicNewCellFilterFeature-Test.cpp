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

#include "RiaApplication.h"
#include "RiaFeatureTestModelBuilder.h"

#include "RimCellFilterCollection.h"
#include "RimEclipseView.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicNewPolygonFilter3dviewFeature, which adds a polygon cell filter to the active
/// view's cell-filter collection. The feature resolves its context from the active view.
//--------------------------------------------------------------------------------------------------
class RicNewCellFilterFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaApplication::instance()->setActiveReservoirView( nullptr );
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewCellFilterFeatureTest, NewPolygonFilterAddedToActiveView )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseView != nullptr );

    RimCellFilterCollection* filterCollection = model.eclipseView->cellFilterCollection();
    ASSERT_TRUE( filterCollection != nullptr );
    const size_t countBefore = filterCollection->filters().size();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewPolygonFilter3dviewFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, filterCollection->filters().size() );
}
