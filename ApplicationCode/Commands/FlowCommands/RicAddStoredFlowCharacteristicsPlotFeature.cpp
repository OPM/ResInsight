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

#include "RicAddStoredFlowCharacteristicsPlotFeature.h"

#include "RiaApplication.h"

#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimFlowCharacteristicsPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAddStoredFlowCharacteristicsPlotFeature, "RicAddStoredFlowCharacteristicsPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAddStoredFlowCharacteristicsPlotFeature::isCommandEnabled()
{
    if (RiaApplication::instance()->project())
    {
        RimFlowPlotCollection* flowPlotColl = RiaApplication::instance()->project()->mainPlotCollection->flowPlotCollection();
        if (flowPlotColl)
        {
            RimFlowCharacteristicsPlot* flowCharacteristicsPlot = dynamic_cast<RimFlowCharacteristicsPlot*>(caf::SelectionManager::instance()->selectedItem());

            if (flowPlotColl->defaultFlowCharacteristicsPlot() == flowCharacteristicsPlot)
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
void RicAddStoredFlowCharacteristicsPlotFeature::onActionTriggered(bool isChecked)
{
    if (RiaApplication::instance()->project())
    {
        RimFlowPlotCollection* flowPlotColl = RiaApplication::instance()->project()->mainPlotCollection->flowPlotCollection();
        if (flowPlotColl)
        {
            RimFlowCharacteristicsPlot* sourceObject = dynamic_cast<RimFlowCharacteristicsPlot*>(caf::SelectionManager::instance()->selectedItem());

            RimFlowCharacteristicsPlot* flowCharacteristicsPlot = dynamic_cast<RimFlowCharacteristicsPlot*>(sourceObject->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
            CVF_ASSERT(flowCharacteristicsPlot);

            flowPlotColl->addFlowCharacteristicsPlotToStoredPlots(flowCharacteristicsPlot);
            flowCharacteristicsPlot->resolveReferencesRecursively();
            
            flowCharacteristicsPlot->loadDataAndUpdate();

            flowPlotColl->updateConnectedEditors();

            RiuPlotMainWindowTools::showPlotMainWindow();
           
            RiuPlotMainWindowTools::selectAsCurrentItem(flowCharacteristicsPlot);
            RiuPlotMainWindowTools::setExpanded(flowCharacteristicsPlot);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddStoredFlowCharacteristicsPlotFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Add Stored Flow Characteristics Plot");
}
