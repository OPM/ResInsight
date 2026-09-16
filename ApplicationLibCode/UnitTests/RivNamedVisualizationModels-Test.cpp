/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RivNamedVisualizationModels.h"

#include "cvfModelBasicList.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RivNamedVisualizationModelsTest, FindOrCreateReturnsSameInstanceForSameName )
{
    RivNamedVisualizationModels models;

    auto* modelA = models.findOrCreate( "TestModel" );
    auto* modelB = models.findOrCreate( "TestModel" );

    EXPECT_EQ( modelA, modelB );
    EXPECT_EQ( 1u, models.allModels().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RivNamedVisualizationModelsTest, FindOrCreateCreatesDistinctInstancesForDifferentNames )
{
    RivNamedVisualizationModels models;

    auto* modelA = models.findOrCreate( "TestModelA" );
    auto* modelB = models.findOrCreate( "TestModelB" );

    EXPECT_NE( modelA, modelB );
    EXPECT_EQ( 2u, models.allModels().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RivNamedVisualizationModelsTest, FindOrCreateAndClearRemovesParts )
{
    RivNamedVisualizationModels models;

    auto* model = models.findOrCreate( "TestModel" );
    model->addPart( new cvf::Part );
    EXPECT_EQ( 1u, model->partCount() );

    auto* clearedModel = models.findOrCreateAndClear( "TestModel" );
    EXPECT_EQ( model, clearedModel );
    EXPECT_EQ( 0u, clearedModel->partCount() );
}

//--------------------------------------------------------------------------------------------------
/// The map only keeps the model alive through its own cvf::ref. Verify that clear()/destruction
/// actually releases that reference, rather than e.g. leaking a raw pointer, by keeping an external
/// cvf::ref alive and inspecting the reference count before and after.
//--------------------------------------------------------------------------------------------------
TEST( RivNamedVisualizationModelsTest, ClearReleasesOwnedModels )
{
    RivNamedVisualizationModels models;

    cvf::ref<cvf::ModelBasicList> externalRef = models.findOrCreate( "TestModel" );

    // One reference held by the map's internal storage, one held by externalRef.
    EXPECT_EQ( 2, externalRef->refCount() );

    models.clear();

    // The map has released its reference; only externalRef remains.
    EXPECT_EQ( 1, externalRef->refCount() );
    EXPECT_TRUE( models.allModels().empty() );
}

//--------------------------------------------------------------------------------------------------
/// Verify that destroying the RivNamedVisualizationModels instance itself (going out of scope)
/// releases the reference held on each named model.
//--------------------------------------------------------------------------------------------------
TEST( RivNamedVisualizationModelsTest, DestructorReleasesOwnedModels )
{
    cvf::ref<cvf::ModelBasicList> externalRef;

    {
        RivNamedVisualizationModels models;
        externalRef = models.findOrCreate( "TestModel" );
        EXPECT_EQ( 2, externalRef->refCount() );
    }

    EXPECT_EQ( 1, externalRef->refCount() );
}
