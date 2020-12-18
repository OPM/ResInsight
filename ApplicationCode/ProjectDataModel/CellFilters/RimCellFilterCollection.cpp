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

#include "RigPolyLinesData.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilter.h"
#include "RimCellRangeFilter.h"
#include "RimGeoMechView.h"
#include "RimPolylineFilter.h"
#include "RimUserDefinedFilter.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cvfStructGridGeometryGenerator.h"

CAF_PDM_SOURCE_INIT( RimCellFilterCollection, "RimCellFilterCollection" );

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
std::vector<RimCellFilter*> RimCellFilterCollection::filters() const
{
    return m_cellFilters.childObjects();
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
        if ( filter->isActive() ) return true;
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
        if ( filter->isActive() && filter->filterMode() == RimCellFilter::INCLUDE ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineFilter* RimCellFilterCollection::addNewPolylineFilter( RimCase* srcCase )
{
    RimPolylineFilter* pFilter = new RimPolylineFilter();
    pFilter->setCase( srcCase );
    m_cellFilters.push_back( pFilter );
    pFilter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );

    this->updateConnectedEditors();
    onFilterUpdated( pFilter );

    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedFilter* RimCellFilterCollection::addNewUserDefinedFilter( RimCase* srcCase )
{
    RimUserDefinedFilter* pFilter = new RimUserDefinedFilter();
    m_cellFilters.push_back( pFilter );
    pFilter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );

    this->updateConnectedEditors();
    onFilterUpdated( pFilter );

    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RimCellFilterCollection::addNewCellRangeFilter( RimCase* srcCase, int sliceDirection )
{
    RimCellRangeFilter* pFilter = new RimCellRangeFilter();
    m_cellFilters.push_back( pFilter );
    pFilter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );
    pFilter->setDefaultValues( sliceDirection );

    this->updateConnectedEditors();
    onFilterUpdated( pFilter );

    return pFilter;
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
            // TODO - more generic update here!?
            // Update data for cell filter
            // Update of display model is handled by view->scheduleGeometryRegen, also for managed views
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
        if ( filter->isActive() && static_cast<size_t>( filter->gridIndex() ) == gridIndex )
        {
            filter->updateCompundFilter( cellRangeFilter );
        }
    }
}
