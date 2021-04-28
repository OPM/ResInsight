/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RicCreateEnsembleFractureStatisticsPlotFeature.h"

#include "RiaGuiApplication.h"

#include "RimEnsembleFractureStatisticsPlot.h"
#include "RimEnsembleFractureStatisticsPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateEnsembleFractureStatisticsPlotFeature, "RicCreateEnsembleFractureStatisticsPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateEnsembleFractureStatisticsPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleFractureStatisticsPlotFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();

    RimEnsembleFractureStatisticsPlotCollection* collection =
        project->mainPlotCollection()->ensembleFractureStatisticsPlotCollection();

    RimEnsembleFractureStatisticsPlot* plot = new RimEnsembleFractureStatisticsPlot();
    plot->zoomAll();
    plot->updateConnectedEditors();
    plot->setAsPlotMdiWindow();
    plot->loadDataAndUpdate();

    collection->addEnsembleFractureStatisticsPlot( plot );
    collection->updateAllRequiredEditors();
    RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleFractureStatisticsPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Ensemble Fracture Statistics Plot" );
}
