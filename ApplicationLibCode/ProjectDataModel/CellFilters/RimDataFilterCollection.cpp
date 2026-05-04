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

#include "RimDataFilterCollection.h"

#include "RimCase.h"
#include "RimCellRangeFilter.h"
#include "RimCombinedFilter.h"
#include "RimEclipsePropertyFilter.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimDataFilterCollection, "DataFilterCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterCollection::RimDataFilterCollection()
    : filtersChanged( this )
{
    CAF_PDM_InitObject( "Data Filters", ":/CellFilter.png" );

    CAF_PDM_InitFieldNoDefault( &m_items, "Filters", "Filters" );

    CAF_PDM_InitFieldNoDefault( &m_srcCase, "SourceCase", "Source Case" );
    m_srcCase.uiCapability()->setUiHidden( true );
    m_srcCase.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterCollection::~RimDataFilterCollection() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::setCase( RimCase* srcCase )
{
    m_srcCase = srcCase;
    for ( RimCellFilter* child : items() )
    {
        if ( child ) child->setCase( srcCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimDataFilterCollection::ownerCase() const
{
    if ( m_srcCase() ) return m_srcCase();
    return firstAncestorOfType<RimCase>();
}

//--------------------------------------------------------------------------------------------------
/// Programmatic removal path: deleteItem in the templated base does not call onItemsChanged, so
/// we send filtersChanged ourselves. UI-driven detachment routes through onChildDeleted instead.
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::removeFilter( RimCellFilter* f )
{
    if ( !f ) return;
    deleteItem( f );
    filtersChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter* RimDataFilterCollection::addNewPropertyFilter()
{
    auto* propertyFilter = new RimEclipsePropertyFilter();
    addFilter( propertyFilter );
    return propertyFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RimDataFilterCollection::addNewRangeFilter()
{
    // No setDefaultValues call: it asserts on a Rim3dView ancestor, which the case-level
    // collection does not have. Constructor defaults (1,1,1,1,1,1) are used as-is.
    auto* rangeFilter = new RimCellRangeFilter();
    addFilter( rangeFilter );
    return rangeFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCombinedFilter* RimDataFilterCollection::addNewCombinedFilter()
{
    auto* combined = new RimCombinedFilter();
    addFilter( combined );
    return combined;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataFilterCollection::hasActiveFilters() const
{
    for ( RimCellFilter* filter : items() )
    {
        if ( filter && filter->isFilterEnabled() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Hook from caf::PdmObjectCollection<T>: invoked after addItem / insertItem and on UI-driven
/// changes to m_items. caf::Signal::connect silently skips a re-connect when the same observer
/// is already registered (cafSignal.h:141), so iterating over all items each time is safe.
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::onItemsChanged()
{
    for ( RimCellFilter* child : items() )
    {
        connectChildSignal( child );
        if ( child && m_srcCase() ) child->setCase( m_srcCase() );
    }
    filtersChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Hook from caf::PdmObjectCollection<T>: PDM invokes this after a child is detached from
/// m_items. Forward as filtersChanged so view-side wrappers reconcile.
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                              std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateConnectedEditors();
    filtersChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::initAfterRead()
{
    for ( RimCellFilter* child : items() )
    {
        connectChildSignal( child );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicEclipsePropertyFilterNewFeature";
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
    menuBuilder << "Separator";
    menuBuilder << "RicEclipseCombinedPropertyFilterNewFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::connectChildSignal( RimCellFilter* child )
{
    if ( !child ) return;
    child->filterChanged.connect( this, &RimDataFilterCollection::onChildFilterChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterCollection::onChildFilterChanged( const caf::SignalEmitter* /*emitter*/ )
{
    filtersChanged.send();
}
