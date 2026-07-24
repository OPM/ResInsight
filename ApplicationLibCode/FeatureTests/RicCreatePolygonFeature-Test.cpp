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

#include "Polygons/RimPolygonCollection.h"
#include "RimTools.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// Curated test for RicCreatePolygonFeature, which appends a user-defined polygon to the project's
/// polygon collection. The feature falls back to the global polygon collection when nothing is
/// selected, so no selection or active view is required.
//--------------------------------------------------------------------------------------------------
class RicCreatePolygonFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }
};

TEST_F( RicCreatePolygonFeatureTest, CreatePolygonAddedToCollection )
{
    // Start from a clean, empty project; the polygon collection exists on the active oil field.
    RiaFeatureTestModelBuilder::closeProject();

    RimPolygonCollection* polygonCollection = RimTools::polygonCollection();
    ASSERT_TRUE( polygonCollection != nullptr );
    const size_t countBefore = polygonCollection->allPolygons().size();

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicCreatePolygonFeature" );
    ASSERT_TRUE( feature != nullptr );
    ASSERT_TRUE( feature->canFeatureBeExecuted() );

    feature->actionTriggered( false );

    EXPECT_EQ( countBefore + 1, polygonCollection->allPolygons().size() );
}
