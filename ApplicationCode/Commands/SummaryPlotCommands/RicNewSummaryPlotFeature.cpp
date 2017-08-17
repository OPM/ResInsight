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

#include "RicNewSummaryPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include "cvfAssert.h"


CAF_CMD_SOURCE_INIT(RicNewSummaryPlotFeature, "RicNewSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();
    CVF_ASSERT(summaryPlotColl);

    RimSummaryCase* summaryCase = nullptr;
    std::vector<RimSummaryCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    if (selection.size() == 1)
    {
        summaryCase = selection[0];
    }
    else
    {
        std::vector<RimSummaryCase*> cases;
        project->allSummaryCases(cases);
        if (cases.size() > 0)
        {
            summaryCase = cases[0];
        }
    }

    createNewSummaryPlot(summaryPlotColl, summaryCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryPlotFeature::createNewSummaryPlot(RimSummaryPlotCollection* summaryPlotColl, RimSummaryCase* summaryCase)
{
    RimSummaryPlot* plot = new RimSummaryPlot();
    summaryPlotColl->summaryPlots().push_back(plot);

    plot->setDescription(QString("Summary Plot %1").arg(summaryPlotColl->summaryPlots.size()));

    RimSummaryCurveFilter* newCurveFilter = new RimSummaryCurveFilter();

    if (summaryCase)
    {
        newCurveFilter->createCurves(summaryCase, RiaApplication::instance()->preferences()->defaultCurveFilter());
    }

    plot->addCurveFilter(newCurveFilter);

    summaryPlotColl->updateConnectedEditors();
    plot->loadDataAndUpdate();

    RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
    if (mainPlotWindow)
    {
        mainPlotWindow->selectAsCurrentItem(newCurveFilter);
        mainPlotWindow->setExpanded(newCurveFilter, true);
    }

    return plot;
}
