/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicSummaryCurveSwitchAxisFeature.h"

#include "RimAsciiDataCurve.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include "cafSelectionManagerTools.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicSummaryCurveSwitchAxisFeature, "RicSummaryCurveSwitchAxisFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveSwitchAxisFeature::isCommandEnabled()
{
    std::vector<RimSummaryCurve*>           summaryCurves;
    std::vector<RimAsciiDataCurve*>         asciiDataCurves;
    std::vector<RimGridTimeHistoryCurve*>   gridTimeHistoryCurves;

    RicSummaryCurveSwitchAxisFeature::extractSelectedCurves(&summaryCurves, &asciiDataCurves, &gridTimeHistoryCurves);
    
    return summaryCurves.size() || asciiDataCurves.size() || gridTimeHistoryCurves.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveSwitchAxisFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryCurve*>           summaryCurves;
    std::vector<RimAsciiDataCurve*>         asciiDataCurves;
    std::vector<RimGridTimeHistoryCurve*>   gridTimeHistoryCurves;

    RicSummaryCurveSwitchAxisFeature::extractSelectedCurves(&summaryCurves, &asciiDataCurves, &gridTimeHistoryCurves);

    for (RimSummaryCurve* summaryCurve : summaryCurves)
    {
        RiaDefines::PlotAxis plotAxis = summaryCurve->axisY();

        if ( plotAxis == RiaDefines::PLOT_AXIS_LEFT )
        {
            summaryCurve->setLeftOrRightAxisY(RiaDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            summaryCurve->setLeftOrRightAxisY(RiaDefines::PLOT_AXIS_LEFT);
        }

        summaryCurve->updateQwtPlotAxis();
        summaryCurve->updateConnectedEditors();

        RimSummaryPlot* plot = nullptr;
        summaryCurve->firstAncestorOrThisOfType(plot);
        if ( plot ) plot->updateAxes();
    }

    for (RimAsciiDataCurve* asciiCurve : asciiDataCurves)
    {
        RiaDefines::PlotAxis plotAxis = asciiCurve->yAxis();

        if (plotAxis == RiaDefines::PLOT_AXIS_LEFT)
        {
            asciiCurve->setYAxis(RiaDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            asciiCurve->setYAxis(RiaDefines::PLOT_AXIS_LEFT);
        }

        asciiCurve->updateQwtPlotAxis();
        asciiCurve->updateConnectedEditors();

        RimSummaryPlot* plot = nullptr;
        asciiCurve->firstAncestorOrThisOfType(plot);
        if (plot) plot->updateAxes();
    }

    for (RimGridTimeHistoryCurve* timeHistoryCurve : gridTimeHistoryCurves)
    {
        RiaDefines::PlotAxis plotAxis = timeHistoryCurve->yAxis();

        if (plotAxis == RiaDefines::PLOT_AXIS_LEFT)
        {
            timeHistoryCurve->setYAxis(RiaDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            timeHistoryCurve->setYAxis(RiaDefines::PLOT_AXIS_LEFT);
        }

        timeHistoryCurve->updateConnectedEditors();

        RimSummaryPlot* plot = nullptr;
        timeHistoryCurve->firstAncestorOrThisOfType(plot);
        if (plot) plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveSwitchAxisFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Switch Plot Axis");
}

//--------------------------------------------------------------------------------------------------
/// Solo curves means selected curves that does not have a selected curve filter as parent 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveSwitchAxisFeature::extractSelectedCurves(std::vector<RimSummaryCurve*>* summaryCurves,
                                                             std::vector<RimAsciiDataCurve*>* asciiDataCurves,
                                                             std::vector<RimGridTimeHistoryCurve*>* gridTimeHistoryCurves)
{
    summaryCurves->clear();
    asciiDataCurves->clear();
    gridTimeHistoryCurves->clear();

    *summaryCurves = caf::selectedObjectsByType<RimSummaryCurve*>();

    *asciiDataCurves = caf::selectedObjectsByType<RimAsciiDataCurve*>();

    *gridTimeHistoryCurves = caf::selectedObjectsByType<RimGridTimeHistoryCurve*>();
}
