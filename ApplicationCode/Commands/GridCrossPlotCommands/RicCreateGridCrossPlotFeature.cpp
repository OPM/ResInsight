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

#include "RiaGuiApplication.h"

#include "RimEclipseView.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimGridView.h"
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
    RimProject* project      = RiaApplication::instance()->project();
    
    bool launchedFromPlotCollection = true;
    RimGridCrossPlotCollection* collection =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGridCrossPlotCollection>();
    if (!collection)
    {
        collection = project->mainPlotCollection()->gridCrossPlotCollection();
        launchedFromPlotCollection = false;
    }
    RimGridCrossPlot* plot = collection->createGridCrossPlot();    
    RimGridCrossPlotDataSet* dataSet = plot->createDataSet();

    if (!launchedFromPlotCollection)
    {
        dataSet->setCellFilterView(RiaApplication::instance()->activeGridView());
    }
    
    plot->loadDataAndUpdate();
    plot->zoomAll();
    plot->updateConnectedEditors();

    collection->updateAllRequiredEditors();
    RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem(dataSet);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCrossPlotFeature::setupActionLook(QAction* actionToSetup)
{
    RimGridCrossPlotCollection* collection  =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGridCrossPlotCollection>();
    if (!collection)
    {
        actionToSetup->setText("Create Grid Cross Plot from 3d View");
    }
    else
    {
        actionToSetup->setText("Create Grid Cross Plot");     
    }
    actionToSetup->setIcon(QIcon(":/SummaryXPlotsLight16x16.png"));
}
