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

#include "RicAddWellLogToPlotFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellLogFileCurve.h"
#include "RimProject.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigWellLogFile.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RiuWellLogTrack.h"

#include "cafSelectionManager.h"
#include "cafPdmUiTreeView.h"

#include <QAction>

namespace caf
{
    CAF_CMD_SOURCE_INIT(RicAddWellLogToPlotFeature, "RicAddWellLogToPlotFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAddWellLogToPlotFeature::isCommandEnabled()
{
    std::vector<RimWellLogFileChannel*> selection = selectedWellLogs();
    return selection.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddWellLogToPlotFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellLogFileChannel*> selection = selectedWellLogs();
    if (selection.size() < 1) return;
    
    RimWellLogPlot* plot = RicNewWellLogPlotFeatureImpl::createWellLogPlot();

    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addTrack(plotTrack);
    plotTrack->setDescription(QString("Track %1").arg(plot->trackCount()));

    plot->loadDataAndUpdate();

    caf::PdmUiItem* uiItem = NULL;

    for (size_t wlIdx = 0; wlIdx < selection.size(); wlIdx++)
    {
        RimWellLogFileChannel* wellLog = selection[wlIdx];

        RimWellPath* wellPath = NULL;
        wellLog->firstAnchestorOrThisOfType(wellPath);

        RimWellLogFile* wellLogFile = NULL;
        wellLog->firstAnchestorOrThisOfType(wellLogFile);
        if (wellLogFile)
        {
            RimWellLogFileCurve* curve = new RimWellLogFileCurve;
            plotTrack->addCurve(curve);

            RigWellLogFile* wellLogDataFile = wellLogFile->wellLogFile();
            CVF_ASSERT(wellLogDataFile);

            if (wlIdx == 0)
            {
                // Initialize plot with depth unit from the first log file
                plot->setDepthUnit(wellLogDataFile->depthUnit());
            }

            cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable();
            curve->setColor(curveColor);
            curve->setWellPath(wellPath);
            curve->setWellLogChannelName(wellLog->name());

            curve->updatePlotData();

            if (wlIdx == selection.size() - 1)
            {
                uiItem = curve;
            }
        }        
    }

    plot->calculateAvailableDepthRange();
    plot->zoomAllDepth();
    plotTrack->viewer()->replot();

    RiaApplication::instance()->project()->updateConnectedEditors();

    if (uiItem)
    {
        RiuMainWindow::instance()->projectTreeView()->selectAsCurrentItem(uiItem);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddWellLogToPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Add To New Plot");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFileChannel*> RicAddWellLogToPlotFeature::selectedWellLogs()
{
    std::vector<RimWellLogFileChannel*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection;
}

} // end namespace caf
