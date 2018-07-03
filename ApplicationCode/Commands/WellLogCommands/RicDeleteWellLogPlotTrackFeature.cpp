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

#include "RicDeleteWellLogPlotTrackFeature.h"
#include "RicWellLogPlotCurveFeatureImpl.h"

#include "RiaApplication.h"
#include "RiuPlotMainWindow.h"
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"

#include "RimWellLogTrack.h"
#include "RimWellLogPlot.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicDeleteWellLogPlotTrackFeature, "RicDeleteWellLogPlotTrackFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeleteWellLogPlotTrackFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;

    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        RimWellLogPlot* wellLogPlot = nullptr;
        selection[0]->firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot && wellLogPlot->trackCount() > 1)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteWellLogPlotTrackFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    RiuPlotMainWindow* plotWindow = RiaApplication::instance()->getOrCreateMainPlotWindow();
    std::set<RimWellLogPlot*> alteredWellLogPlots;

    for (size_t i = 0; i < selection.size(); i++)
    {
        RimWellLogTrack* track = selection[i];

        RimWellLogPlot* wellLogPlot = nullptr;
        track->firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot && wellLogPlot->trackCount() > 1)
        {
            alteredWellLogPlots.insert(wellLogPlot);
            wellLogPlot->removeTrack(track);
            caf::SelectionManager::instance()->removeObjectFromAllSelections(track);

            wellLogPlot->updateConnectedEditors();
            delete track;
        }
    }

    for (RimWellLogPlot* wellLogPlot : alteredWellLogPlots)
    {
        RiuWellLogPlot* viewWidget = dynamic_cast<RiuWellLogPlot*>(wellLogPlot->viewWidget());
        plotWindow->setWidthOfMdiWindow(viewWidget, viewWidget->preferredSize().width());
        wellLogPlot->calculateAvailableDepthRange();
        wellLogPlot->updateDepthZoom();
        wellLogPlot->uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteWellLogPlotTrackFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Track");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}
