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

#include "RimPolygonCloudRealizationGroup.h"

#include "Polygons/RimPolygon.h"

CAF_PDM_SOURCE_INIT( RimPolygonCloudRealizationGroup, "RimPolygonCloudRealizationGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudRealizationGroup::RimPolygonCloudRealizationGroup()
{
    CAF_PDM_InitObject( "Realization", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );

    CAF_PDM_InitField( &m_realization, "Realization", -1, "Realization" );
    m_realization.uiCapability()->setUiReadOnly( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudRealizationGroup::RimPolygonCloudRealizationGroup( int realization )
    : RimPolygonCloudRealizationGroup()
{
    m_realization = realization;
    setCollectionName( QString( "Real %1" ).arg( realization ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPolygonCloudRealizationGroup::realization() const
{
    return m_realization();
}

//--------------------------------------------------------------------------------------------------
/// Only populated programmatically (RimPolygonCloudAddress::ensureRealizationGroupFetched()) --
/// no user-driven add/remove of sub-collections here.
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudRealizationGroup::canAddSubCollection() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer* RimPolygonCloudRealizationGroup::addNewSubCollection()
{
    return nullptr;
}
