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

#include "RimPolygonCloudFolder.h"

#include "Polygons/RimPolygon.h"

CAF_PDM_SOURCE_INIT( RimPolygonCloudFolder, "RimPolygonCloudFolder" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudFolder::RimPolygonCloudFolder()
{
    CAF_PDM_InitObject( "Folder", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );
    m_items.uiCapability()->setUiHidden( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudFolder::RimPolygonCloudFolder( const QString& folderName )
    : RimPolygonCloudFolder()
{
    setCollectionName( folderName );
}

//--------------------------------------------------------------------------------------------------
/// Only built programmatically by RimPolygonCloudSource::buildDirectoryTree().
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudFolder::canAddSubCollection() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer* RimPolygonCloudFolder::addNewSubCollection()
{
    return nullptr;
}
