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

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Executes RicNewViewFeature against a selected Eclipse case and asserts that a new reservoir view
/// is added to the case.
///
/// This is a curated deep test: it drives a feature end-to-end through the selection system with a
/// valid selection context and asserts the resulting model change.
//--------------------------------------------------------------------------------------------------
class RicNewViewFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewViewFeatureTest, NewViewFromSelectedEclipseCaseAddsAView )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseCase != nullptr );

    const size_t viewCountBefore = model.eclipseCase->reservoirViews().size();

    caf::SelectionManager::instance()->setSelectedItem( model.eclipseCase );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewViewFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( viewCountBefore + 1, model.eclipseCase->reservoirViews().size() );
}
