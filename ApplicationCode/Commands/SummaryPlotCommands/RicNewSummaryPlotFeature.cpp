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
#include "RiaSummaryTools.h"

#include "RicEditSummaryPlotFeature.h"
#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"

#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "RiuPlotMainWindow.h"

#include "cvfAssert.h"
#include "cafSelectionManagerTools.h"
//#include "cafPdmUiItem.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryPlotFeature, "RicNewSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotFeature::isCommandEnabled()
{
    RimSummaryPlotCollection* sumPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlotColl = RiaSummaryTools::parentSummaryPlotCollection(selObj);
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
void RicNewSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    std::vector<RimSummaryCase*> selectedCases = caf::selectedObjectsByType<RimSummaryCase*>();
    std::vector<RimSummaryCaseCollection*> selectedGroups = caf::selectedObjectsByType<RimSummaryCaseCollection*>();

    // Append grouped cases
    for (auto group : selectedGroups)
    {
        auto groupCases = group->allSummaryCases();
        selectedCases.insert(selectedCases.end(), groupCases.begin(), groupCases.end());
    }

    auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog();

    if (!dialog->isVisible())
    {
        dialog->show();
    }
    else
    {
        dialog->raise();
    }

    dialog->updateFromDefaultCases(selectedCases);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlotLight16x16.png"));
}

