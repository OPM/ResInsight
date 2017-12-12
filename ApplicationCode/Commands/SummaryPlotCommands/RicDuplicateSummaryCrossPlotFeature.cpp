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

#include "RicDuplicateSummaryCrossPlotFeature.h"

#include "RiaSummaryTools.h"

#include "RicPasteSummaryCrossPlotFeature.h"

#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"

#include "cvfAssert.h"
#include "cafSelectionManagerTools.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicDuplicateSummaryCrossPlotFeature, "RicDuplicateSummaryCrossPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDuplicateSummaryCrossPlotFeature::isCommandEnabled()
{
    RimSummaryCrossPlotCollection* sumPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlotColl = RiaSummaryTools::parentCrossPlotCollection(selObj);
    }

    if (sumPlotColl) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryCrossPlotFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryCrossPlot*> selectedObjects = caf::selectedObjectsByType<RimSummaryCrossPlot*>();

    if (selectedObjects.size() == 1)
    {
        RicPasteSummaryCrossPlotFeature::copyPlotAndAddToCollection(selectedObjects[0]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryCrossPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Duplicate Summary Cross Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}

