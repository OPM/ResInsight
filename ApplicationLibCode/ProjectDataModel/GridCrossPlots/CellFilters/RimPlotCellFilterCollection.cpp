/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotCellFilterCollection.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"

CAF_PDM_SOURCE_INIT( RimPlotCellFilterCollection, "RimPlotCellFilterCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCellFilterCollection::RimPlotCellFilterCollection()
{
    CAF_PDM_InitObject( "Plot Cell Filters", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_cellFilters, "CellFilters", "Cell Filters", "", "", "" );
    m_cellFilters.uiCapability()->setUiHidden( true );

    setName( "Filter Collection" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellFilterCollection::addCellFilter( RimPlotCellFilter* cellFilter )
{
    m_cellFilters.push_back( cellFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimPlotCellFilterCollection::cellFilterCount() const
{
    return m_cellFilters.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellFilterCollection::computeCellVisibilityFromFilter( size_t timeStepIndex, cvf::UByteArray* cellVisibility )
{
    if ( isChecked() )
    {
        updateCellVisibilityFromFilter( timeStepIndex, cellVisibility );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellFilterCollection::setCase( RimCase* gridCase )
{
    RimEclipseResultCase* eclipseResultCase = dynamic_cast<RimEclipseResultCase*>( gridCase );
    if ( eclipseResultCase )
    {
        std::vector<RimEclipseResultDefinition*> resultDefinitions;

        this->descendantsIncludingThisOfType( resultDefinitions );
        for ( auto r : resultDefinitions )
        {
            r->setEclipseCase( eclipseResultCase );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellFilterCollection::updateCellVisibilityFromFilter( size_t timeStepIndex, cvf::UByteArray* cellVisibility )
{
    for ( RimPlotCellFilter* f : m_cellFilters() )
    {
        if ( f->isChecked() )
        {
            f->updateCellVisibility( timeStepIndex, cellVisibility );
        }
    }
}
