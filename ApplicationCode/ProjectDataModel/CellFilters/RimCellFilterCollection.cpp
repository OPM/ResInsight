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
bool RimCellFilterCollection::isEmpty()
{
    return m_cellFilters.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterCollection::isActive()
{
    return m_isActive();
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

    // RimGeoMechView* view = nullptr;
    // this->firstAncestorOrThisOfType( view );
    // if ( view )
    //{
    //    RimViewController* viewController = view->viewController();
    //    if ( viewController && ( viewController->isPropertyFilterOveridden() ||
    //    viewController->isVisibleCellsOveridden() ) )
    //    {
    //        activeIcon = false;
    //    }
    //}

    if ( !m_isActive )
    {
        activeIcon = false;
    }

    updateUiIconFromState( activeIcon );

    for ( auto& filter : m_cellFilters )
    {
        // filter->updateActiveState();
        filter->updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilter* RimCellFilterCollection::addNewPolylineFilter( RimCase* srcCase )
{
    RimPolylineFilter* pFilter = new RimPolylineFilter();
    m_cellFilters.push_back( pFilter );
    pFilter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );

    this->updateConnectedEditors();

    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilter* RimCellFilterCollection::addNewUserDefinedFilter( RimCase* srcCase )
{
    RimUserDefinedFilter* pFilter = new RimUserDefinedFilter();
    m_cellFilters.push_back( pFilter );
    pFilter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );

    this->updateConnectedEditors();

    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilter* RimCellFilterCollection::addNewCellRangeFilter( RimCase* srcCase )
{
    RimCellRangeFilter* pFilter = new RimCellRangeFilter();
    m_cellFilters.push_back( pFilter );
    pFilter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );
    pFilter->setDefaultValues();

    this->updateConnectedEditors();

    return pFilter;
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
            // Update data for range filter
            // Update of display model is handled by view->scheduleGeometryRegen, also for managed views
            viewLinker->updateRangeFilters( nullptr );
        }
    }

    view->scheduleGeometryRegen( VISIBLE_WELL_CELLS );
    view->scheduleGeometryRegen( RANGE_FILTERED );
    view->scheduleGeometryRegen( RANGE_FILTERED_INACTIVE );

    view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// RimCellRangeFilter is using Eclipse 1-based indexing, adjust filter values before
//  populating cvf::CellRangeFilter (which is 0-based)
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
