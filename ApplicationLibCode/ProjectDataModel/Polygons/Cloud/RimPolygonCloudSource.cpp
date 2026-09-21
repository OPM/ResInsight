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

#include "RimPolygonCloudSource.h"

#include "RiaApplication.h"

#include "Cloud/RiaSumoConnector.h"
#include "Cloud/RimCloudDataSourceCollection.h"
#include "Cloud/RimSumoDataSource.h"

#include "Polygons/Cloud/RimPolygonCloudAddress.h"
#include "Polygons/Cloud/RimPolygonCloudFolder.h"
#include "Polygons/Cloud/RimPolygonCloudRealizationGroup.h"
#include "Polygons/RimPolygon.h"

#include "Polygons/RimPolygonInViewCollection.h"

#include "Rim3dView.h"
#include "RimRoffCaseSumo.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiComboBoxEditor.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimPolygonCloudSource, "RimPolygonCloudSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudSource::RimPolygonCloudSource()
{
    CAF_PDM_InitObject( "Cloud Polygon Source", ":/CloudBlobs.svg" );

    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );
    m_items.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_dataSource, "DataSource", "Data Source" );
    m_dataSource.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_baseRealization, "BaseRealization", 0, "Base Realization" );
    m_baseRealization.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_directoryBuilt, "DirectoryBuilt", false, "Directory Built" );
    m_directoryBuilt.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::setDataSource( RimSumoDataSource* dataSource )
{
    m_dataSource = dataSource;
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::setBaseRealization( int realization )
{
    m_baseRealization = realization;
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSumoDataSource* RimPolygonCloudSource::dataSource() const
{
    return m_dataSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPolygonCloudSource::baseRealization() const
{
    return m_baseRealization();
}

//--------------------------------------------------------------------------------------------------
/// Fetches the polygon result directory once (metadata only) and builds the full nested
/// RimPolygonCloudFolder/RimPolygonCloudAddress structure. No-op if already built.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::buildDirectoryTree()
{
    if ( m_directoryBuilt() ) return;

    auto* dataSource = m_dataSource();
    auto* connector  = sumoConnector();
    if ( !dataSource || !connector ) return;

    auto directory = connector->polygons().polygonResultDirectory( dataSource->caseId(), dataSource->ensembleName() );

    if ( !directory.fieldOutline.empty() )
    {
        auto* folder  = new RimPolygonCloudFolder( "Field Outline" );
        auto* address = new RimPolygonCloudAddress();
        address->configureIdentity( SumoPolygonResult::FieldOutline, QString(), QString() );
        folder->addSubCollection( address );
        addSubCollection( folder );
    }

    if ( !directory.structureDepthFaultLines.empty() )
    {
        auto* folder = new RimPolygonCloudFolder( "Structure Depth Fault Lines" );

        std::vector<QString> namesAdded;
        for ( const auto& meta : directory.structureDepthFaultLines )
        {
            if ( std::ranges::find( namesAdded, meta.name ) != namesAdded.end() ) continue;
            namesAdded.push_back( meta.name );

            auto* address = new RimPolygonCloudAddress();
            address->configureIdentity( SumoPolygonResult::StructureDepthFaultLines, meta.name, QString() );
            folder->addSubCollection( address );
        }

        addSubCollection( folder );
    }

    if ( !directory.fluidContactOutline.empty() )
    {
        auto* resultFolder = new RimPolygonCloudFolder( "Fluid Contact Outline" );

        std::vector<QString> namesAdded;
        for ( const auto& meta : directory.fluidContactOutline )
        {
            if ( std::ranges::find( namesAdded, meta.name ) != namesAdded.end() ) continue;
            namesAdded.push_back( meta.name );

            auto* nameFolder = new RimPolygonCloudFolder( meta.name );

            for ( const auto& contactMeta : directory.fluidContactOutline )
            {
                if ( contactMeta.name != meta.name || contactMeta.contactType.isEmpty() ) continue;

                auto* address = new RimPolygonCloudAddress();
                address->configureIdentity( SumoPolygonResult::FluidContactOutline, meta.name, contactMeta.contactType );
                nameFolder->addSubCollection( address );
            }

            resultFolder->addSubCollection( nameFolder );
        }

        addSubCollection( resultFolder );
    }

    m_directoryBuilt = true;

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudSource::name() const
{
    return collectionName();
}

//--------------------------------------------------------------------------------------------------
/// The folder/address tree beneath this object is only ever built by buildDirectoryTree() -- no
/// user-driven add/remove of sub-collections here.
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudSource::canAddSubCollection() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer* RimPolygonCloudSource::addNewSubCollection()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudSource::supportsRealizationOverride() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimPolygonCloudSource::availableRealizationIdsForOverride() const
{
    std::vector<int> realizations;

    if ( auto* ds = m_dataSource() )
    {
        for ( const auto& realizationId : ds->selectedRealizationIds() )
        {
            bool ok    = false;
            int  value = realizationId.toInt( &ok );
            if ( ok ) realizations.push_back( value );
        }
    }

    return realizations;
}

//--------------------------------------------------------------------------------------------------
/// Only follows a view's own case realization when that case is a RimRoffCaseSumo created from
/// this source's own data source -- i.e. the same Sumo case/ensemble. A view whose case belongs to
/// a different field/data source (e.g. a Johan Sverdrup grid case in an otherwise Drogon-
/// configured project) must never have its realization silently applied to this source's leaves.
//--------------------------------------------------------------------------------------------------
int RimPolygonCloudSource::resolveViewMatchingRealization( const Rim3dView* view ) const
{
    if ( !view || !m_dataSource() ) return -1;

    if ( auto* sumoCase = dynamic_cast<const RimRoffCaseSumo*>( view->ownerCase() ) )
    {
        if ( sumoCase->dataSource() == m_dataSource() )
        {
            return sumoCase->realization();
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
/// Walks every RimPolygonCloudAddress beneath this source and evicts any realization group (or the
/// base data) that is no longer checked in any open view.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::evictUnusedRealizationData()
{
    for ( auto* address : allAddresses() )
    {
        if ( address->hasBaseData() && !RimPolygonInViewCollection::isSourceCheckedInAnyView( address ) )
        {
            address->evictBaseData();
        }

        for ( auto* group : address->realizationGroups() )
        {
            if ( !RimPolygonInViewCollection::isSourceCheckedInAnyView( group ) )
            {
                address->evictRealizationGroup( group->realization() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_collectionName );
    uiOrdering.add( &m_dataSource );
    uiOrdering.add( &m_baseRealization );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoConnector* RimPolygonCloudSource::sumoConnector()
{
    if ( !m_sumoConnector )
    {
        m_sumoConnector = RiaApplication::instance()->makeSumoConnector();
    }

    return m_sumoConnector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::updateName()
{
    QString baseName = m_dataSource() ? m_dataSource()->name() : QString( "Cloud Polygon Source" );
    setCollectionName( QString( "%1 / Real %2" ).arg( baseName ).arg( m_baseRealization() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonCloudAddress*> RimPolygonCloudSource::allAddresses() const
{
    std::vector<RimPolygonCloudAddress*> addresses;

    std::function<void( const RimPolygonContainer* )> visit = [&]( const RimPolygonContainer* container )
    {
        for ( auto* sub : container->subCollections() )
        {
            if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( sub ) )
            {
                addresses.push_back( address );
            }
            visit( sub );
        }
    };
    visit( this );

    return addresses;
}
