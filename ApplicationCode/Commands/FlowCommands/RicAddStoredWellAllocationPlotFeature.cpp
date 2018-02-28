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

#include "RicAddStoredWellAllocationPlotFeature.h"

#include "RiaApplication.h"

#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellAllocationPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAddStoredWellAllocationPlotFeature, "RicAddStoredWellAllocationPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAddStoredWellAllocationPlotFeature::isCommandEnabled()
{
    if (RiaApplication::instance()->project())
    {
        RimFlowPlotCollection* flowPlotColl = RiaApplication::instance()->project()->mainPlotCollection->flowPlotCollection();
        if (flowPlotColl)
        {
            RimWellAllocationPlot* wellAllocationPlot = dynamic_cast<RimWellAllocationPlot*>(caf::SelectionManager::instance()->selectedItem());

            if (flowPlotColl->defaultWellAllocPlot() == wellAllocationPlot)
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddStoredWellAllocationPlotFeature::onActionTriggered(bool isChecked)
{
    if (RiaApplication::instance()->project())
    {
        RimFlowPlotCollection* flowPlotColl = RiaApplication::instance()->project()->mainPlotCollection->flowPlotCollection();
        if (flowPlotColl)
        {
            RimWellAllocationPlot* sourceObject = dynamic_cast<RimWellAllocationPlot*>(caf::SelectionManager::instance()->selectedItem());

            RimWellAllocationPlot* wellAllocationPlot = dynamic_cast<RimWellAllocationPlot*>(sourceObject->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
            CVF_ASSERT(wellAllocationPlot);

            flowPlotColl->addWellAllocPlotToStoredPlots(wellAllocationPlot);
            wellAllocationPlot->resolveReferencesRecursively();
            
            wellAllocationPlot->loadDataAndUpdate();

            flowPlotColl->updateConnectedEditors();

            RiuPlotMainWindowTools::selectAsCurrentItem(wellAllocationPlot);
            RiuPlotMainWindowTools::setExpanded(wellAllocationPlot);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddStoredWellAllocationPlotFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Add Stored Well Allocation Plot");
}
