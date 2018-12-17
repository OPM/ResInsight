/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicClearSourceSteppingSummaryCurveFeature.h"
#include "RicClearSourceSteppingEnsembleCurveSetFeature.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicClearSourceSteppingSummaryCurveFeature, "RicClearSourceSteppingSummaryCurveFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicClearSourceSteppingSummaryCurveFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicClearSourceSteppingSummaryCurveFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryCurve*> summaryCurves;
    caf::SelectionManager::instance()->objectsByType(&summaryCurves);

    if (summaryCurves.size() == 1)
    {
        auto c = summaryCurves[0];

        RimSummaryPlot* summaryPlot = nullptr;
        c->firstAncestorOrThisOfType(summaryPlot);
        if (summaryPlot)
        {
            RicClearSourceSteppingEnsembleCurveSetFeature::clearAllSourceSteppingInSummaryPlot(summaryPlot);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicClearSourceSteppingSummaryCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Clear Source Stepping Curve");
    actionToSetup->setIcon(QIcon(":/updownarrow.png"));
}
