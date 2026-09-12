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

#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicEclipsePropertyFilterNewInViewFeature, which adds an Eclipse property filter
/// to the active view's property-filter collection. The feature resolves its context from the active
/// view (RiaApplication::activeMainOrComparisonGridView()), not the selection.
//--------------------------------------------------------------------------------------------------
class RicEclipsePropertyFilterFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaApplication::instance()->setActiveReservoirView( nullptr );
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicEclipsePropertyFilterFeatureTest, NewPropertyFilterAddedToActiveView )
{
    FeatureTestModel model       = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    auto*            eclipseView = dynamic_cast<RimEclipseView*>( model.eclipseView );
    ASSERT_TRUE( eclipseView != nullptr );

    RimEclipsePropertyFilterCollection* filterCollection = eclipseView->eclipsePropertyFilterCollection();
    ASSERT_TRUE( filterCollection != nullptr );
    const size_t countBefore = filterCollection->propertyFilters().size();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicEclipsePropertyFilterNewInViewFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, filterCollection->propertyFilters().size() );
}
