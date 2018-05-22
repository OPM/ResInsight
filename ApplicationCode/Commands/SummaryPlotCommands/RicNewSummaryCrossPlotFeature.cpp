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

#include "RicNewSummaryCrossPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaSummaryTools.h"

#include "RicEditSummaryPlotFeature.h"
#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryCrossPlotFeature, "RicNewSummaryCrossPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCrossPlotFeature::isCommandEnabled()
{
    RimSummaryCrossPlotCollection* sumPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlotColl = RiaSummaryTools::parentCrossPlotCollection(selObj);
    }

    if (sumPlotColl) return true;

    // Multiple case selections
    std::vector<caf::PdmUiItem*> selectedItems = caf::selectedObjectsByTypeStrict<caf::PdmUiItem*>();

    for (auto item : selectedItems)
    {
        if (!dynamic_cast<RimSummaryCase*>(item) && !dynamic_cast<RimSummaryCaseCollection*>(item))
            return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimSummaryCrossPlotCollection* summaryCrossPlotColl = project->mainPlotCollection()->summaryCrossPlotCollection();
    RimSummaryPlot* summaryPlot = summaryCrossPlotColl->createSummaryPlot();
    
    summaryCrossPlotColl->addSummaryPlot(summaryPlot);
    if (summaryPlot)
    {
        summaryCrossPlotColl->updateConnectedEditors();
        summaryPlot->loadDataAndUpdate();

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::selectAsCurrentItem(summaryPlot);
        RiuPlotMainWindowTools::setExpanded(summaryPlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Cross Plot");
    actionToSetup->setIcon(QIcon(":/SummaryXPlotLight16x16.png"));
}

