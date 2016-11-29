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

    RicSummaryCurveSwitchAxisFeature::extractSelectedCurveFiltersAndSoloCurves(&selectedCurveFilters,
                                                                               &selectedSoloCurves);
    return (selectedCurveFilters.size() || selectedSoloCurves.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveSwitchAxisFeature::onActionTriggered(bool isChecked)
{
    std::set<RimSummaryCurveFilter*> selectedCurveFilters;
    std::set<RimSummaryCurve*> selectedSoloCurves;

    RicSummaryCurveSwitchAxisFeature::extractSelectedCurveFiltersAndSoloCurves(&selectedCurveFilters, 
                                                                               &selectedSoloCurves);
    
    for (RimSummaryCurveFilter* summaryCurveFilter: selectedCurveFilters)
    {
        RimDefines::PlotAxis plotAxis = summaryCurveFilter->associatedPlotAxis();

        if ( plotAxis == RimDefines::PLOT_AXIS_LEFT )
        {
            summaryCurveFilter->setPlotAxis(RimDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            summaryCurveFilter->setPlotAxis(RimDefines::PLOT_AXIS_LEFT);
        }

        summaryCurveFilter->updateConnectedEditors();
    }

    for (RimSummaryCurve* summaryCurve : selectedSoloCurves)
    {
        RimDefines::PlotAxis plotAxis = summaryCurve->yAxis();

        if ( plotAxis == RimDefines::PLOT_AXIS_LEFT )
        {
            summaryCurve->setYAxis(RimDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            summaryCurve->setYAxis(RimDefines::PLOT_AXIS_LEFT);
        }

        summaryCurve->updateQwtPlotAxis();
        summaryCurve->updateConnectedEditors();

        RimSummaryPlot* plot = nullptr;
        summaryCurve->firstAncestorOrThisOfType(plot);
        if ( plot ) plot->updateAxes();
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
/// Solo curves means selected curves that does not have a selected curvefilter as parent 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveSwitchAxisFeature::extractSelectedCurveFiltersAndSoloCurves(std::set<RimSummaryCurveFilter*>* selectedCurveFilters, 
                                                                                std::set<RimSummaryCurve*>* selectedSoloCurves)
{

    selectedSoloCurves->clear();
    {
        std::vector<RimSummaryCurve*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        for (RimSummaryCurve* curve: selection)
        {
            selectedSoloCurves->insert(curve);
        }
    }

    selectedCurveFilters->clear();
    {
        std::vector<RimSummaryCurveFilter*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        for ( RimSummaryCurveFilter* curveFilter: selection )
        {
            selectedCurveFilters->insert(curveFilter);
        }
    }

    std::vector<RimSummaryCurve*> curvesToRemove;

    for (RimSummaryCurve* curve: (*selectedSoloCurves))
    {
        RimSummaryCurveFilter* parentCurveFilter;
        curve->firstAncestorOrThisOfType(parentCurveFilter);
        if (parentCurveFilter)
        {
            if (selectedCurveFilters->count(parentCurveFilter) > 0 )
            {
                curvesToRemove.push_back(curve);
            }
        }
    }

    for  (RimSummaryCurve* curve: curvesToRemove)
    {
        selectedSoloCurves->erase(curve);
    }

}