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
#include "RimCellIndexFilter.h"
#include "RimCellRangeFilter.h"
#include "RimPolygonFilter.h"
#include "RimProject.h"
#include "RimUserDefinedFilter.h"
#include "RimUserDefinedIndexFilter.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiLabelEditor.h"

#include "cvfStructGridGeometryGenerator.h"

namespace caf
{
template <>
void caf::AppEnum<RimCellFilterCollection::CombineFilterModeType>::setUp()
{
    addItem( RimCellFilterCollection::AND, "AND", "AND" );
    addItem( RimCellFilterCollection::OR, "OR", "OR" );
    setDefault( RimCellFilterCollection::AND );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCellFilterCollection, "CellFilterCollection", "RimCellFilterCollection", "CellRangeFilterCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterCollection::RimCellFilterCollection()
    : filtersChanged( this )
{
    CAF_PDM_InitScriptableObject( "Cell Filters", ":/CellFilter.png" );

    CAF_PDM_InitScriptableField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_combineFilterMode, "CombineFilterMode", "" );

    CAF_PDM_InitField( &m_combineModeLabel, "CombineModeLabel", QString( "" ), "Combine Polygon and Range Filters Using Operation" );
    m_combineModeLabel.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_combineModeLabel.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_cellFilters, "CellFilters", "Filters" );
    caf::PdmFieldReorderCapability::addToField( &m_cellFilters );

