/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimSummaryCrossPlotCollection.h"

#include "RimProject.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryPlot.h"

#include "cafPdmFieldReorderCapability.h"

CAF_PDM_SOURCE_INIT( RimSummaryCrossPlotCollection, "SummaryCrossPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection::RimSummaryCrossPlotCollection()
{
    CAF_PDM_InitObject( "Summary Cross Plots", ":/SummaryXPlotsLight16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCrossPlots, "SummaryCrossPlots", "Summary Cross Plots", "", "", "" );
    m_summaryCrossPlots.uiCapability()->setUiTreeHidden( true );
    caf::PdmFieldReorderCapability::addToField( &m_summaryCrossPlots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection::~RimSummaryCrossPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::deleteAllPlots()
{
    m_summaryCrossPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimSummaryCrossPlotCollection::plots() const
{
    return m_summaryCrossPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryCrossPlotCollection::plotCount() const
{
    return m_summaryCrossPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::updateSummaryNameHasChanged()
{
    for ( RimSummaryPlot* plot : m_summaryCrossPlots )
    {
        plot->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const
{
    for ( RimSummaryPlot* plot : m_summaryCrossPlots() )
    {
        caf::IconProvider icon        = plot->uiCapability()->uiIconProvider();
        QString           displayName = plot->description();

        optionInfos->push_back( caf::PdmOptionItemInfo( displayName, plot, false, icon ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::insertPlot( RimSummaryPlot* plot, size_t index )
{
    m_summaryCrossPlots.insert( index, plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::removePlot( RimSummaryPlot* plot )
{
    m_summaryCrossPlots.removeChildObject( plot );
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimSummaryCrossPlotCollection::createSummaryPlot()
{
    RimSummaryPlot* plot = new RimSummaryCrossPlot();
    plot->setAsPlotMdiWindow();

    plot->setDescription( QString( "Summary Cross Plot %1" ).arg( m_summaryCrossPlots.size() ) );

    return plot;
}
