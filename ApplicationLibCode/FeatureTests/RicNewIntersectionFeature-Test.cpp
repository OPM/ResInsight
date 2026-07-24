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

#include "RimEclipseView.h"
#include "RimIntersectionCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated tests for the intersection-creation features that read the active 3D view rather than the
/// selection (RiaApplication::activeReservoirView() / activeGridView()).
///
/// These exercise the active-view path of the harness: RiaFeatureTestModelBuilder sets the active
/// reservoir view when it builds an Eclipse model, so features that resolve their context from the
/// active view have a valid context without a RiuMainWindow.
//--------------------------------------------------------------------------------------------------
class RicNewIntersectionFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaApplication::instance()->setActiveReservoirView( nullptr );
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicNewIntersectionFeatureTest, BuilderActivatesReservoirView )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseView != nullptr );

    // The builder must publish the view as the active reservoir view so active-view features work.
    EXPECT_EQ( model.eclipseView, RiaApplication::instance()->activeReservoirView() );
}

TEST_F( RicNewIntersectionFeatureTest, AzimuthDipIntersectionAddedToActiveViewCollection )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseView != nullptr );

    const size_t countBefore = model.eclipseView->intersectionCollection()->intersections().size();

    // No selection is set: the feature must resolve the view from RiaApplication::activeGridView().
    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewAzimuthDipIntersectionFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, model.eclipseView->intersectionCollection()->intersections().size() );
}

TEST_F( RicNewIntersectionFeatureTest, PolylineIntersectionAddedToActiveViewCollection )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseView != nullptr );

    const size_t countBefore = model.eclipseView->intersectionCollection()->intersections().size();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewPolylineIntersectionFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, model.eclipseView->intersectionCollection()->intersections().size() );
}
