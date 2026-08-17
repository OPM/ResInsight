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
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonInViewCollection.h"

#include <memory>

namespace
{
bool isPolygonVisible( const RimPolygonInViewCollection& collection, const RimPolygon* polygon )
{
    for ( auto* polygonInView : collection.visiblePolygonsInView() )
    {
        if ( polygonInView->polygon() == polygon ) return polygonInView->showLines();
    }

    return false;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimPolygonInViewCollection, PolygonVisibilityIsIndependentPerView )
{
    auto sourceCollection = std::make_unique<RimPolygonCollection>();

    auto* rootPolygon = new RimPolygon();
    rootPolygon->setName( "Root polygon" );
    sourceCollection->addUserDefinedPolygon( rootPolygon );

    auto* subCollection = new RimPolygonCollection();
    subCollection->setCollectionName( "Folder" );
    sourceCollection->addSubCollection( subCollection );

    auto* nestedPolygon = new RimPolygon();
    nestedPolygon->setName( "Nested polygon" );
    subCollection->addUserDefinedPolygon( nestedPolygon );

    RimPolygonInViewCollection firstViewCollection;
    firstViewCollection.setSourceCollection( sourceCollection.get() );
    firstViewCollection.updateFromPolygonCollection();

    RimPolygonInViewCollection secondViewCollection;
    secondViewCollection.setSourceCollection( sourceCollection.get() );
    secondViewCollection.updateFromPolygonCollection();

    EXPECT_TRUE( isPolygonVisible( firstViewCollection, rootPolygon ) );
    EXPECT_TRUE( isPolygonVisible( firstViewCollection, nestedPolygon ) );
    EXPECT_TRUE( isPolygonVisible( secondViewCollection, nestedPolygon ) );

    EXPECT_TRUE( firstViewCollection.setPolygonVisible( nestedPolygon, false ) );
    EXPECT_TRUE( isPolygonVisible( firstViewCollection, rootPolygon ) );
    EXPECT_FALSE( isPolygonVisible( firstViewCollection, nestedPolygon ) );
    EXPECT_TRUE( isPolygonVisible( secondViewCollection, nestedPolygon ) );

    EXPECT_TRUE( firstViewCollection.setPolygonVisible( nestedPolygon, true ) );
    EXPECT_TRUE( isPolygonVisible( firstViewCollection, nestedPolygon ) );

    auto unrelatedPolygon = std::make_unique<RimPolygon>();
    EXPECT_FALSE( firstViewCollection.setPolygonVisible( unrelatedPolygon.get(), false ) );
    EXPECT_TRUE( isPolygonVisible( firstViewCollection, nestedPolygon ) );
}
