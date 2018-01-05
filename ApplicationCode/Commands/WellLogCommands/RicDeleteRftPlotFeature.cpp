/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicDeleteRftPlotFeature.h"

#include "RimRftPlotCollection.h"
#include "RimWellRftPlot.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeleteRftPlotFeature, "RicDeleteRftPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeleteRftPlotFeature::isCommandEnabled()
{
    std::vector<RimWellRftPlot*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() > 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteRftPlotFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellRftPlot*> selectedPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedPlots);

    if (selectedPlots.size() == 0) return;

    RimWellRftPlot* firstPlot = selectedPlots[0];

    RimRftPlotCollection* rftPlotCollection = nullptr;
    firstPlot->firstAncestorOrThisOfType(rftPlotCollection);
    if (!rftPlotCollection) return;

    for (RimWellRftPlot* plot : selectedPlots)
    {
        rftPlotCollection->removePlot(plot);
        delete plot;
    }

    rftPlotCollection->uiCapability()->updateConnectedEditors();
    //rftPlotCollection->scheduleRedrawAffectedViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteRftPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete RFT Plot");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}
