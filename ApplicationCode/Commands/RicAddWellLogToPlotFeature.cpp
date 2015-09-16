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

#include "RimWellLasFileInfo.h"
#include "RimWellLog.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrace.h"
#include "RimWellLogFileCurve.h"
#include "RimProject.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigWellLogFile.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RiuWellLogTracePlot.h"

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
    std::vector<RimWellLog*> selection = selectedWellLogs();
    return selection.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddWellLogToPlotFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellLog*> selection = selectedWellLogs();
    if (selection.size() < 1) return;
    
    RimWellLogPlot* plot = RicNewWellLogPlotFeatureImpl::createWellLogPlot();

    RimWellLogPlotTrace* plotTrace = new RimWellLogPlotTrace();
    plot->addTrace(plotTrace);

    plot->loadDataAndUpdate();

    caf::PdmUiItem* uiItem = NULL;

    for (size_t wlIdx = 0; wlIdx < selection.size(); wlIdx++)
    {
        RimWellLog* wellLog = selection[wlIdx];

        RimWellPath* wellPath;
        wellLog->firstAnchestorOrThisOfType(wellPath);

        RimWellLasFileInfo* lasFileInfo;
        wellLog->firstAnchestorOrThisOfType(lasFileInfo);
        if (lasFileInfo)
        {
            size_t curveIdx = plotTrace->curveCount();

            RimWellLogFileCurve* curve = new RimWellLogFileCurve;
            plotTrace->addCurve(curve);

            cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromIndex(curveIdx);
            curve->setColor(curveColor);
            curve->setDescription(wellLog->name());
            curve->setWellPath(wellPath);
            curve->setWellLogChannelName(wellLog->name());

            curve->updatePlotData();

            if (wlIdx == selection.size() - 1)
            {
                uiItem = curve;
            }
        }        
    }

    plot->updateAvailableDepthRange();
    plot->setVisibleDepthRangeFromContents();
    plotTrace->viewer()->replot();

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
    actionToSetup->setText("Add To Plot");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLog*> RicAddWellLogToPlotFeature::selectedWellLogs()
{
    std::vector<RimWellLog*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection;
}

} // end namespace caf
