/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimFilterInViewCollection.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimCellFilter.h"
#include "RimCellFilterCollection.h"
#include "RimDataFilterInView.h"
#include "RimDataFilterInViewCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimFilterDisplayUtil.h"
#include "RimTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeOrdering.h"

#include <QIcon>
#include <QStringList>
#include <QVariant>

CAF_PDM_SOURCE_INIT( RimFilterInViewCollection, "FilterInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFilterInViewCollection::RimFilterInViewCollection()
{
    CAF_PDM_InitObject( "Filters", ":/CellFilter.png" );

    CAF_PDM_InitFieldNoDefault( &m_cellFilters, "CellFilters", "Cell Filters" );
    m_cellFilters.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_propertyFilters, "PropertyFilters", "Property Filters" );
    m_propertyFilters.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_dataFiltersInView, "DataFiltersInView", "Data Filters" );
    m_dataFiltersInView.uiCapability()->setUiHidden( true );

    setName( "Filters" );
    nameField()->uiCapability()->setUiHidden( true );

    setCheckState( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFilterInViewCollection::~RimFilterInViewCollection()
{
    disconnectSourceSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::setSourceCollections( RimCellFilterCollection*            cellFilters,
                                                      RimEclipsePropertyFilterCollection* propertyFilters,
                                                      RimDataFilterInViewCollection*      dataFiltersInView )
{
    disconnectSourceSignals();

    m_cellFilters       = cellFilters;
    m_propertyFilters   = propertyFilters;
    m_dataFiltersInView = dataFiltersInView;

    connectSourceSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterCollection* RimFilterInViewCollection::cellFilters() const
{
    return m_cellFilters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection* RimFilterInViewCollection::propertyFilters() const
{
    return m_propertyFilters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterInViewCollection* RimFilterInViewCollection::dataFiltersInView() const
{
    return m_dataFiltersInView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFilterInViewCollection::activeFiltersDisplayText() const
{
    if ( !isChecked() ) return {};

    QStringList parts;

    if ( auto* cf = m_cellFilters(); cf && cf->isActive() && cf->hasActiveFilters() )
    {
        const QString cellPart = RimFilterDisplayUtil::filterNamesJoined( cf->filters(), cf->useAndOperation() );
        if ( !cellPart.isEmpty() ) parts << cellPart;
    }

    const bool useAndOperation = true;

    if ( auto* pf = m_propertyFilters(); pf && pf->isActive && pf->hasActiveFilters() )
    {
        const QString propPart = RimFilterDisplayUtil::filterNamesJoined( pf->filtersForEvaluation(), useAndOperation );
        if ( !propPart.isEmpty() ) parts << propPart;
    }

    if ( auto* df = m_dataFiltersInView(); df && df->isChecked() && df->hasActiveFilters() )
    {
        std::vector<RimCellFilter*> sourceFilters;
        for ( auto* w : df->activeWrappers() )
        {
            if ( w && w->sourceFilter() ) sourceFilters.push_back( w->sourceFilter() );
        }
        const QString dataPart = RimFilterDisplayUtil::filterNamesJoined( sourceFilters, useAndOperation );
        if ( !dataPart.isEmpty() ) parts << dataPart;
    }

    return parts.join( QStringLiteral( ", " ) );
}

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::initAfterRead()
{
    connectSourceSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString /*uiConfigName*/ )
{
    if ( m_cellFilters() )
    {
        for ( RimCellFilter* f : m_cellFilters()->filters() )
        {
            if ( f ) uiTreeOrdering.add( f );
        }
    }

    if ( m_propertyFilters() )
    {
        for ( RimCellFilter* f : m_propertyFilters()->filtersForEvaluation() )
        {
            if ( f ) uiTreeOrdering.add( f );
        }
    }

    if ( m_dataFiltersInView() )
    {
        for ( RimDataFilterInView* w : m_dataFiltersInView()->wrappers() )
        {
            if ( w ) uiTreeOrdering.add( w );
        }
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == objectToggleField() )
    {
        cascadeMasterToggle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    // Cell-filter menu (mirrors RimCellFilterCollection::appendMenuItems).
    menuBuilder << "RicPasteCellFiltersFeature";
    menuBuilder << "Separator";

    menuBuilder.subMenuStart( "Polygon Filter", QIcon( ":/CellFilter_Polygon.png" ) );
    {
        if ( auto* polygonCollection = RimTools::polygonCollection() )
        {
            for ( auto* p : polygonCollection->allPolygons() )
            {
                if ( !p ) continue;

                menuBuilder.addCmdFeatureWithUserData( "RicNewPolygonFilterFeature", p->name(), QVariant::fromValue( static_cast<void*>( p ) ) );
            }
        }
    }
    menuBuilder.subMenuEnd();

    menuBuilder << "RicNewPolygonFilterFeature";
    menuBuilder << "Separator";
    menuBuilder.subMenuStart( "Range Filter" );
    menuBuilder << "RicNewRangeFilterSliceIFeature";
    menuBuilder << "RicNewRangeFilterSliceJFeature";
    menuBuilder << "RicNewRangeFilterSliceKFeature";
    menuBuilder << "RicNewCellRangeFilterFeature";
    menuBuilder.subMenuEnd();
    menuBuilder << "RicNewCellIndexFilterFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicNewUserDefinedFilterFeature";
    menuBuilder << "RicNewUserDefinedIndexFilterFeature";

    // Property-filter menu (mirrors RimEclipsePropertyFilterCollection::appendMenuItems).
    menuBuilder << "Separator";
    menuBuilder << "RicEclipsePropertyFilterNewFeature";
    menuBuilder << "RicAddLinkedEclipsePropertyFilterFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicEclipseCombinedPropertyFilterNewFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::connectSourceSignals()
{
    if ( m_cellFilters() )
    {
        m_cellFilters()->filtersChanged.connect( this, &RimFilterInViewCollection::onSourceFiltersChanged );
    }
    if ( m_propertyFilters() )
    {
        m_propertyFilters()->filtersChanged.connect( this, &RimFilterInViewCollection::onSourceFiltersChanged );
    }
    if ( m_dataFiltersInView() )
    {
        m_dataFiltersInView()->wrappersChanged.connect( this, &RimFilterInViewCollection::onSourceFiltersChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::disconnectSourceSignals()
{
    if ( m_cellFilters() ) m_cellFilters()->filtersChanged.disconnect( this );
    if ( m_propertyFilters() ) m_propertyFilters()->filtersChanged.disconnect( this );
    if ( m_dataFiltersInView() ) m_dataFiltersInView()->wrappersChanged.disconnect( this );
}

//--------------------------------------------------------------------------------------------------
/// Source-collection child set changed (filter added, removed, or reordered). Repaint just this
/// subtree by asking the framework to re-run our defineUiTreeOrdering.
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::onSourceFiltersChanged( const caf::SignalEmitter* /*emitter*/ )
{
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFilterInViewCollection::cascadeMasterToggle()
{
    const bool checked = isChecked();

    if ( m_cellFilters() ) m_cellFilters()->setActive( checked );

    if ( m_propertyFilters() )
    {
        m_propertyFilters()->isActive = checked;
        m_propertyFilters()->updateConnectedEditors();
        m_propertyFilters()->updateDisplayModelNotifyManagedViews( nullptr );
    }

    if ( m_dataFiltersInView() ) m_dataFiltersInView()->setCheckState( checked );
}
