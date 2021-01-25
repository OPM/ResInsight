/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCellFilterCollection.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilter.h"
#include "RimCellRangeFilter.h"
#include "RimPolygonFilter.h"
#include "RimUserDefinedFilter.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cvfStructGridGeometryGenerator.h"

CAF_PDM_SOURCE_INIT( RimCellFilterCollection, "RimCellFilterCollection", "CellRangeFilterCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterCollection::RimCellFilterCollection()
{
    CAF_PDM_InitScriptableObject( "Cell Filters", ":/CellFilter.png", "", "" );

    CAF_PDM_InitScriptableField( &m_isActive, "Active", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_cellFilters, "CellFilters", "Filters", "", "", "" );
    m_cellFilters.uiCapability()->setUiTreeHidden( true );
    caf::PdmFieldReorderCapability::addToField( &m_cellFilters );

    // for backwards project file compatibility with old CellRangeFilterCollection
    CAF_PDM_InitFieldNoDefault( &m_rangeFilters_OBSOLETE, "RangeFilters", "Range Filters", "", "", "" );
    m_rangeFilters_OBSOLETE.uiCapability()->setUiHidden( true );
    m_rangeFilters_OBSOLETE.xmlCapability()->setIOWritable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterCollection::~RimCellFilterCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterCollection::isEmpty() const
{
    return m_cellFilters.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::setActive( bool bActive )
{
    m_isActive = bActive;
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::setCase( RimCase* theCase )
{
    for ( RimCellFilter* filter : m_cellFilters )
    {
        RimPolygonFilter* polyFilter = dynamic_cast<RimPolygonFilter*>( filter );
        if ( polyFilter ) polyFilter->setCase( theCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCellFilter*> RimCellFilterCollection::filters() const
{
    return m_cellFilters.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::initAfterRead()
{
    std::list<RimCellRangeFilter*> filters;

    // get any old range filters and put them in the new cell filter collection
    for ( auto& filter : m_rangeFilters_OBSOLETE )
    {
        filters.push_back( filter );
    }
    for ( auto& filter : filters )
    {
        m_rangeFilters_OBSOLETE.removeChildObject( filter );
        m_cellFilters.push_back( filter );
    }

    for ( const auto& filter : m_cellFilters )
    {
        filter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue )
{
    updateIconState();
    uiCapability()->updateConnectedEditors();

    onFilterUpdated( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellFilterCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    PdmObject::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    Rim3dView* rimView = nullptr;
    this->firstAncestorOrThisOfType( rimView );
    RimViewController* viewController = rimView->viewController();
    if ( viewController && ( viewController->isPropertyFilterOveridden() || viewController->isVisibleCellsOveridden() ) )
    {
        m_isActive.uiCapability()->setUiReadOnly( true, uiConfigName );
    }
    else
    {
        m_isActive.uiCapability()->setUiReadOnly( false, uiConfigName );
    }

    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::updateIconState()
{
    bool activeIcon = true;

    Rim3dView* rimView = nullptr;
    this->firstAncestorOrThisOfType( rimView );
    RimViewController* viewController = rimView->viewController();

    bool isControlled = viewController &&
                        ( viewController->isCellFiltersControlled() || viewController->isVisibleCellsOveridden() );

    if ( isControlled )
    {
        activeIcon = false;
    }

    if ( !isActive() )
    {
        activeIcon = false;
    }

    updateUiIconFromState( activeIcon );

    for ( auto& filter : m_cellFilters )
    {
        filter->updateActiveState( isControlled );
        filter->updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterCollection::hasActiveFilters() const
{
    if ( !isActive() ) return false;

    for ( const auto& filter : m_cellFilters )
    {
        if ( filter->isFilterEnabled() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterCollection::hasActiveIncludeFilters() const
{
    if ( !isActive() ) return false;

    for ( const auto& filter : m_cellFilters )
    {
        if ( filter->isFilterEnabled() && filter->filterMode() == RimCellFilter::INCLUDE ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFilter* RimCellFilterCollection::addNewPolygonFilter( RimCase* srcCase )
{
    RimPolygonFilter* pFilter = new RimPolygonFilter();
    pFilter->setCase( srcCase );
    addFilter( pFilter );
    pFilter->enablePicking( true );
    onFilterUpdated( pFilter );
    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedFilter* RimCellFilterCollection::addNewUserDefinedFilter( RimCase* srcCase )
{
    RimUserDefinedFilter* pFilter = new RimUserDefinedFilter();
    addFilter( pFilter );
    onFilterUpdated( pFilter );
    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RimCellFilterCollection::addNewCellRangeFilter( RimCase* srcCase, int sliceDirection, int defaultSlice )
{
    RimCellRangeFilter* pFilter = new RimCellRangeFilter();
    addFilter( pFilter );
    pFilter->setDefaultValues( sliceDirection, defaultSlice );
    onFilterUpdated( pFilter );
    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::addFilter( RimCellFilter* pFilter )
{
    setAutoName( pFilter );
    m_cellFilters.push_back( pFilter );
    connectToFilterUpdates( pFilter );
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::setAutoName( RimCellFilter* pFilter )
{
    int nPolyFilters  = 1;
    int nRangeFilters = 1;
    int nUserFilters  = 1;

    for ( RimCellFilter* filter : m_cellFilters )
    {
        if ( dynamic_cast<RimCellRangeFilter*>( filter ) ) nRangeFilters++;
        if ( dynamic_cast<RimUserDefinedFilter*>( filter ) ) nUserFilters++;
        if ( dynamic_cast<RimPolygonFilter*>( filter ) ) nPolyFilters++;
    }
    if ( dynamic_cast<RimCellRangeFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "Range Filter %1" ).arg( QString::number( nRangeFilters ) ) );
    }
    else if ( dynamic_cast<RimUserDefinedFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "User Defined Filter %1" ).arg( QString::number( nUserFilters ) ) );
    }
    else if ( dynamic_cast<RimPolygonFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "Polygon Filter %1" ).arg( QString::number( nPolyFilters ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                              std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    onFilterUpdated( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::removeFilter( RimCellFilter* filter )
{
    m_cellFilters.removeChildObject( filter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::connectToFilterUpdates( RimCellFilter* filter )
{
    filter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::onFilterUpdated( const SignalEmitter* emitter )
{
    Rim3dView* view = nullptr;
    firstAncestorOrThisOfType( view );
    if ( !view ) return;

    if ( view->isMasterView() )
    {
        RimViewLinker* viewLinker = view->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateCellFilters( dynamic_cast<const RimCellFilter*>( emitter ) );
        }
    }

    view->scheduleGeometryRegen( VISIBLE_WELL_CELLS );
    view->scheduleGeometryRegen( RANGE_FILTERED );
    view->scheduleGeometryRegen( RANGE_FILTERED_INACTIVE );

    view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// Populate the given view filter with info from our filters
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::compoundCellRangeFilter( cvf::CellRangeFilter* cellRangeFilter, size_t gridIndex ) const
{
    CVF_ASSERT( cellRangeFilter );

    for ( RimCellFilter* filter : m_cellFilters )
    {
        if ( filter->isFilterEnabled() && static_cast<size_t>( filter->gridIndex() ) == gridIndex )
        {
            filter->updateCompundFilter( cellRangeFilter );
        }
    }
}
