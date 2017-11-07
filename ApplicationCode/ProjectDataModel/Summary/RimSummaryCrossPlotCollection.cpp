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


CAF_PDM_SOURCE_INIT(RimSummaryCrossPlotCollection, "SummaryCrossPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection::RimSummaryCrossPlotCollection()
{
    CAF_PDM_InitObject("Summary Cross Plots", ":/SummaryPlots16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&summaryCrossPlots, "SummaryCrossPlots", "Summary Cross Plots",  "", "", "");
    summaryCrossPlots.uiCapability()->setUiHidden(true);
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
    summaryCrossPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimSummaryCrossPlotCollection::summaryPlots() const
{
    return summaryCrossPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::updateSummaryNameHasChanged()
{
    for (RimSummaryPlot* plot : summaryCrossPlots)
    {
        plot->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCrossPlotCollection::summaryPlotItemInfos(QList<caf::PdmOptionItemInfo>* optionInfos) const
{
    for (RimSummaryPlot* plot : summaryCrossPlots())
    {
        QIcon icon = plot->uiCapability()->uiIcon();
        QString displayName = plot->description();

        optionInfos->push_back(caf::PdmOptionItemInfo(displayName, plot, false, icon));
    }
}

