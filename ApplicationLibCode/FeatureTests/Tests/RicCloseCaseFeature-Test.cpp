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
#include "RimProject.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Executes RicCloseCaseFeature against a selected Eclipse case and asserts the case is removed from
/// the project.
///
/// This is a curated deep test: it drives a feature end-to-end through the selection system with a
/// valid selection context and asserts the resulting model change.
//--------------------------------------------------------------------------------------------------
class RicCloseCaseFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicCloseCaseFeatureTest, CloseSelectedEclipseCaseRemovesItFromProject )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseCase != nullptr );
    ASSERT_EQ( 1u, RimProject::current()->eclipseCases().size() );

    caf::SelectionManager::instance()->setSelectedItem( model.eclipseCase );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicCloseCaseFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    // The case (and its views) are deleted, so model.eclipseCase is now dangling; query the project.
    EXPECT_TRUE( RimProject::current()->eclipseCases().empty() );
}
