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
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveSwitchAxisFeature::onActionTriggered(bool isChecked)
{
    RimSummaryCurve* summaryCurve = RicSummaryCurveSwitchAxisFeature::selectedSummaryCurve();
    RimSummaryCurveFilter* summaryCurveFilter = RicSummaryCurveSwitchAxisFeature::selectedSummaryCurveFilter();
    if (summaryCurve)
    {
        RimDefines::PlotAxis plotAxis = summaryCurve->associatedPlotAxis();

        if (plotAxis == RimDefines::PLOT_AXIS_LEFT)
        {
            summaryCurve->setPlotAxis(RimDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            summaryCurve->setPlotAxis(RimDefines::PLOT_AXIS_LEFT);
        }

        summaryCurve->updateQwtPlotAxis();
        summaryCurve->updateConnectedEditors();

        RimSummaryPlot* plot = nullptr;
        summaryCurve->firstAncestorOrThisOfType(plot);
        if (plot) plot->updateAxes();
    }
    else if (summaryCurveFilter)
    {
        RimDefines::PlotAxis plotAxis = summaryCurveFilter->associatedPlotAxis();
         
        if (plotAxis == RimDefines::PLOT_AXIS_LEFT)
        {
            summaryCurveFilter->setPlotAxis(RimDefines::PLOT_AXIS_RIGHT);
        }
        else
        {
            summaryCurveFilter->setPlotAxis(RimDefines::PLOT_AXIS_LEFT);
        }

        summaryCurveFilter->updateConnectedEditors();
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
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicSummaryCurveSwitchAxisFeature::selectedSummaryCurve()
{
    std::vector<RimSummaryCurve*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveFilter* RicSummaryCurveSwitchAxisFeature::selectedSummaryCurveFilter()
{
    std::vector<RimSummaryCurveFilter*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : nullptr;
}

