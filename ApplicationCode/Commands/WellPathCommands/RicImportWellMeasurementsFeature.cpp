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

#include "RicImportWellMeasurementsFeature.h"

#include "RiaApplication.h"

#include "RimWellPathCollection.h"

#include "RicWellMeasurementImportTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportWellMeasurementsFeature, "RicImportWellMeasurementsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportWellMeasurementsFeature::isCommandEnabled()
{
    return ( RicWellMeasurementImportTools::selectedWellPathCollection() != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellMeasurementsFeature::onActionTriggered( bool isChecked )
{
    RimWellPathCollection* wellPathCollection = RicWellMeasurementImportTools::selectedWellPathCollection();
    CVF_ASSERT( wellPathCollection );

    // Open dialog box to select well path files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "WELLPATH_DIR" );
    QStringList     wellPathFilePaths =
        RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                              "Import Well Measurements",
                                              defaultDir,
                                              "Well Measurements (*.csv);;All Files (*.*)" );

    if ( wellPathFilePaths.size() < 1 ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "WELLPATH_DIR", QFileInfo( wellPathFilePaths.last() ).absolutePath() );

    RicWellMeasurementImportTools::importWellMeasurementsFromFiles( wellPathFilePaths, wellPathCollection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellMeasurementsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Measurements" );
    actionToSetup->setIcon( QIcon( ":/WellMeasurement16x16.png" ) );
}
