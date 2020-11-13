/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RicCreateGridStatisticsPlotFeature.h"
//#include "RicGridStatisticsUi.h"

#include "RiaGuiApplication.h"
// #include "RiaLogging.h"
// #include "RiaPorosityModel.h"

// #include "RigCaseCellResultsData.h"
// #include "RigEclipseCaseData.h"
// #include "RigEclipseResultAddress.h"
// #include "RigEquil.h"

// #include "RimEclipseResultCase.h"
#include "RimGridStatisticsPlot.h"
#include "RimGridStatisticsPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateGridStatisticsPlotFeature, "RicCreateGridStatisticsPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateGridStatisticsPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridStatisticsPlotFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();

    RimGridStatisticsPlotCollection* collection = project->mainPlotCollection()->gridStatisticsPlotCollection();

    RimGridStatisticsPlot* plot = new RimGridStatisticsPlot();
    plot->loadDataAndUpdate();
    plot->zoomAll();
    plot->updateConnectedEditors();
    plot->setAsPlotMdiWindow();

    collection->addGridStatisticsPlot( plot );
    collection->updateAllRequiredEditors();
    RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridStatisticsPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Grid Statistics Plot" );
}
