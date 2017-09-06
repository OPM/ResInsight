/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicReloadSummaryCaseFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicReloadSummaryCaseFeature, "RicReloadSummaryCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicReloadSummaryCaseFeature::isCommandEnabled()
{
    std::vector<RimSummaryCase*> caseSelection = selectedSummaryCases();

    return (caseSelection.size() > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();
    CVF_ASSERT(summaryPlotColl);

    std::vector<RimSummaryCase*> caseSelection = selectedSummaryCases();
    for (RimSummaryCase* summaryCase : caseSelection)
    {
        summaryCase->reloadCase();

        RiaLogging::info(QString("Reloaded data for %1").arg(summaryCase->summaryHeaderFilename()));
    }

    for (RimSummaryPlot* summaryPlot : summaryPlotColl->summaryPlots)
    {
        summaryPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Reload");
    actionToSetup->setIcon(QIcon(":/Refresh-32.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicReloadSummaryCaseFeature::selectedSummaryCases()
{
    std::vector<RimSummaryCase*> caseSelection;
    caf::SelectionManager::instance()->objectsByType(&caseSelection);

    std::vector<RimSummaryCaseCollection*> collectionSelection;
    caf::SelectionManager::instance()->objectsByType(&collectionSelection);

    for (auto sumColl : collectionSelection)
    {
        for (size_t i = 0; i < sumColl->summaryCaseCount(); i++)
        {
            caseSelection.push_back(sumColl->summaryCase(i));
        }
    }

    return caseSelection;
}
