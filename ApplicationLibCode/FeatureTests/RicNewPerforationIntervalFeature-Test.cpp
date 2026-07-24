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

#include "RimPerforationCollection.h"
#include "RimWellPath.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicNewPerforationIntervalFeature, which appends a perforation interval to the
/// perforation collection of the selected well path. The selected well path resolves to its
/// perforation collection, and its unit system (set by the builder) avoids a unit-system dialog.
//--------------------------------------------------------------------------------------------------
class RicNewPerforationIntervalFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewPerforationIntervalFeatureTest, NewPerforationIntervalAddedToSelectedWellPath )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::wellPath();
    ASSERT_TRUE( model.wellPath != nullptr );

    RimPerforationCollection* perforationCollection = model.wellPath->perforationIntervalCollection();
    ASSERT_TRUE( perforationCollection != nullptr );
    const size_t countBefore = perforationCollection->perforations().size();

    caf::SelectionManager::instance()->setSelectedItem( model.wellPath );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewPerforationIntervalFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, perforationCollection->perforations().size() );
}
