/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RicCreateGridCrossPlotCurveSetFeature.h"

#include "RiaApplication.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurveSet.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicCreateGridCrossPlotCurveSetFeature, "RicCreateGridCrossPlotCurveSetFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateGridCrossPlotCurveSetFeature::isCommandEnabled()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCrossPlotCurveSetFeature::onActionTriggered(bool isChecked)
{
    RimGridCrossPlot* crossPlot = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>();

    RimGridCrossPlotCurveSet* curveSet = crossPlot->createCurveSet();
    curveSet->loadDataAndUpdate(true);
    
    RiaApplication::instance()->project()->updateConnectedEditors();
    RiaApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem(curveSet);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCrossPlotCurveSetFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Cross Plot Curve Set");
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
}
