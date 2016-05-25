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

#include "RicNewSummaryCurveFeature.h"

#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "RiaApplication.h"

#include <QAction>

#include "cvfAssert.h"
#include "RimSummaryPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimSummaryCurve.h"
#include "RiuMainWindow.h"
#include "cafSelectionManager.h"


CAF_CMD_SOURCE_INIT(RicNewSummaryCurveFeature, "RicNewSummaryCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCurveFeature::isCommandEnabled()
{
    return (selectedSummaryPlot());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();
    CVF_ASSERT(summaryPlotColl);

    RimSummaryPlot* plot = selectedSummaryPlot();
    if (plot)
    {
        RimSummaryCurve* newCurve = new RimSummaryCurve();
        plot->addCurve(newCurve);
        plot->updateConnectedEditors();

        RiuMainWindow::instance()->selectAsCurrentItem(newCurve);

    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Curve");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryCurveFeature::selectedSummaryPlot() const
{
    std::vector<RimSummaryPlot*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}
