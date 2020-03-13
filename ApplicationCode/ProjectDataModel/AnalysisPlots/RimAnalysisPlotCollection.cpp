/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-  Equinor ASA
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

#include "RimAnalysisPlotCollection.h"

#include "RimAnalysisPlot.h"

CAF_PDM_SOURCE_INIT( RimAnalysisPlotCollection, "AnalysisPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlotCollection::RimAnalysisPlotCollection()
{
    CAF_PDM_InitObject( "Analysis Plots", ":/AnalysisPlots16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_analysisPlots, "AnalysisPlots", "Analysis Plots", "", "", "" );
    m_analysisPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlotCollection::~RimAnalysisPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot* RimAnalysisPlotCollection::createAnalysisPlot()
{
    RimAnalysisPlot* plot = new RimAnalysisPlot();
    plot->setAsPlotMdiWindow();

    // plot->enableAutoPlotTitle( true );
    m_analysisPlots.push_back( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlotCollection::updateSummaryNameHasChanged()
{
    for ( RimAnalysisPlot* plot : m_analysisPlots )
    {
        plot->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlotCollection::removeSummaryPlot( RimAnalysisPlot* analysisPlot )
{
    m_analysisPlots.removeChildObject( analysisPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimAnalysisPlot*> RimAnalysisPlotCollection::plots()
{
    return m_analysisPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlotCollection::deleteAllChildObjects()
{
    m_analysisPlots.deleteAllChildObjects();
}
