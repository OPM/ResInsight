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

#include "RimSummaryPlot.h"
#include "RimSummaryCrossPlot.h"


CAF_PDM_SOURCE_INIT(RimSummaryCrossPlotCollection, "SummaryCrossPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection::RimSummaryCrossPlotCollection()
{
    CAF_PDM_InitObject("Summary Cross Plots", ":/SummaryXPlotsLight16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCrossPlots, "SummaryCrossPlots", "Summary Cross Plots",  "", "", "");
    m_summaryCrossPlots.uiCapability()->setUiHidden(true);
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
void RimSummaryCrossPlotCollection::deleteAllChildObjects()
{
    m_summaryCrossPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimSummaryCrossPlotCollection::summaryPlots() const
{
    return m_summaryCrossPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::updateSummaryNameHasChanged()
{
    for (RimSummaryPlot* plot : m_summaryCrossPlots)
    {
        plot->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::summaryPlotItemInfos(QList<caf::PdmOptionItemInfo>* optionInfos) const
{
    for (RimSummaryPlot* plot : m_summaryCrossPlots())
    {
        QIcon icon = plot->uiCapability()->uiIcon();
        QString displayName = plot->description();

        optionInfos->push_back(caf::PdmOptionItemInfo(displayName, plot, false, icon));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimSummaryCrossPlotCollection::createSummaryPlot()
{
    RimSummaryPlot* plot = new RimSummaryCrossPlot();

    plot->setDescription(QString("Summary Cross Plot %1").arg(m_summaryCrossPlots.size()));

    return plot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::addSummaryPlot(RimSummaryPlot *plot)
{
    m_summaryCrossPlots().push_back(plot);
}
