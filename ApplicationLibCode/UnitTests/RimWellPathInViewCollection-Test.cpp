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

#include "WellPath/RimWellPath.h"
#include "WellPath/RimWellPathCollection.h"
#include "WellPath/RimWellPathInViewCollection.h"

#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWellPathInViewCollection, WellPathVisibilityIsIndependentPerView )
{
    auto sourceCollection = std::make_unique<RimWellPathCollection>();

    auto* firstWellPath = new RimWellPath();
    firstWellPath->setName( "A" );
    sourceCollection->addWellPath( firstWellPath );

    auto* secondWellPath = new RimWellPath();
    secondWellPath->setName( "B" );
    sourceCollection->addWellPath( secondWellPath );

    RimWellPathInViewCollection firstViewCollection;
    firstViewCollection.setSourceCollection( sourceCollection.get() );
    firstViewCollection.updateFromWellPathCollection();

    RimWellPathInViewCollection secondViewCollection;
    secondViewCollection.setSourceCollection( sourceCollection.get() );
    secondViewCollection.updateFromWellPathCollection();

    EXPECT_EQ( 2u, firstViewCollection.allWellPathsInView().size() );
    EXPECT_TRUE( firstViewCollection.isWellPathVisible( firstWellPath ) );
    EXPECT_TRUE( secondViewCollection.isWellPathVisible( firstWellPath ) );

    EXPECT_TRUE( firstViewCollection.setWellPathVisible( firstWellPath, false ) );
    EXPECT_FALSE( firstViewCollection.isWellPathVisible( firstWellPath ) );
    EXPECT_TRUE( firstViewCollection.isWellPathVisible( secondWellPath ) );
    EXPECT_TRUE( secondViewCollection.isWellPathVisible( firstWellPath ) );
    EXPECT_EQ( 1u, firstViewCollection.visibleWellPathsInView().size() );

    firstViewCollection.setCheckState( false );
    EXPECT_FALSE( firstViewCollection.isWellPathVisible( secondWellPath ) );
    EXPECT_TRUE( firstViewCollection.visibleWellPathsInView().empty() );
    EXPECT_TRUE( secondViewCollection.isWellPathVisible( secondWellPath ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWellPathInViewCollection, SyncFollowsSourceCollection )
{
    auto sourceCollection = std::make_unique<RimWellPathCollection>();

    auto* firstWellPath = new RimWellPath();
    firstWellPath->setName( "A" );
    sourceCollection->addWellPath( firstWellPath );

    RimWellPathInViewCollection viewCollection;
    viewCollection.setSourceCollection( sourceCollection.get() );
    viewCollection.updateFromWellPathCollection();
    ASSERT_EQ( 1u, viewCollection.allWellPathsInView().size() );

    auto* secondWellPath = new RimWellPath();
    secondWellPath->setName( "B" );
    sourceCollection->addWellPath( secondWellPath );
    viewCollection.updateFromWellPathCollection();
    ASSERT_EQ( 2u, viewCollection.allWellPathsInView().size() );

    EXPECT_TRUE( viewCollection.setWellPathVisible( secondWellPath, false ) );

    sourceCollection->deleteWell( firstWellPath );
    viewCollection.updateFromWellPathCollection();

    ASSERT_EQ( 1u, viewCollection.allWellPathsInView().size() );
    EXPECT_EQ( secondWellPath, viewCollection.allWellPathsInView().front()->wellPath() );
    EXPECT_FALSE( viewCollection.isWellPathVisible( secondWellPath ) );
}
