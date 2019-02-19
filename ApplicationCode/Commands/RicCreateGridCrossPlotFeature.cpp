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
#include "RicCreateGridCrossPlotFeature.h"

#include "RiaApplication.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicCreateGridCrossPlotFeature, "RicCreateGridCrossPlotFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateGridCrossPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCrossPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    RimGridCrossPlotCollection* collection = project->mainPlotCollection()->gridCrossPlotCollection();    
    RimGridCrossPlot* plot = collection->createGridCrossPlot();
    
    plot->loadDataAndUpdate();
    plot->updateConnectedEditors();

    RiaApplication::instance()->project()->updateConnectedEditors();
    RiaApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem(plot);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCrossPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Grid Cross Plot");
    actionToSetup->setIcon(QIcon(":/SummaryXPlotsLight16x16.png"));
}
