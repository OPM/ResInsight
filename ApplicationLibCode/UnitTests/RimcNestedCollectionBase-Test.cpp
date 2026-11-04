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

#include "Polygons/Cloud/RimPolygonCloudAddress.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonFile.h"

#include "RimcDataContainerString.h"
#include "RimcNestedCollectionBase.h"

#include <memory>

namespace
{
QString scriptCollectionName( caf::PdmNestedCollectionBase* collection )
{
    RimcNestedCollectionBase_collectionName method( collection );

    auto result = method.execute();
    if ( !result.has_value() ) return "<error>";

    std::unique_ptr<RimcDataContainerString> strings( dynamic_cast<RimcDataContainerString*>( result.value() ) );
    if ( !strings || strings->m_stringValues().size() != 1 ) return "<error>";

    return strings->m_stringValues().front();
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimcNestedCollectionBase, CollectionNameReturnsStoredName )
{
    RimPolygonCollection collection;
    collection.setCollectionName( "Folder A" );

    EXPECT_EQ( "Folder A", scriptCollectionName( &collection ) );
}

//--------------------------------------------------------------------------------------------------
/// A polygon file falls back to the file name, or to a default, when no name is stored.
//--------------------------------------------------------------------------------------------------
TEST( RimcNestedCollectionBase, CollectionNameUsesPolygonFileFallback )
{
    RimPolygonFile polygonFile;
    EXPECT_TRUE( polygonFile.collectionName().isEmpty() );
    EXPECT_EQ( "Polygon File", scriptCollectionName( &polygonFile ) );

    // Setting the file stores the base name of the file as the folder name
    polygonFile.setFileName( "/data/polygons/outline.pol" );
    EXPECT_EQ( "outline", scriptCollectionName( &polygonFile ) );

    polygonFile.setCollectionName( "" );
    EXPECT_EQ( "outline.pol", scriptCollectionName( &polygonFile ) );
    EXPECT_EQ( polygonFile.name(), scriptCollectionName( &polygonFile ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimcNestedCollectionBase, CollectionNameUsesCloudAddressFallback )
{
    RimPolygonCloudAddress address;
    EXPECT_EQ( "Sumo Polygon Address", scriptCollectionName( &address ) );

    address.setCollectionName( "Outline" );
    EXPECT_EQ( "Outline", scriptCollectionName( &address ) );
}
