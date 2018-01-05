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

#include "RicDeletePltPlotFeature.h"

#include "RimPltPlotCollection.h"
#include "RimWellPltPlot.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeletePltPlotFeature, "RicDeletePltPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeletePltPlotFeature::isCommandEnabled()
{
    std::vector<RimWellPltPlot*> objects;
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
void RicDeletePltPlotFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPltPlot*> selectedPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedPlots);

    if (selectedPlots.size() == 0) return;

    RimWellPltPlot* firstPlot = selectedPlots[0];

    RimPltPlotCollection* pltPlotCollection = nullptr;
    firstPlot->firstAncestorOrThisOfType(pltPlotCollection);
    if (!pltPlotCollection) return;

    for (RimWellPltPlot* plot : selectedPlots)
    {
        pltPlotCollection->removePlot(plot);
        delete plot;
    }

    pltPlotCollection->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeletePltPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete PLT Plot");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}
