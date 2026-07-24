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
#include "RimSummaryCase.h"
#include "RimSummaryMultiPlotCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicNewDefaultSummaryPlotFeature, which creates a summary multi plot from a
/// selected summary case. Uses an in-memory RimMockSummaryCase (no .SMSPEC file).
//--------------------------------------------------------------------------------------------------
class RicNewDefaultSummaryPlotFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewDefaultSummaryPlotFeatureTest, NewDefaultSummaryPlotFromSelectedCaseIsAdded )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::summaryCase();
    ASSERT_TRUE( model.summaryCase != nullptr );

    RimSummaryMultiPlotCollection* plotCollection = RimMainPlotCollection::current()->summaryMultiPlotCollection();
    ASSERT_TRUE( plotCollection != nullptr );
    const size_t countBefore = plotCollection->multiPlots().size();

    caf::SelectionManager::instance()->setSelectedItem( model.summaryCase );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewDefaultSummaryPlotFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, plotCollection->multiPlots().size() );
}
