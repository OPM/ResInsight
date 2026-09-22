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

#include "Cloud/RiaSumoConnector.h"
#include "Cloud/RimSumoDataSource.h"

#include "Polygons/Cloud/RimCloudPolygon.h"
#include "Polygons/Cloud/RimPolygonCloudSource.h"
#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInViewCollection.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeAttributes.h"

#include <QColor>

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimPolygonCloudAddress, "RimPolygonCloudAddress" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudAddress::RimPolygonCloudAddress()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "Sumo Polygon Address", ":/CloudBlobs.svg" );

    // Inherited fields, kept declared here so this derived class's ui/xml keyword for them stays
    // independent of siblings', matching the convention used throughout this container hierarchy.
    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );

    // Fixed identity, set once via configureIdentity() -- read-only in the property panel.
    CAF_PDM_InitField( &m_polygonResult, "PolygonResult", QString( "field_outline" ), "Polygon Result" );
    m_polygonResult.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_name, "PolygonName", QString(), "Name" );
    m_name.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_contactType, "ContactType", QString(), "Fluid Contact Type" );
    m_contactType.uiCapability()->setUiReadOnly( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudAddress::~RimPolygonCloudAddress()
{
    for ( auto& [realization, polygons] : m_realizationCache )
    {
        for ( auto* polygon : polygons )
        {
            delete polygon;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::configureIdentity( SumoPolygonResult polygonResult, const QString& sumoName, const QString& contactType )
{
    m_polygonResult = RiaSumoPolygons::polygonResultKey( polygonResult );
    m_name          = sumoName;
    m_contactType   = contactType;

    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SumoPolygonResult RimPolygonCloudAddress::polygonResult() const
{
    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        return SumoPolygonResult::StructureDepthFaultLines;
    }
    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        return SumoPolygonResult::FluidContactOutline;
    }
    return SumoPolygonResult::FieldOutline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::sumoName() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::contactType() const
{
    return m_contactType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudSource* RimPolygonCloudAddress::owningSource() const
{
    return firstAncestorOfType<RimPolygonCloudSource>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::hasBaseData() const
{
    return !items().empty();
}

//--------------------------------------------------------------------------------------------------
/// Idempotent: no-op if base data has already been fetched. Safe to call on every view sync.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::ensureBaseFetched()
{
    if ( hasBaseData() ) return;

    auto* source = owningSource();
    if ( !source ) return;

    auto fetchedPolygons = fetchPolygonsFromSumo( source->baseRealization() );

    m_items.deleteChildren();
    m_items.setValue( fetchedPolygons );
    for ( auto* polygon : fetchedPolygons )
    {
        ensureUniquePolygonName( polygon );
    }

    if ( fetchedPolygons.empty() )
    {
        RiaLogging::warning( "No polygons found for Sumo polygon address: " + name().toStdString() );
    }
    else
    {
        RiaLogging::info(
            QString( "Fetched %1 polygon(s) from Sumo for address: %2" ).arg( fetchedPolygons.size() ).arg( name() ).toStdString() );
    }

    // The address's own project-tree node was already rendered (with zero children) when the
    // directory tree was first built -- a structural refresh is required for the newly-added
    // RimCloudPolygon children to actually show up there (updateConnectedEditors() alone only
    // refreshes editors bound to this object's own fields, e.g. the property panel).
    uiCapability()->updateAllRequiredEditors();

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::evictBaseData()
{
    if ( !hasBaseData() ) return;

    m_items.deleteChildren();
    uiCapability()->updateAllRequiredEditors();
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::hasDataForRealization( int realization ) const
{
    auto it = m_realizationCache.find( realization );
    return it != m_realizationCache.end() && !it->second.empty();
}

//--------------------------------------------------------------------------------------------------
/// Idempotent: no-op if this realization has already been fetched and cached.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::ensureRealizationFetched( int realization )
{
    if ( m_realizationCache.find( realization ) != m_realizationCache.end() ) return;

    auto fetchedPolygons = fetchPolygonsFromSumo( realization );
    for ( auto* polygon : fetchedPolygons )
    {
        ensureUniquePolygonName( polygon );
    }

    m_realizationCache[realization] = fetchedPolygons;

    if ( fetchedPolygons.empty() )
    {
        RiaLogging::warning(
            QString( "No polygons found for Sumo polygon address: %1 (realization %2)" ).arg( name() ).arg( realization ).toStdString() );
    }
    else
    {
        RiaLogging::info( QString( "Fetched %1 polygon(s) from Sumo for address: %2 (realization %3)" )
                              .arg( fetchedPolygons.size() )
                              .arg( name() )
                              .arg( realization )
                              .toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Pure read -- never triggers a fetch. Returns an empty vector if this realization has not been
/// fetched yet (or has none), see ensureRealizationFetched().
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCloudAddress::cachedItemsForRealization( int realization ) const
{
    auto it = m_realizationCache.find( realization );
    if ( it == m_realizationCache.end() ) return {};
    return it->second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::evictRealizationIfUnused( int realization )
{
    auto it = m_realizationCache.find( realization );
    if ( it == m_realizationCache.end() ) return;

    for ( auto* polygon : it->second )
    {
        delete polygon;
    }
    m_realizationCache.erase( it );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimPolygonCloudAddress::cachedRealizations() const
{
    std::vector<int> realizations;
    for ( const auto& [realization, polygons] : m_realizationCache )
    {
        realizations.push_back( realization );
    }
    return realizations;
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
/// Realization-group children are only ever added programmatically -- no user-driven add/remove
/// of sub-collections here.
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
    uiOrdering.add( &m_polygonResult );

    const bool isFieldOutline = ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) );
    const bool isFluidContact = ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) );

    m_name.uiCapability()->setUiHidden( isFieldOutline );
    uiOrdering.add( &m_name );

    m_contactType.uiCapability()->setUiHidden( !isFluidContact );
    uiOrdering.add( &m_contactType );

    uiOrdering.skipRemainingFields();
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
    if ( hasBaseData() ) return;

    auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute );
    if ( !treeItemAttribute ) return;

    // Clickable download icon -- fetches this leaf's own base realization directly from the tree.
    auto downloadTag     = caf::PdmUiTreeViewItemAttribute::createTag();
    downloadTag->icon    = caf::IconProvider( ":/Download.svg" );
    downloadTag->toolTip = "Fetch polygon data";
    downloadTag->clicked.connect( this, &RimPolygonCloudAddress::onDownloadTagClicked );
    treeItemAttribute->tags.push_back( std::move( downloadTag ) );

    // Plain, unobtrusive "not fetched" marker -- brackets, no colored pill.
    auto textTag     = caf::PdmUiTreeViewItemAttribute::createTag();
    textTag->text    = "[Not fetched]";
    textTag->bgColor = QColor( Qt::white );
    textTag->fgColor = QColor( Qt::darkGray );
    treeItemAttribute->tags.push_back( std::move( textTag ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::onDownloadTagClicked( const caf::SignalEmitter* emitter, size_t index )
{
    ensureBaseFetched();

    updateAllRequiredEditors();
    objectChanged.send();
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
void RimPolygonCloudAddress::updateName()
{
    QString name;

    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) )
    {
        // No distinguishing name/contact type exists for field outline, and there is always
        // exactly one such leaf under the "Field Outline" folder -- fall back to the category
        // label itself.
        name = polygonResultLabel( SumoPolygonResult::FieldOutline );
    }
    else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        // The owning "Structure Depth Fault Lines" folder already conveys the category -- avoid
        // repeating it here.
        name = m_name();
    }
    else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        // The owning folder chain ("Fluid Contact Outline" / <name>) already conveys the category
        // and name -- only the distinguishing contact type is left to show here.
        name = m_contactType();
    }

    setCollectionName( name );
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
/// Fetches from Sumo for the given realization -- this address's own fixed polygon result/name/
/// contact type, combined with the owning source's data source.
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCloudAddress::fetchPolygonsFromSumo( int realization )
{
    std::vector<RimPolygon*> polygons;

    auto* source = owningSource();
    if ( !source ) return polygons;

    auto* dataSource = source->dataSource();
    auto* connector  = sumoConnector();
    if ( !dataSource || !connector ) return polygons;

    auto polygonDataList = connector->polygons().polygonsData( dataSource->caseId(),
                                                               dataSource->ensembleName(),
                                                               realization,
                                                               polygonResult(),
                                                               m_name(),
                                                               m_contactType() );

    const bool nameGroupsAreDistinct = polygonDataList.size() > 1;

    for ( const auto& data : polygonDataList )
    {
        auto* polygon = new RimCloudPolygon();
        polygon->disableStorageOfPolygonPoints();
        polygon->setReadOnly( true );
        polygon->setDeletable( false );
        polygon->setSumoIdentity( dataSource->caseId().get(), dataSource->ensembleName(), realization, m_polygonResult(), m_name(), m_contactType() );

        QString polygonName = data.name.isEmpty() ? name() : data.name;
        if ( nameGroupsAreDistinct ) polygonName = QString( "%1 (ID: %2)" ).arg( polygonName ).arg( data.polyId );
        polygon->setName( polygonName );

        std::vector<cvf::Vec3d> points;
        const size_t            pointCount = std::min( { data.xArr.size(), data.yArr.size(), data.zArr.size() } );
        points.reserve( pointCount );
        for ( size_t i = 0; i < pointCount; i++ )
        {
            // Sumo's zTvdSSArray is a positive-down TVDSS depth. ResInsight's domain z convention
            // is elevation (negative down), matching the sign flip RifPolygonReader applies when
            // parsing depth values from a polygon file -- so invert here for the same reason.
            points.emplace_back( data.xArr[i], data.yArr[i], -data.zArr[i] );
        }
        polygon->setPointsInDomainCoords( points );

        polygons.push_back( polygon );
    }

    return polygons;
}
