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

#include "RimRegularSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInViewCollection.h"

#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSurfaceInViewCollection, SurfaceSettingsAreIndependentPerView )
{
    auto sourceCollection = std::make_unique<RimSurfaceCollection>();

    auto* subCollection = new RimSurfaceCollection();
    subCollection->setCollectionName( "Folder" );
    sourceCollection->addSubCollection( subCollection );

    auto* surface = new RimRegularSurface();
    surface->setUserDescription( "Surface" );
    surface->setNx( 2 );
    surface->setNy( 2 );
    surface->setProperty( "Property A", { 1.0f, 2.0f, 3.0f, 4.0f } );
    surface->setProperty( "Property B", { 5.0f, 6.0f, 7.0f, 8.0f } );
    surface->onLoadData();
    subCollection->addSurface( surface );

    RimSurfaceInViewCollection firstViewCollection;
    firstViewCollection.setSurfaceCollection( sourceCollection.get() );
    firstViewCollection.updateFromSurfaceCollection();

    RimSurfaceInViewCollection secondViewCollection;
    secondViewCollection.setSurfaceCollection( sourceCollection.get() );
    secondViewCollection.updateFromSurfaceCollection();

    EXPECT_TRUE( firstViewCollection.isSurfaceVisible( surface ) );
    EXPECT_TRUE( secondViewCollection.isSurfaceVisible( surface ) );
    EXPECT_EQ( QString( "Property A" ), firstViewCollection.surfaceProperty( surface ) );
    EXPECT_EQ( QString( "Property A" ), secondViewCollection.surfaceProperty( surface ) );

    EXPECT_TRUE( firstViewCollection.setSurfaceVisible( surface, false ) );
    EXPECT_FALSE( firstViewCollection.isSurfaceVisible( surface ) );
    EXPECT_TRUE( secondViewCollection.isSurfaceVisible( surface ) );

    auto propertyResult = firstViewCollection.setSurfaceProperty( surface, "Property B" );
    ASSERT_TRUE( propertyResult.has_value() );
    EXPECT_EQ( QString( "Property B" ), firstViewCollection.surfaceProperty( surface ) );
    EXPECT_EQ( QString( "Property A" ), secondViewCollection.surfaceProperty( surface ) );

    auto invalidPropertyResult = firstViewCollection.setSurfaceProperty( surface, "Missing" );
    EXPECT_FALSE( invalidPropertyResult.has_value() );
    EXPECT_EQ( QString( "Property B" ), firstViewCollection.surfaceProperty( surface ) );

    auto unrelatedSurface = std::make_unique<RimRegularSurface>();
    EXPECT_FALSE( firstViewCollection.setSurfaceVisible( unrelatedSurface.get(), false ) );
    EXPECT_FALSE( firstViewCollection.setSurfaceProperty( unrelatedSurface.get(), "Property A" ).has_value() );
}
