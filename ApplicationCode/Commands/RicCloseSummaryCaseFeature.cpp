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

#include "RicCloseSummaryCaseFeature.h"

#include "RiaApplication.h"
#include "RiaSummaryTools.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicCloseSummaryCaseFeature, "RicCloseSummaryCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseFeature::setupActionLook(QAction* actionToSetup)
{
	actionToSetup->setText("Close Summary Case");
	actionToSetup->setIcon(QIcon(":/Erase.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseFeature::deleteSummaryCases(const std::vector<RimSummaryCase*>& cases)
{
    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    for (RimSummaryCase* summaryCase : cases)
    {
        for (RimSummaryPlot* summaryPlot : summaryPlotColl->summaryPlots)
        {
            summaryPlot->deleteCurvesAssosiatedWithCase(summaryCase);
        }
        summaryPlotColl->updateConnectedEditors();

        RimSummaryCaseMainCollection* summaryCaseMainCollection = nullptr;
        summaryCase->firstAncestorOrThisOfTypeAsserted(summaryCaseMainCollection);

        summaryCaseMainCollection->removeCase(summaryCase);
        delete summaryCase;
        summaryCaseMainCollection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseSummaryCaseFeature::isCommandEnabled()
{
    std::vector<RimSummaryCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() == 0)
    {
        return false;
    }

    for (RimSummaryCase* summaryCase : selection)
    {
        if (summaryCase->isObservedData())
        {
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseFeature::onActionTriggered(bool isChecked)
{
	std::vector<RimSummaryCase*> selection;
	caf::SelectionManager::instance()->objectsByType(&selection);
    CVF_ASSERT(selection.size() > 0);
    
    RicCloseSummaryCaseFeature::deleteSummaryCases(selection);
}
