/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "RimSummaryPlotCollection.h"

#include "RimSummaryPlot.h"


CAF_PDM_SOURCE_INIT(RimSummaryPlotCollection, "SummaryPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection::RimSummaryPlotCollection()
{
    CAF_PDM_InitObject("Summary Plots", ":/SummaryPlotsLight16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&summaryPlots, "SummaryPlots", "Summary Plots",  "", "", "");
    summaryPlots.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection::~RimSummaryPlotCollection()
{
    summaryPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimSummaryPlotCollection::createSummaryPlotWithAutoTitle()
{
    RimSummaryPlot* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle(true);
    summaryPlots.push_back(plot);

    return plot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimSummaryPlotCollection::createNamedSummaryPlot(const QString& name)
{
    RimSummaryPlot* plot = new RimSummaryPlot();
    summaryPlots.push_back(plot);
    plot->setDescription(name);

    return plot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotCollection::updateSummaryNameHasChanged()
{
    for (RimSummaryPlot* plot : summaryPlots)
    {
        plot->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotCollection::summaryPlotItemInfos(QList<caf::PdmOptionItemInfo>* optionInfos) const
{
    for (RimSummaryPlot* plot : summaryPlots())
    {
        QIcon icon = plot->uiCapability()->uiIcon();
        QString displayName = plot->description();

        optionInfos->push_back(caf::PdmOptionItemInfo(displayName, plot, false, icon));
    }
}

