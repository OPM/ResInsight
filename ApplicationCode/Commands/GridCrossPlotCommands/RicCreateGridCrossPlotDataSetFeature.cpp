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
#include "RicCreateGridCrossPlotDataSetFeature.h"

#include "RiaGuiApplication.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateGridCrossPlotDataSetFeature, "RicCreateGridCrossPlotDataSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateGridCrossPlotDataSetFeature::isCommandEnabled()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCrossPlotDataSetFeature::onActionTriggered( bool isChecked )
{
    RimGridCrossPlot* crossPlot = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>();

    RimGridCrossPlotDataSet* dataSet = crossPlot->createDataSet();
    dataSet->loadDataAndUpdate( true );

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( dataSet );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCrossPlotDataSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Data Set" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