    // for backwards project file compatibility with old CellRangeFilterCollection
    CAF_PDM_InitFieldNoDefault( &m_rangeFilters_OBSOLETE, "RangeFilters", "Range Filters" );
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
    return !m_cellFilters.empty();
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
bool RimCellFilterCollection::useAndOperation() const
{
    return m_combineFilterMode() == RimCellFilterCollection::AND;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::setCase( RimCase* theCase )
{
    for ( RimCellFilter* filter : m_cellFilters )
    {
        filter->setCase( theCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCellFilter*> RimCellFilterCollection::filters() const
{
    return m_cellFilters.childrenByType();
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
        m_rangeFilters_OBSOLETE.removeChild( filter );
        m_cellFilters.push_back( filter );
    }

    // fallback to OR mode for older projects made without AND support
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.12.0" ) )
    {
        m_combineFilterMode = RimCellFilterCollection::OR;
    }

    // Copy by xml serialization does not give a RimCase parent the first time initAfterRead is called here when creating a new a contour
    // view from a 3d view. The second time we get called it is ok, so just skip setting up the filter connections if we have no case.
    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView == nullptr ) return;

    auto rimCase = rimView->ownerCase();
    if ( rimCase == nullptr ) return;

    for ( const auto& filter : m_cellFilters )
    {
        filter->setCase( rimCase );
        filter->filterChanged.connect( this, &RimCellFilterCollection::onFilterUpdated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateIconState();
    uiCapability()->updateConnectedEditors();

    onFilterUpdated( nullptr );

    for ( const auto& filter : m_cellFilters )
    {
        // Update the filters to make sure the 3D polygon targets are removed if the filter collection is disabled
        filter->updateConnectedEditors();
    }
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
void RimCellFilterCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_combineModeLabel );
    uiOrdering.add( &m_combineFilterMode );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    PdmObject::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    auto               rimView        = firstAncestorOrThisOfType<Rim3dView>();
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
void RimCellFilterCollection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiLabelEditorAttribute* myAttr = dynamic_cast<caf::PdmUiLabelEditorAttribute*>( attribute );
    if ( myAttr )
    {
        myAttr->m_useSingleWidgetInsteadOfLabelAndEditorWidget = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::updateIconState()
{
    bool activeIcon = true;

    auto               rimView        = firstAncestorOrThisOfType<Rim3dView>();
    RimViewController* viewController = rimView->viewController();

    bool isControlled = viewController && ( viewController->isCellFiltersControlled() || viewController->isVisibleCellsOveridden() );

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
bool RimCellFilterCollection::hasActiveIncludeIndexFilters() const
{
    if ( !isActive() ) return false;

    for ( const auto& filter : m_cellFilters )
    {
        if ( filter->isFilterEnabled() && filter->isIndexFilter() && filter->filterMode() == RimCellFilter::INCLUDE ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterCollection::hasActiveIncludeRangeFilters() const
{
    if ( !isActive() ) return false;

    for ( const auto& filter : m_cellFilters )
    {
        if ( filter->isFilterEnabled() && filter->isRangeFilter() && filter->filterMode() == RimCellFilter::INCLUDE ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFilter* RimCellFilterCollection::addNewPolygonFilter( RimCase* srcCase, RimPolygon* polygon )
{
    RimPolygonFilter* pFilter = new RimPolygonFilter();
    pFilter->setCase( srcCase );
    pFilter->setPolygon( polygon );
    addFilter( pFilter );
    pFilter->configurePolygonEditor();
    if ( polygon )
    {
        pFilter->enableFilter( true );
    }
    else
    {
        pFilter->enablePicking( true );
    }

    onFilterUpdated( pFilter );
    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedFilter* RimCellFilterCollection::addNewUserDefinedFilter( RimCase* srcCase )
{
    RimUserDefinedFilter* pFilter = new RimUserDefinedFilter();
    pFilter->setCase( srcCase );
    addFilter( pFilter );
    onFilterUpdated( pFilter );
    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedIndexFilter* RimCellFilterCollection::addNewUserDefinedIndexFilter( RimCase* srcCase, const std::vector<size_t>& defCellIndexes )
{
    RimUserDefinedIndexFilter* pFilter = new RimUserDefinedIndexFilter();
    pFilter->setCase( srcCase );
    pFilter->setCellIndexes( defCellIndexes );
    addFilter( pFilter );
    onFilterUpdated( pFilter );
    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::addFilterAndNotifyChanges( RimCellFilter* pFilter, RimCase* srcCase )
{
    addFilter( pFilter );
    pFilter->setCase( srcCase );
    onFilterUpdated( pFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RimCellFilterCollection::addNewCellRangeFilter( RimCase* srcCase, int gridIndex, int sliceDirection, int defaultSlice )
{
    RimCellRangeFilter* pFilter = new RimCellRangeFilter();
    pFilter->setCase( srcCase );
    addFilter( pFilter );
    pFilter->setGridIndex( gridIndex );
    pFilter->setDefaultValues( sliceDirection, defaultSlice );
    onFilterUpdated( pFilter );
    return pFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellIndexFilter* RimCellFilterCollection::addNewCellIndexFilter( RimCase* srcCase )
{
    RimCellIndexFilter* pFilter = new RimCellIndexFilter();
    pFilter->setCase( srcCase );
    addFilter( pFilter );
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
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::setAutoName( RimCellFilter* pFilter )
{
    int nPolyFilters  = 1;
    int nRangeFilters = 1;
    int nUserFilters  = 1;
    int nIndexFilters = 1;

    for ( RimCellFilter* filter : m_cellFilters )
    {
        if ( dynamic_cast<RimCellRangeFilter*>( filter ) ) nRangeFilters++;
        if ( dynamic_cast<RimUserDefinedFilter*>( filter ) ) nUserFilters++;
        if ( dynamic_cast<RimPolygonFilter*>( filter ) ) nPolyFilters++;
        if ( dynamic_cast<RimCellIndexFilter*>( filter ) ) nIndexFilters++;
    }
    if ( dynamic_cast<RimCellRangeFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "Range Filter %1" ).arg( QString::number( nRangeFilters ) ) );
    }
    else if ( dynamic_cast<RimUserDefinedFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "User Defined IJK Filter %1" ).arg( QString::number( nUserFilters ) ) );
    }
    else if ( dynamic_cast<RimPolygonFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "Polygon Filter %1" ).arg( QString::number( nPolyFilters ) ) );
    }
    else if ( dynamic_cast<RimCellIndexFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "Index Filter %1" ).arg( QString::number( nIndexFilters ) ) );
    }
    else if ( dynamic_cast<RimUserDefinedIndexFilter*>( pFilter ) )
    {
        pFilter->setName( QString( "User Defined Index Filter %1" ).arg( QString::number( nIndexFilters ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    onFilterUpdated( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::removeFilter( RimCellFilter* filter )
{
    m_cellFilters.removeChild( filter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::notifyGridReload()
{
    for ( RimCellFilter* filter : m_cellFilters )
    {
        filter->onGridChanged();
    }
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
    auto view = firstAncestorOrThisOfType<Rim3dView>();
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

    filtersChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Populate the given view filter with info from our range filters
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::compoundCellRangeFilter( cvf::CellRangeFilter* cellRangeFilter, size_t gridIndex ) const
{
    CVF_ASSERT( cellRangeFilter );

    int gIndx = static_cast<int>( gridIndex );

    for ( RimCellFilter* filter : m_cellFilters )
    {
        if ( filter->isFilterEnabled() && filter->isRangeFilter() )
        {
            filter->updateCompundFilter( cellRangeFilter, gIndx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Populate the given view filter with info from our cell index filters
/// - includeCellVisibility is set to the visibility from the include filters,
/// - excludeCellVisibility is set to the visibility from the exclude filters
//--------------------------------------------------------------------------------------------------
void RimCellFilterCollection::updateCellVisibilityByIndex( cvf::UByteArray* includeCellVisibility,
                                                           cvf::UByteArray* excludeCellVisibility,
                                                           size_t           gridIndex ) const
{
    CVF_ASSERT( includeCellVisibility );
    CVF_ASSERT( excludeCellVisibility );

    bool needIncludeVisibilityReset = true;

    excludeCellVisibility->setAll( 1 );

    for ( RimCellFilter* filter : m_cellFilters )
    {
        if ( filter->isFilterEnabled() && filter->isIndexFilter() )
        {
            if ( ( filter->filterMode() == RimCellFilter::INCLUDE ) && needIncludeVisibilityReset )
            {
                includeCellVisibility->setAll( 0 );
                needIncludeVisibilityReset = false;
            }

            filter->updateCellIndexFilter( includeCellVisibility, excludeCellVisibility, (int)gridIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonInView*> RimCellFilterCollection::enabledCellFilterPolygons() const
{
    std::vector<RimPolygonInView*> polyInView;

    for ( const auto& filter : m_cellFilters )
    {
        if ( !filter->isActive() ) continue;

        if ( auto polygonFilter = dynamic_cast<RimPolygonFilter*>( filter.p() ) )
        {
            polyInView.push_back( polygonFilter->polygonInView() );
        }
    }

    return polyInView;
}
