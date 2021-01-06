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

#include "RimPlotDataFilterCollection.h"
#include "RimPlotDataFilterItem.h"

CAF_PDM_SOURCE_INIT( RimPlotDataFilterCollection, "PlotDataFilterCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotDataFilterCollection::RimPlotDataFilterCollection()
    : filtersChanged( this )
{
    CAF_PDM_InitObject( "Plot Data Filters", ":/AnalysisPlotFilter16x16.png", "", "" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "IsActive", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_filters, "PlotDataFiltersField", "", "", "", "" );
    m_filters.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotDataFilterItem* RimPlotDataFilterCollection::addFilter()
{
    auto newFilter = new RimPlotDataFilterItem();
    m_filters.push_back( newFilter );

    newFilter->updateMaxMinAndDefaultValues( false );
    newFilter->filterChanged.connect( this, &RimPlotDataFilterCollection::onFilterChanged );

    filtersChanged.send();

    return newFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotDataFilterCollection::removeFilter( RimPlotDataFilterItem* filter )
{
    m_filters.removeChildObject( filter );
    delete filter;

    filtersChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotDataFilterItem*> RimPlotDataFilterCollection::filters() const
{
    return m_filters.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotDataFilterCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotDataFilterCollection::objectToggleField()
{
    return &m_isActive;
}

void RimPlotDataFilterCollection::onFilterChanged( const caf::SignalEmitter* emitter )
{
    filtersChanged.send();
}
