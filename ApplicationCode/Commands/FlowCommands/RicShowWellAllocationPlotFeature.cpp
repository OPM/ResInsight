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
#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "Rim3dView.h"
#include "RimWellAllocationPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include "RimWellPath.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"

CAF_CMD_SOURCE_INIT(RicShowWellAllocationPlotFeature, "RicShowWellAllocationPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowWellAllocationPlotFeature::isCommandEnabled()
{
    std::vector<RimSimWellInView*> simWellCollection;
    caf::SelectionManager::instance()->objectsByType(&simWellCollection);

    if (simWellCollection.size() > 0)
    {
        return true;
    }

    std::vector<RimWellPath*> wellPathCollection;
    caf::SelectionManager::instance()->objectsByType(&wellPathCollection);

    if (wellPathCollection.empty()) return false;
    
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return false;
    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
    if (!eclView) return false;

    RimSimWellInView* simWellFromWellPath = eclView->wellCollection->findWell(wellPathCollection[0]->associatedSimulationWellName());

    if (simWellFromWellPath)
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
    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    std::vector<RimWellPath*> wellPathCollection;
    caf::SelectionManager::instance()->objectsByType(&wellPathCollection);

    RimSimWellInView* simWell = nullptr;

    if (collection.size() > 0)
    {
        simWell = collection[0];
    }
    else if (wellPathCollection.size() > 0)
    {
        Rim3dView* view = RiaApplication::instance()->activeReservoirView();
        if (!view) return;
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
        if (!eclView) return;

        simWell = eclView->wellCollection->findWell(wellPathCollection[0]->associatedSimulationWellName());
        if (!simWell) return;
    }
    else return;

    if (RiaApplication::instance()->project())
    {
        RimFlowPlotCollection* flowPlotColl = RiaApplication::instance()->project()->mainPlotCollection->flowPlotCollection();
        if (flowPlotColl)
        {
            flowPlotColl->defaultWellAllocPlot()->setFromSimulationWell(simWell);
            flowPlotColl->defaultWellAllocPlot()->updateConnectedEditors();

            // Make sure the summary plot window is created and visible
            RiuPlotMainWindowTools::showPlotMainWindow();
            RiuPlotMainWindowTools::selectAsCurrentItem(flowPlotColl->defaultWellAllocPlot());
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
