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

#include "RicNewWellLogPlotTrackFeature.h"

#include "RicNewWellLogCurveExtractionFeature.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewWellLogPlotTrackFeature, "RicNewWellLogPlotTrackFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogPlotTrackFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ||
        RicWellLogPlotCurveFeatureImpl::parentWellRftPlot())
    {
        return false;
    }
 
    return selectedWellLogPlot() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotTrackFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    RimWellLogPlot* wellLogPlot = selectedWellLogPlot();
    if (wellLogPlot)
    {
         RimWellLogTrack* plotTrack = new RimWellLogTrack;
         wellLogPlot->addTrack(plotTrack);
         plotTrack->setDescription(QString("Track %1").arg(wellLogPlot->trackCount()));

         wellLogPlot->updateConnectedEditors();
         RicWellLogTools::addExtractionCurve(plotTrack, nullptr, nullptr, nullptr, -1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotTrackFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Track");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RicNewWellLogPlotTrackFeature::selectedWellLogPlot()
{
    std::vector<RimWellLogPlot*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}
