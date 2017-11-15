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

#include "RimGridTimeHistoryCurve.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicSummaryCurveSwitchAxisFeature, "RicSummaryCurveSwitchAxisFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveSwitchAxisFeature::isCommandEnabled()
{
    std::set<RimSummaryCurveFilter*> selectedCurveFilters;
    std::set<RimSummaryCurve*> selectedSoloCurves;
    std::vector<RimGridTimeHistoryCurve*> gridTimeHistoryCurves;

    RicSummaryCurveSwitchAxisFeature::extractSelectedCurveFiltersAndSoloCurves(&selectedCurveFilters,
                                                                               &selectedSoloCurves,
                                                                               &gridTimeHistoryCurves);
    return (   selectedCurveFilters.size()
            || selectedSoloCurves.size()
            || gridTimeHistoryCurves.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveSwitchAxisFeature::onActionTriggered(bool isChecked)
{
    std::set<RimSummaryCurveFilter*> selectedCurveFilters;
    std::set<RimSummaryCurve*> selectedSoloCurves;
    std::vector<RimGridTimeHistoryCurve*> gridTimeHistoryCurves;

    RicSummaryCurveSwitchAxisFeature::extractSelectedCurveFiltersAndSoloCurves(&selectedCurveFilters, 
                                                                               &selectedSoloCurves,
                                                                               &gridTimeHistoryCurves);
    
    for (RimSummaryCurveFilter* summaryCurveFilter: selectedCurveFilters)
    {
        RiaDefines::PlotAxis plotAxis = summaryCurveFilter->associatedPlotAxis();

        if ( plotAxis == RiaDefines::PLOT_AXIS_LEFT )
        {
            summaryCurveFilter->setPlotAxis(RiaDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            summaryCurveFilter->setPlotAxis(RiaDefines::PLOT_AXIS_LEFT);
        }

        summaryCurveFilter->updateConnectedEditors();
    }

    for (RimSummaryCurve* summaryCurve : selectedSoloCurves)
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
void RicSummaryCurveSwitchAxisFeature::extractSelectedCurveFiltersAndSoloCurves(std::set<RimSummaryCurveFilter*>* selectedCurveFilters, 
                                                                                std::set<RimSummaryCurve*>* selectedSoloCurves,
                                                                                std::vector<RimGridTimeHistoryCurve*>* gridTimeHistoryCurves)
{
    selectedSoloCurves->clear();
    {
        std::vector<RimSummaryCurve*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        for (RimSummaryCurve* curve : selection)
        {
            RimSummaryCurveFilter* parentCurveFilter = nullptr;
            curve->firstAncestorOrThisOfType(parentCurveFilter);
            if (!parentCurveFilter)
            {
                selectedSoloCurves->insert(curve);
            }
        }
    }

    selectedCurveFilters->clear();
    {
        std::vector<RimSummaryCurveFilter*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        for (RimSummaryCurveFilter* curveFilter : selection)
        {
            selectedCurveFilters->insert(curveFilter);
        }
    }

    // Read out all time history curves directly from selection manager
    caf::SelectionManager::instance()->objectsByType(gridTimeHistoryCurves);
}
