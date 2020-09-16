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
#include "RimGridCrossPlotCollection.h"

#include "RimGridCrossPlot.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT( RimGridCrossPlotCollection, "RimGridCrossPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCollection::RimGridCrossPlotCollection()
{
    CAF_PDM_InitObject( "Grid Cross Plots", ":/SummaryXPlotsLight16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_gridCrossPlots, "GridCrossPlots", "Grid Cross Plots", "", "", "" );
    m_gridCrossPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCollection::~RimGridCrossPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridCrossPlot*> RimGridCrossPlotCollection::plots() const
{
    return m_gridCrossPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGridCrossPlotCollection::plotCount() const
{
    return m_gridCrossPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot* RimGridCrossPlotCollection::createGridCrossPlot()
{
    RimGridCrossPlot* plot = new RimGridCrossPlot();
    plot->setAsPlotMdiWindow();

    // plot->setDescription(QString("Summary Cross Plot %1").arg(m_gridCrossPlots.size()));
    addPlot( plot );
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCollection::insertPlot( RimGridCrossPlot* plot, size_t index )
{
    m_gridCrossPlots.insert( index, plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCollection::removePlot( RimGridCrossPlot* plot )
{
    m_gridCrossPlots.removeChildObject( plot );
}
