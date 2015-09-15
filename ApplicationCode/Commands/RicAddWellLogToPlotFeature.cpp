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

#include "RimWellLasFileInfo.h"
#include "RimWellLog.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrace.h"
#include "RimWellLogFileCurve.h"
#include "RimProject.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RiuWellLogTracePlot.h"

#include "cafSelectionManager.h"

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

    RimWellLogPlot* plot = createWellLogPlot();

    RimWellLogPlotTrace* plotTrace = new RimWellLogPlotTrace();
    plot->addTrace(plotTrace);

    plot->loadDataAndUpdate();

    for (size_t wlIdx = 0; wlIdx < selection.size(); wlIdx++)
    {
        RimWellLog* wellLog = selection[wlIdx];

        RimWellLasFileInfo* lasFileInfo;
        wellLog->firstAnchestorOrThisOfType(lasFileInfo);
        if (lasFileInfo)
        {
            RimWellLogFileCurve* curve = new RimWellLogFileCurve;
            plotTrace->addCurve(curve);

            curve->setCurveData(lasFileInfo->logValues(wellLog->name()), lasFileInfo->depthValues());
            curve->setDescription(wellLog->name());
            curve->updatePlotData();
        }        
    }

    RiaApplication::instance()->project()->updateConnectedEditors();
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection* RicAddWellLogToPlotFeature::mainPlotCollection()
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    if (!mainPlotColl)
    {
        project->recreateMainPlotCollection();
    }

    return project->mainPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RicAddWellLogToPlotFeature::wellLogPlotCollection()
{
    RimMainPlotCollection* mainPlotColl = mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimWellLogPlotCollection* wellLogPlotColl = mainPlotColl->wellLogPlotCollection();
    if (!wellLogPlotColl)
    {
        mainPlotColl->recreateWellLogPlotCollection();
    }

    return mainPlotColl->wellLogPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RicAddWellLogToPlotFeature::createWellLogPlot()
{
    RimWellLogPlotCollection* wellLogPlotColl = wellLogPlotCollection();
    CVF_ASSERT(wellLogPlotColl);

    RimWellLogPlot* plot = new RimWellLogPlot();
    wellLogPlotColl->wellLogPlots().push_back(plot);

    return plot;
}

} // end namespace caf
