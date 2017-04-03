/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicShowWellAllocationPlotFeature.h"

#include "RiaApplication.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseWell.h"
#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimWellAllocationPlot.h"

#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowWellAllocationPlotFeature, "RicShowWellAllocationPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowWellAllocationPlotFeature::isCommandEnabled()
{
    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    if (collection.size() > 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowWellAllocationPlotFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    if (collection.size() > 0)
    {
        RimEclipseWell* eclWell = collection[0];

        if (RiaApplication::instance()->project())
        {
            RimFlowPlotCollection* flowPlotColl = RiaApplication::instance()->project()->mainPlotCollection->flowPlotCollection();
            if (flowPlotColl)
            {
                flowPlotColl->defaultWellAllocPlot()->setFromSimulationWell(eclWell);
                flowPlotColl->defaultWellAllocPlot()->updateConnectedEditors();

                // Make sure the summary plot window is created and visible
                RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
                plotwindow->selectAsCurrentItem(flowPlotColl->defaultWellAllocPlot());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowWellAllocationPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/WellAllocPlot16x16.png"));
    actionToSetup->setText("Plot Well Allocation");
}
