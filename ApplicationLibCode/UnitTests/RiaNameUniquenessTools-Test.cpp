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

#include "RiaNameUniquenessTools.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimRegularSurface.h"
#include "RimSurfaceCollection.h"

#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaNameUniquenessTools, MakeUniqueAppendsNumericSuffix )
{
    EXPECT_EQ( QString( "Top" ), RiaNameUniquenessTools::makeUnique( "Top", {} ) );
    EXPECT_EQ( QString( "Top_1" ), RiaNameUniquenessTools::makeUnique( "Top", { "Top" } ) );
    EXPECT_EQ( QString( "Top_2" ), RiaNameUniquenessTools::makeUnique( "Top", { "Top", "Top_1" } ) );
    EXPECT_EQ( QString( "Top_3" ), RiaNameUniquenessTools::makeUnique( "Top", { "Top", "Top_1", "Top_2" } ) );

    // A gap in the numbering is filled
    EXPECT_EQ( QString( "Top_1" ), RiaNameUniquenessTools::makeUnique( "Top", { "Top", "Top_2" } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaNameUniquenessTools, MakeUniqueIsCaseSensitive )
{
    EXPECT_EQ( QString( "fault" ), RiaNameUniquenessTools::makeUnique( "fault", { "Fault" } ) );
    EXPECT_EQ( QString( "Fault_1" ), RiaNameUniquenessTools::makeUnique( "Fault", { "Fault", "fault" } ) );
}

//--------------------------------------------------------------------------------------------------
/// An empty name means the object derives its tree label from other properties, and must not be
/// numbered.
//--------------------------------------------------------------------------------------------------
TEST( RiaNameUniquenessTools, MakeUniqueLeavesEmptyNameUntouched )
{
    EXPECT_EQ( QString( "" ), RiaNameUniquenessTools::makeUnique( "", { "", "Top" } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaNameUniquenessTools, SurfacesAreUniquePerFolder )
{
    auto rootCollection = std::make_unique<RimSurfaceCollection>();

    auto* firstSurface = new RimRegularSurface();
    firstSurface->setUserDescription( "Top" );
    rootCollection->addSurface( firstSurface );

    auto* secondSurface = new RimRegularSurface();
    secondSurface->setUserDescription( "Top" );
    rootCollection->addSurface( secondSurface );

    EXPECT_EQ( QString( "Top" ), firstSurface->userDescription() );
    EXPECT_EQ( QString( "Top_1" ), secondSurface->userDescription() );

    // The same name is free again in another folder
    auto* subFolder = new RimSurfaceCollection();
    rootCollection->addSubCollection( subFolder );

    auto* surfaceInSubFolder = new RimRegularSurface();
    surfaceInSubFolder->setUserDescription( "Top" );
    subFolder->addSurface( surfaceInSubFolder );

    EXPECT_EQ( QString( "Top" ), surfaceInSubFolder->userDescription() );
}

//--------------------------------------------------------------------------------------------------
/// Folders and items are two separate namespaces, a folder and a surface may share a name.
//--------------------------------------------------------------------------------------------------
TEST( RiaNameUniquenessTools, FoldersAndItemsAreSeparateNamespaces )
{
    auto rootCollection = std::make_unique<RimSurfaceCollection>();

    auto* surface = new RimRegularSurface();
    surface->setUserDescription( "Top" );
    rootCollection->addSurface( surface );

    auto* subFolder = new RimSurfaceCollection();
    subFolder->setCollectionName( "Top" );
    rootCollection->addSubCollection( subFolder );
    RiaNameUniquenessTools::ensureUniqueAmongSiblings( subFolder );

    EXPECT_EQ( QString( "Top" ), surface->userDescription() );
    EXPECT_EQ( QString( "Top" ), subFolder->collectionName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaNameUniquenessTools, PolygonsAreUniquePerFolder )
{
    auto rootCollection = std::make_unique<RimPolygonCollection>();

    auto* firstPolygon = new RimPolygon();
    firstPolygon->setName( "Fence" );
    rootCollection->addUserDefinedPolygon( firstPolygon );

    auto* secondPolygon = new RimPolygon();
    secondPolygon->setName( "Fence" );
    rootCollection->addUserDefinedPolygon( secondPolygon );

    EXPECT_EQ( QString( "Fence" ), firstPolygon->name() );
    EXPECT_EQ( QString( "Fence_1" ), secondPolygon->name() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaNameUniquenessTools, ConflictPolicyForScripting )
{
    auto rootCollection = std::make_unique<RimSurfaceCollection>();

    auto* surface = new RimRegularSurface();
    surface->setUserDescription( "Top" );
    rootCollection->addSurface( surface );

    {
        auto resolution =
            RiaNameUniquenessTools::applyConflictPolicy( &rootCollection->itemsField(), "Base", RiaDefines::NameConflictPolicy::FAIL );
        EXPECT_TRUE( resolution.errorMessage.isEmpty() );
        EXPECT_EQ( nullptr, resolution.objectToReplace );
        EXPECT_EQ( QString( "Base" ), resolution.nameToUse );
    }

    {
        auto resolution =
            RiaNameUniquenessTools::applyConflictPolicy( &rootCollection->itemsField(), "Top", RiaDefines::NameConflictPolicy::FAIL );
        EXPECT_FALSE( resolution.errorMessage.isEmpty() );
    }

    {
        auto resolution =
            RiaNameUniquenessTools::applyConflictPolicy( &rootCollection->itemsField(), "Top", RiaDefines::NameConflictPolicy::AUTO_RENAME );
        EXPECT_TRUE( resolution.errorMessage.isEmpty() );
        EXPECT_EQ( QString( "Top_1" ), resolution.nameToUse );
    }

    {
        auto resolution =
            RiaNameUniquenessTools::applyConflictPolicy( &rootCollection->itemsField(), "Top", RiaDefines::NameConflictPolicy::OVERWRITE );
        EXPECT_TRUE( resolution.errorMessage.isEmpty() );
        EXPECT_EQ( surface, resolution.objectToReplace );
        EXPECT_EQ( QString( "Top" ), resolution.nameToUse );
    }
}
