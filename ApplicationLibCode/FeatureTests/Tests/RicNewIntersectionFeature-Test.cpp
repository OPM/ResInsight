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
#include "RimExtrudedCurveIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimTools.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonInView.h"
#include "Polygons/RimPolygonInViewCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// Curated tests for the intersection-creation features that read the active 3D view rather than the
/// selection (RiaApplication::activeReservoirView() / activeGridView()).
///
/// These exercise the active-view path of the harness: RiaFeatureTestModelBuilder sets the active
/// reservoir view when it builds an Eclipse model, so features that resolve their context from the
/// active view have a valid context without depending on the project tree selection.
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

TEST_F( RicNewIntersectionFeatureTest, PolygonIntersectionCreatesPolygonInEditMode )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseView != nullptr );

    RimPolygonCollection* polygonCollection = RimTools::polygonCollection();
    ASSERT_TRUE( polygonCollection != nullptr );

    const size_t intersectionCountBefore = model.eclipseView->intersectionCollection()->intersections().size();
    const auto   polygonsBefore          = polygonCollection->allPolygons();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewPolygonIntersectionFromPickingFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    auto intersections = model.eclipseView->intersectionCollection()->intersections();
    ASSERT_EQ( intersectionCountBefore + 1, intersections.size() );

    RimPolygon* polygon = nullptr;
    for ( auto candidate : polygonCollection->allPolygons() )
    {
        if ( std::find( polygonsBefore.begin(), polygonsBefore.end(), candidate ) == polygonsBefore.end() )
        {
            ASSERT_TRUE( polygon == nullptr ) << "Expected exactly one new polygon";
            polygon = candidate;
        }
    }
    ASSERT_TRUE( polygon != nullptr );

    RimExtrudedCurveIntersection* intersection = intersections.back();
    EXPECT_EQ( RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYGON, intersection->type() );
    EXPECT_FALSE( polygon->isClosed() );
    EXPECT_TRUE( polygon->name().startsWith( "Polyline" ) );

    // The polygon in the view must be in edit mode to receive points picked in the 3D view
    RimPolygonInView* polygonInView = nullptr;
    for ( auto candidate : model.eclipseView->polygonInViewCollection()->allPolygonsInView() )
    {
        if ( candidate->polygon() == polygon ) polygonInView = candidate;
    }
    ASSERT_TRUE( polygonInView != nullptr );
    EXPECT_TRUE( polygonInView->pickingEnabled() );

    // The intersection geometry follows the points of the polygon
    polygon->setPointsInDomainCoords( { { 0.0, 0.0, -1000.0 }, { 100.0, 0.0, -1000.0 }, { 100.0, 100.0, -1000.0 } } );
    const auto polyLines = intersection->polyLines();
    ASSERT_FALSE( polyLines.empty() );
    EXPECT_FALSE( polyLines.front().empty() );
}

TEST_F( RicNewIntersectionFeatureTest, PolygonIntersectionUsesSelectedIntersectionCollection )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::eclipseCaseWithResults();
    ASSERT_TRUE( model.eclipseView != nullptr );

    RimIntersectionCollection* collection  = model.eclipseView->intersectionCollection();
    const size_t               countBefore = collection->intersections().size();

    // Triggered from the project tree: the selected collection is used, independent of the active view
    RiaApplication::instance()->setActiveReservoirView( nullptr );
    caf::SelectionManager::instance()->setSelectedItem( collection );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicNewPolygonIntersectionFromPickingFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, collection->intersections().size() );
}