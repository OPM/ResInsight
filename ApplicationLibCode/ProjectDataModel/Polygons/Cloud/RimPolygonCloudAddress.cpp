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

#include "RimPolygonCloudAddress.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaQStringFormatter.h"

#include "Cloud/RiaSumoConnector.h"
#include "Cloud/RimCloudDataSourceCollection.h"
#include "Cloud/RimSumoDataSource.h"

#include "Polygons/RimPolygon.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeAttributes.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimPolygonCloudAddress, "RimPolygonCloudAddress" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudAddress::RimPolygonCloudAddress()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "Sumo Polygon Address", ":/CloudBlobs.svg" );

    // Inherited field, kept declared here so the derived class's ui/xml keyword for it stays
    // independent of RimPolygonFile's, matching the convention used there.
    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );

    CAF_PDM_InitFieldNoDefault( &m_dataSource, "DataSource", "Data Source" );
    CAF_PDM_InitField( &m_realization, "Realization", 0, "Realization" );
    m_realization.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_polygonResult, "PolygonResult", QString( "field_outline" ), "Polygon Result" );
    m_polygonResult.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_name, "PolygonName", QString(), "Name" );
    m_name.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_contactType, "ContactType", QString(), "Fluid Contact Type" );
    m_contactType.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setDataSource( RimSumoDataSource* dataSource )
{
    m_dataSource = dataSource;
    invalidateDirectory();
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setRealization( int realization )
{
    m_realization = realization;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setPolygonResult( SumoPolygonResult polygonResult )
{
    m_polygonResult = RiaSumoPolygons::polygonResultKey( polygonResult );
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setPolygonName( const QString& name )
{
    m_name = name;
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setContactType( const QString& contactType )
{
    m_contactType = contactType;
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::loadData()
{
    if ( !hasCompleteSelection() )
    {
        // Selection is not complete yet (e.g. polygon result just changed and name/contact type
        // have not been chosen): clear any stale polygons from a previous selection, but do not
        // fetch or warn -- this is a normal, expected mid-selection state, not an error.
        if ( !m_items.empty() )
        {
            m_items.deleteChildren();
        }
        m_loadedRealization = m_realization();
        return;
    }

    auto fetchedPolygons = fetchPolygonsFromSumo();

    auto existingPolygons = items();
    if ( !fetchedPolygons.empty() && existingPolygons.size() == fetchedPolygons.size() )
    {
        // Same number of polygons as before (the common case: same realization/result, refetched
        // data): update the existing objects in place instead of replacing them, so a view is not
        // forced to throw away and recreate its mirrored RimPolygonInView objects (and any
        // selection/visibility state on them) on every reload.
        for ( size_t i = 0; i < existingPolygons.size(); i++ )
        {
            auto* existingPolygon = existingPolygons[i];
            auto* fetchedPolygon  = fetchedPolygons[i];

            existingPolygon->setDeletable( false );
            existingPolygon->setName( fetchedPolygon->name() );
            existingPolygon->setPointsInDomainCoords( fetchedPolygon->pointsInDomainCoords() );
            existingPolygon->coordinatesChanged.send();
            existingPolygon->objectChanged.send();

            delete fetchedPolygon;
        }
    }
    else
    {
        m_items.deleteChildren();
        m_items.setValue( fetchedPolygons );

        for ( auto* polygon : fetchedPolygons )
        {
            ensureUniquePolygonName( polygon );
        }
    }

    if ( fetchedPolygons.empty() )
    {
        RiaLogging::warning( "No polygons found for Sumo polygon address: " + name().toStdString() );
    }
    else
    {
        RiaLogging::info( std::format( "Fetched {} polygon(s) from Sumo for address: {}", fetchedPolygons.size(), name() ) );
    }

    m_loadedRealization = m_realization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::prepareItemsForRealization( int realization )
{
    if ( realization < 0 ) return;
    if ( realization == m_loadedRealization ) return;

    m_realization = realization;
    loadData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCloudAddress::polygons() const
{
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::name() const
{
    QString nameCandidate = m_collectionName.value();
    if ( !nameCandidate.isEmpty() ) return nameCandidate;

    return "Sumo Polygon Address";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::canAddSubCollection() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer* RimPolygonCloudAddress::addNewSubCollection()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_collectionName );
    uiOrdering.add( &m_dataSource );
    uiOrdering.add( &m_realization );
    uiOrdering.add( &m_polygonResult );

    const bool isFieldOutline    = ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) );
    const bool isFluidContact    = ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) );

    m_name.uiCapability()->setUiHidden( isFieldOutline );
    uiOrdering.add( &m_name );

    m_contactType.uiCapability()->setUiHidden( !isFluidContact );
    uiOrdering.add( &m_contactType );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_dataSource )
    {
        invalidateDirectory();
        m_name        = "";
        m_contactType = "";
        updateName();
        loadData();
    }
    else if ( changedField == &m_polygonResult )
    {
        m_name        = "";
        m_contactType = "";
        updateName();
        loadData();
    }
    else if ( changedField == &m_realization || changedField == &m_name || changedField == &m_contactType )
    {
        updateName();
        loadData();
    }

    RimPolygonContainer::fieldChangedByUi( changedField, oldValue, newValue );

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolygonCloudAddress::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_dataSource )
    {
        for ( auto* dataSource : RimCloudDataSourceCollection::instance()->sumoDataSources() )
        {
            options.push_back( caf::PdmOptionItemInfo( dataSource->caseName() + " / " + dataSource->ensembleName(), dataSource ) );
        }
    }
    else if ( fieldNeedingOptions == &m_realization )
    {
        if ( auto* dataSource = m_dataSource() )
        {
            for ( const auto& realizationId : dataSource->selectedRealizationIds() )
            {
                bool ok    = false;
                int  value = realizationId.toInt( &ok );
                if ( ok ) options.push_back( caf::PdmOptionItemInfo( realizationId, value ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_polygonResult )
    {
        ensureDirectoryFetched();

        if ( !m_cachedDirectory.fieldOutline.empty() )
        {
            options.push_back(
                caf::PdmOptionItemInfo( polygonResultLabel( SumoPolygonResult::FieldOutline ),
                                        RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) ) );
        }
        if ( !m_cachedDirectory.structureDepthFaultLines.empty() )
        {
            options.push_back(
                caf::PdmOptionItemInfo( polygonResultLabel( SumoPolygonResult::StructureDepthFaultLines ),
                                        RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) ) );
        }
        if ( !m_cachedDirectory.fluidContactOutline.empty() )
        {
            options.push_back(
                caf::PdmOptionItemInfo( polygonResultLabel( SumoPolygonResult::FluidContactOutline ),
                                        RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_name )
    {
        ensureDirectoryFetched();

        const std::vector<SumoPolygonMeta>* metaList = nullptr;
        if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
        {
            metaList = &m_cachedDirectory.structureDepthFaultLines;
        }
        else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
        {
            metaList = &m_cachedDirectory.fluidContactOutline;
        }

        if ( metaList )
        {
            std::vector<QString> namesAdded;
            for ( const auto& meta : *metaList )
            {
                if ( std::ranges::find( namesAdded, meta.name ) != namesAdded.end() ) continue;
                namesAdded.push_back( meta.name );
                options.push_back( caf::PdmOptionItemInfo( meta.name, meta.name ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_contactType )
    {
        ensureDirectoryFetched();

        for ( const auto& meta : m_cachedDirectory.fluidContactOutline )
        {
            if ( meta.name != m_name() ) continue;
            if ( meta.contactType.isEmpty() ) continue;

            bool alreadyAdded = false;
            for ( const auto& option : options )
            {
                if ( option.value().toString() == meta.contactType )
                {
                    alreadyAdded = true;
                    break;
                }
            }
            if ( !alreadyAdded ) options.push_back( caf::PdmOptionItemInfo( meta.contactType, meta.contactType ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicReloadPolygonCloudAddressFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( m_items.empty() )
    {
        caf::PdmUiTreeViewItemAttribute::appendTagToTreeViewItemAttribute( attribute, ":/warning.svg" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoConnector* RimPolygonCloudAddress::sumoConnector()
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
void RimPolygonCloudAddress::ensureDirectoryFetched()
{
    if ( m_hasCachedDirectory ) return;

    auto* dataSource = m_dataSource();
    auto* connector  = sumoConnector();
    if ( !dataSource || !connector ) return;

    m_cachedDirectory    = connector->polygons().polygonResultDirectory( dataSource->caseId(), dataSource->ensembleName() );
    m_hasCachedDirectory = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::invalidateDirectory()
{
    m_cachedDirectory    = SumoPolygonDirectory();
    m_hasCachedDirectory = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::updateName()
{
    QStringList parts;

    if ( auto* dataSource = m_dataSource() )
    {
        parts << dataSource->ensembleName();
    }

    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) )
    {
        parts << polygonResultLabel( SumoPolygonResult::FieldOutline );
    }
    else
    {
        if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
        {
            parts << polygonResultLabel( SumoPolygonResult::StructureDepthFaultLines );
        }
        else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
        {
            parts << polygonResultLabel( SumoPolygonResult::FluidContactOutline );
        }

        if ( !m_name().isEmpty() ) parts << m_name();
        if ( !m_contactType().isEmpty() ) parts << m_contactType();
    }

    setCollectionName( parts.join( " / " ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::polygonResultLabel( SumoPolygonResult polygonResult )
{
    switch ( polygonResult )
    {
        case SumoPolygonResult::FieldOutline:
            return "Field Outline";
        case SumoPolygonResult::StructureDepthFaultLines:
            return "Structure Depth Fault Lines";
        case SumoPolygonResult::FluidContactOutline:
            return "Fluid Contact Outline";
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::hasCompleteSelection() const
{
    if ( !m_dataSource() ) return false;

    SumoPolygonResult polygonResult = SumoPolygonResult::FieldOutline;
    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        polygonResult = SumoPolygonResult::StructureDepthFaultLines;
    }
    else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        polygonResult = SumoPolygonResult::FluidContactOutline;
    }

    if ( polygonResult != SumoPolygonResult::FieldOutline && m_name().isEmpty() ) return false;
    if ( polygonResult == SumoPolygonResult::FluidContactOutline && m_contactType().isEmpty() ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCloudAddress::fetchPolygonsFromSumo()
{
    std::vector<RimPolygon*> polygons;

    auto* dataSource = m_dataSource();
    auto* connector  = sumoConnector();
    if ( !dataSource || !connector ) return polygons;

    SumoPolygonResult polygonResult = SumoPolygonResult::FieldOutline;
    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        polygonResult = SumoPolygonResult::StructureDepthFaultLines;
    }
    else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        polygonResult = SumoPolygonResult::FluidContactOutline;
    }

    auto polygonDataList = connector->polygons().polygonsData( dataSource->caseId(),
                                                               dataSource->ensembleName(),
                                                               m_realization(),
                                                               polygonResult,
                                                               m_name(),
                                                               m_contactType() );

    const bool nameGroupsAreDistinct = polygonDataList.size() > 1;

    for ( const auto& data : polygonDataList )
    {
        auto* polygon = new RimPolygon();
        polygon->disableStorageOfPolygonPoints();
        polygon->setReadOnly( true );
        polygon->setDeletable( false );

        QString polygonName = data.name.isEmpty() ? name() : data.name;
        if ( nameGroupsAreDistinct ) polygonName = QString( "%1 (%2)" ).arg( polygonName ).arg( data.polyId );
        polygon->setName( polygonName );

        std::vector<cvf::Vec3d> points;
        const size_t            pointCount = std::min( { data.xArr.size(), data.yArr.size(), data.zArr.size() } );
        points.reserve( pointCount );
        for ( size_t i = 0; i < pointCount; i++ )
        {
            // Sumo's zTvdSSArray is a positive-down TVDSS depth. ResInsight's domain z convention is
            // elevation (negative down), matching the sign flip RifPolygonReader applies when parsing
            // depth values from a polygon file -- so invert here for the same reason.
            points.emplace_back( data.xArr[i], data.yArr[i], -data.zArr[i] );
        }
        polygon->setPointsInDomainCoords( points );

        polygons.push_back( polygon );
    }

    return polygons;
}
