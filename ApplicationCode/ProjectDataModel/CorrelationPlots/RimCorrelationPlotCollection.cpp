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

#include "RimCorrelationPlotCollection.h"

#include "RimCorrelationPlot.h"
#include "RimParameterResultCrossPlot.h"

CAF_PDM_SOURCE_INIT( RimCorrelationPlotCollection, "CorrelationPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlotCollection::RimCorrelationPlotCollection()
{
    CAF_PDM_InitObject( "Correlation Plots", ":/AnalysisPlots16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_correlationPlots, "CorrelationPlots", "Correlation Plots", "", "", "" );
    m_correlationPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlotCollection::~RimCorrelationPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot* RimCorrelationPlotCollection::createCorrelationPlot()
{
    RimCorrelationPlot* plot = new RimCorrelationPlot();
    plot->setAsPlotMdiWindow();

    // plot->enableAutoPlotTitle( true );
    m_correlationPlots.push_back( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot* RimCorrelationPlotCollection::createParameterResultCrossPlot()
{
    RimParameterResultCrossPlot* plot = new RimParameterResultCrossPlot;
    plot->setAsPlotMdiWindow();
    m_correlationPlots.push_back( plot );
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::removePlot( RimAbstractCorrelationPlot* correlationPlot )
{
    m_correlationPlots.removeChildObject( correlationPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimAbstractCorrelationPlot*> RimCorrelationPlotCollection::plots()
{
    return m_correlationPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::deleteAllChildObjects()
{
    m_correlationPlots.deleteAllChildObjects();
}
