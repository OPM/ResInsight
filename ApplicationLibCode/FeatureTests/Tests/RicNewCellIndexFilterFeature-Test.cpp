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
#include "RimGeoMechCase.h"
#include "RimGridView.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicNewCellIndexFilterFeature ("New Element Set Filter"), which is only enabled
/// when the active grid view belongs to a GeoMech case. It adds a cell index filter to that view's
/// cell filter collection.
///
/// The GeoMech model is the small VTK (.pvd) model from the unit test data, so no Abaqus/ODB
/// dependency is needed.
//--------------------------------------------------------------------------------------------------
class RicNewCellIndexFilterFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaApplication::instance()->setActiveReservoirView( nullptr );
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewCellIndexFilterFeatureTest, NewCellIndexFilterAddedToActiveGeoMechView )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::geoMechCase();
    ASSERT_TRUE( model.geoMechCase != nullptr );
    ASSERT_TRUE( model.geoMechView != nullptr );

    RimCellFilterCollection* filterCollection = model.geoMechView->cellFilterCollection();
    ASSERT_TRUE( filterCollection != nullptr );
    const size_t countBefore = filterCollection->filters().size();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewCellIndexFilterFeature" );
    ASSERT_TRUE( feature != nullptr );

    // Enabled only because the active view's owner case is a GeoMech case.
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, filterCollection->filters().size() );
}

TEST_F( RicNewCellIndexFilterFeatureTest, NotExecutableForEclipseView )
{
    // The same feature must stay disabled when the active view belongs to an Eclipse case.
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseView != nullptr );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewCellIndexFilterFeature" );
    ASSERT_TRUE( feature != nullptr );

    EXPECT_FALSE( feature->canFeatureBeExecuted() );
}
