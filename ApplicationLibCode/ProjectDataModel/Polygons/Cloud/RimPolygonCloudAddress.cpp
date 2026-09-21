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
#include "Polygons/Cloud/RimPolygonCloudRealizationGroup.h"
#include "Polygons/Cloud/RimPolygonCloudSource.h"
#include "Polygons/RimPolygon.h"

#include "cafCmdFeatureMenuBuilder.h"
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
        RiaLogging::info( QString( "Fetched %1 polygon(s) from Sumo for address: %2" ).arg( fetchedPolygons.size() ).arg( name() ).toStdString() );
    }

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::evictBaseData()
{
    if ( !hasBaseData() ) return;

    m_items.deleteChildren();
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::hasRealizationGroup( int realization ) const
{
    for ( auto* sub : subCollections() )
    {
        if ( auto* group = dynamic_cast<RimPolygonCloudRealizationGroup*>( sub ) )
        {
            if ( group->realization() == realization ) return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Creates (fetching data for) a comparison realization group if it does not already exist.
/// Idempotent: returns the existing group if one is already present for this realization.
//--------------------------------------------------------------------------------------------------
RimPolygonCloudRealizationGroup* RimPolygonCloudAddress::ensureRealizationGroupFetched( int realization )
{
    for ( auto* sub : subCollections() )
    {
        if ( auto* group = dynamic_cast<RimPolygonCloudRealizationGroup*>( sub ) )
        {
            if ( group->realization() == realization ) return group;
        }
    }

    auto* group           = new RimPolygonCloudRealizationGroup( realization );
    auto  fetchedPolygons = fetchPolygonsFromSumo( realization );

    group->itemsField().setValue( fetchedPolygons );
    for ( auto* polygon : fetchedPolygons )
    {
        group->ensureUniquePolygonName( polygon );
    }

    addSubCollection( group );

    objectChanged.send();

    return group;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::evictRealizationGroup( int realization )
{
    for ( auto* sub : subCollections() )
    {
        if ( auto* group = dynamic_cast<RimPolygonCloudRealizationGroup*>( sub ) )
        {
            if ( group->realization() == realization )
            {
                m_subCollections.removeChild( group );
                delete group;
                objectChanged.send();
                return;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimPolygonCloudAddress::fetchedRealizationGroupRealizations() const
{
    std::vector<int> realizations;
    for ( auto* sub : subCollections() )
    {
        if ( auto* group = dynamic_cast<RimPolygonCloudRealizationGroup*>( sub ) )
        {
            realizations.push_back( group->realization() );
        }
    }
    return realizations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonCloudRealizationGroup*> RimPolygonCloudAddress::realizationGroups() const
{
    std::vector<RimPolygonCloudRealizationGroup*> groups;
    for ( auto* sub : subCollections() )
    {
        if ( auto* group = dynamic_cast<RimPolygonCloudRealizationGroup*>( sub ) )
        {
            groups.push_back( group );
        }
    }
    return groups;
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
    if ( !hasBaseData() )
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
void RimPolygonCloudAddress::updateName()
{
    QStringList parts;

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

    auto polygonDataList =
        connector->polygons().polygonsData( dataSource->caseId(), dataSource->ensembleName(), realization, polygonResult(), m_name(), m_contactType() );

    const bool nameGroupsAreDistinct = polygonDataList.size() > 1;

    for ( const auto& data : polygonDataList )
    {
        auto* polygon = new RimCloudPolygon();
        polygon->disableStorageOfPolygonPoints();
        polygon->setReadOnly( true );
        polygon->setDeletable( false );
        polygon->setSumoIdentity( dataSource->caseId().get(), dataSource->ensembleName(), realization, m_polygonResult(), m_name(), m_contactType() );

        QString polygonName = data.name.isEmpty() ? name() : data.name;
        if ( nameGroupsAreDistinct ) polygonName = QString( "%1 (%2)" ).arg( polygonName ).arg( data.polyId );
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
