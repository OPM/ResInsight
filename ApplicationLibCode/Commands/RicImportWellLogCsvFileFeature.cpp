/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RicImportWellLogCsvFileFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogCsvFile.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportWellLogCsvFileFeature, "RicImportWellLogCsvFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportWellLogCsvFileFeature::isCommandEnabled() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellLogCsvFileFeature::onActionTriggered( bool isChecked )
{
    QString pathCacheName = "WELL_LOGS_DIR";

    if ( auto wellPath = caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>() )
    {
        RiaGuiApplication* app = RiaGuiApplication::instance();

        QString defaultDir = app->lastUsedDialogDirectory( pathCacheName );
        QString fileName =
            RiuFileDialogTools::getOpenFileName( nullptr, "Open Well Log CSV", defaultDir, "Well Log csv (*.csv);;All files(*.*)" );

        if ( fileName.isEmpty() ) return;

        RimOilField* oilField = RimProject::current()->activeOilField();
        if ( oilField == nullptr ) return;

        if ( !oilField->wellPathCollection ) oilField->wellPathCollection = std::make_unique<RimWellPathCollection>();

        RimWellLogCsvFile* wellLogCsvFile = new RimWellLogCsvFile;
        wellLogCsvFile->setFileName( fileName );
        oilField->wellPathCollection->addWellLog( wellLogCsvFile, wellPath );

        QString errorMessage;
        if ( !wellLogCsvFile->readFile( &errorMessage ) )
        {
            wellPath->deleteWellLog( wellLogCsvFile );
            QString displayMessage = "Errors opening the CSV file: \n" + errorMessage;
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "File open error", displayMessage );
            return;
        }
        else
        {
            wellLogCsvFile->updateConnectedEditors();
        }

        app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( fileName ).absolutePath() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellLogCsvFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/LasFile16x16.png" ) );
    actionToSetup->setText( "Import Well Log From CSV" );
}
