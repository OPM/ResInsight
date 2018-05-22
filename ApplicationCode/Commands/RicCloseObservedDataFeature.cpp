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

#include "RicCloseObservedDataFeature.h"

#include "RiaApplication.h"
#include "RiaSummaryTools.h"

#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicCloseObservedDataFeature, "RicCloseObservedDataFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Close");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::deleteObservedData(const std::vector<RimObservedData*>& data)
{
    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    for (RimObservedData* observedData : data)
    {
        for (RimSummaryPlot* summaryPlot : summaryPlotColl->summaryPlots)
        {
            summaryPlot->deleteCurvesAssosiatedWithCase(observedData);
        }
        summaryPlotColl->updateConnectedEditors();

        RimObservedDataCollection* observedDataCollection = nullptr;
        observedData->firstAncestorOrThisOfTypeAsserted(observedDataCollection);
        
        observedDataCollection->removeObservedData(observedData);
        delete observedData;
        observedDataCollection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseObservedDataFeature::isCommandEnabled()
{
    std::vector<RimObservedData*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() == 0)
    {
        return false;
    }
    for (RimObservedData* data : selection)
    {
        if (!data->isObservedData())
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimObservedData*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    CVF_ASSERT(selection.size() > 0);
    
    RicCloseObservedDataFeature::deleteObservedData(selection);
}
