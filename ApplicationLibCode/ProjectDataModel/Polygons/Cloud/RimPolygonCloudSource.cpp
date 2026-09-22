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
#include "Polygons/RimPolygon.h"

#include "Polygons/RimPolygonInViewCollection.h"

#include "Rim3dView.h"
#include "RimRoffCaseSumo.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiButtonBox.h"
#include "cafPdmUiComboBoxEditor.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimPolygonCloudSource, "RimPolygonCloudSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudSource::RimPolygonCloudSource()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "Cloud Polygon Source", ":/CloudBlobs.svg" );

    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );
    m_items.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_dataSource, "DataSource", "Data Source" );
    m_dataSource.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_baseRealization, "BaseRealization", -1, "Base Realization" );
    m_baseRealization.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_appliedDataSource, "AppliedDataSource", "Applied Data Source" );
    m_appliedDataSource.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_appliedBaseRealization, "AppliedBaseRealization", -1, "Applied Base Realization" );
    m_appliedBaseRealization.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_directoryBuilt, "DirectoryBuilt", false, "Directory Built" );
    m_directoryBuilt.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
/// Sets both the pending and applied selection directly -- for deliberate one-shot setup (e.g. from
/// a creation command), not a live UI edit. Does not trigger a fetch.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::setDataSource( RimSumoDataSource* dataSource )
{
    m_dataSource        = dataSource;
    m_appliedDataSource = dataSource;
    updateName();
}

//--------------------------------------------------------------------------------------------------
/// Sets both the pending and applied selection directly -- see setDataSource().
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::setBaseRealization( int realization )
{
    m_baseRealization        = realization;
    m_appliedBaseRealization = realization;
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSumoDataSource* RimPolygonCloudSource::dataSource() const
{
    return m_appliedDataSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPolygonCloudSource::baseRealization() const
{
    return m_appliedBaseRealization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudSource::isDirectoryBuilt() const
{
    return m_directoryBuilt();
}

//--------------------------------------------------------------------------------------------------
/// Fetches the polygon result directory once (metadata only) and builds the full nested
/// RimPolygonCloudFolder/RimPolygonCloudAddress structure, for the currently *applied* data
/// source/base realization. No-op if already built.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::buildDirectoryTree()
{
    if ( m_directoryBuilt() ) return;

    auto* dataSource = m_appliedDataSource();
    auto* connector  = sumoConnector();
    if ( !dataSource || m_appliedBaseRealization() < 0 || !connector ) return;

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
    if ( !view || !m_appliedDataSource() ) return -1;

    if ( auto* sumoCase = dynamic_cast<const RimRoffCaseSumo*>( view->ownerCase() ) )
    {
        if ( sumoCase->dataSource() == m_appliedDataSource() )
        {
            return sumoCase->realization();
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
/// Walks every RimPolygonCloudAddress beneath this source and evicts its base data and/or any
/// cached non-base realization that is no longer in use (checked, with that realization resolved
/// as effective) by any open view.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::evictUnusedRealizationData()
{
    for ( auto* address : allAddresses() )
    {
        address->evictAllUnusedRealizations();

        if ( address->hasBaseData() && !RimPolygonInViewCollection::isRealizationInUseInAnyView( address, baseRealization() ) )
        {
            address->evictBaseData();
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

    const bool hasValidPendingSelection = m_dataSource() != nullptr && m_baseRealization() >= 0;
    const bool pending                  = hasPendingChanges();

    auto* buttonBox = uiOrdering.addNewButtonBox();

    auto& applyButton = buttonBox->addButton( "Apply", [this]() { onApplyClicked(); } );
    applyButton.enabled = hasValidPendingSelection && pending;
    applyButton.toolTip = hasValidPendingSelection ? QString( "" ) : QString( "Select a Data Source and Base Realization first." );

    auto& cancelButton = buttonBox->addButton( "Cancel", [this]() { onCancelClicked(); } );
    cancelButton.enabled = m_directoryBuilt() && pending;
    cancelButton.toolTip = "Discards the pending edits above, restoring the currently applied Data Source and Base Realization.";

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
void RimPolygonCloudSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_dataSource )
    {
        // A new data source invalidates whatever realization was picked for the previous one.
        auto realizations = availableRealizationIdsForOverride();
        m_baseRealization  = realizations.empty() ? -1 : realizations.front();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolygonCloudSource::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_dataSource )
    {
        for ( auto* ds : RimCloudDataSourceCollection::instance()->sumoDataSources() )
        {
            options.push_back( caf::PdmOptionItemInfo( ds->name(), ds ) );
        }
    }
    else if ( fieldNeedingOptions == &m_baseRealization )
    {
        for ( int realization : availableRealizationIdsForOverride() )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( realization ), realization ) );
        }
    }

    return options;
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
    setCollectionName( composeName( m_appliedBaseRealization() ) );
}

//--------------------------------------------------------------------------------------------------
/// Composes the display name for an arbitrary realization -- shared by updateName() (always uses
/// the Applied base realization) and nameForRealization() (used by view mirrors that are Auto-
/// Following a different realization).
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudSource::composeName( int realization ) const
{
    if ( !m_appliedDataSource() ) return "Cloud Polygon Source";

    if ( realization < 0 ) return m_appliedDataSource()->name();

    return QString( "%1 / Real %2" ).arg( m_appliedDataSource()->name() ).arg( realization );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudSource::nameForRealization( int realization ) const
{
    return composeName( realization );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudSource::hasPendingChanges() const
{
    if ( !m_directoryBuilt() ) return true;

    return m_dataSource() != m_appliedDataSource() || m_baseRealization() != m_appliedBaseRealization();
}

//--------------------------------------------------------------------------------------------------
/// Commits the pending Data Source/Base Realization selection. The very first Apply builds the
/// folder/leaf tree from scratch. A later Apply (after the tree already exists) either rebuilds it
/// from scratch (if the data source/ensemble itself changed) or just evicts every address's stale
/// base-realization data (if only the base realization changed) -- see the class comment.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::onApplyClicked()
{
    const bool wasBuilt          = m_directoryBuilt();
    const bool dataSourceChanged = m_dataSource() != m_appliedDataSource();

    m_appliedDataSource      = m_dataSource();
    m_appliedBaseRealization = m_baseRealization();

    updateName();

    if ( wasBuilt && dataSourceChanged )
    {
        // The ensemble identity itself changed -- the existing folder/leaf tree (and any fetched/
        // cached polygon data underneath it) was built for the old ensemble and no longer applies.
        m_subCollections.deleteChildren();
        m_directoryBuilt = false;
    }
    else if ( wasBuilt )
    {
        // Same data source, only the base realization changed -- the folder/leaf structure itself
        // is still valid, but every address's own already-fetched base-realization data was
        // fetched for the old value and must be evicted so it is lazily re-fetched for the new one.
        for ( auto* address : allAddresses() )
        {
            address->evictBaseData();
        }
    }

    buildDirectoryTree();

    updateConnectedEditors();
    uiCapability()->updateAllRequiredEditors();

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Discards the pending edits, restoring the pending Data Source/Base Realization fields back to
/// the currently applied selection. No fetch, no change to the tree.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudSource::onCancelClicked()
{
    m_dataSource      = m_appliedDataSource();
    m_baseRealization = m_appliedBaseRealization();

    updateConnectedEditors();
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
