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

#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicNewWellLogPlotFeature, which creates a new well log plot (with a track and an
/// extraction curve) in the well log plot collection. The feature needs no selection.
//--------------------------------------------------------------------------------------------------
class RicNewWellLogPlotFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewWellLogPlotFeatureTest, NewWellLogPlotIsAddedToCollection )
{
    // A well path (with geometry) gives the extraction curve something to reference.
    FeatureTestModel model = RiaFeatureTestModelBuilder::combinedModel();
    ASSERT_TRUE( model.eclipseView != nullptr );

    RimWellLogPlotCollection* plotCollection = RimMainPlotCollection::current()->wellLogPlotCollection();
    ASSERT_TRUE( plotCollection != nullptr );
    const size_t countBefore = plotCollection->wellLogPlots().size();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewWellLogPlotFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, plotCollection->wellLogPlots().size() );
}
