/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicExportFractureModelPlotToFileFeature.h"

#include "RiaApplication.h"

#include "RimFractureModelPlot.h"

#include "RifFractureModelPlotExporter.h"

#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicExportFractureModelPlotToFileFeature, "RicExportFractureModelPlotToFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportFractureModelPlotToFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureModelPlotToFileFeature::onActionTriggered( bool isChecked )
{
    RimFractureModelPlot* fractureModelPlot =
        caf::SelectionManager::instance()->selectedItemOfType<RimFractureModelPlot>();
    if ( !fractureModelPlot ) return;

    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "FRACTURE_MODEL_PLOT" );

    QString fileNameCandidate = "Geological";
    QString defaultFileName   = defaultDir + "/" + caf::Utils::makeValidFileBasename( fileNameCandidate ) + ".frk";
    QString fileName          = QFileDialog::getSaveFileName( nullptr,
                                                     "Select File for Fracture Model Plot Export",
                                                     defaultFileName,
                                                     "Geologic Model File(*.frk);;All files(*.*)" );

    if ( fileName.isEmpty() ) return;

    RifFractureModelPlotExporter::writeToFile( fractureModelPlot, fileName );

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "FRACTURE_MODEL_PLOT", QFileInfo( fileName ).absolutePath() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureModelPlotToFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Fracture Model to File" );
    actionToSetup->setIcon( QIcon( ":/Save.png" ) );
}
