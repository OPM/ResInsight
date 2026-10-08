/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicImportWellFormationsFeature.h"

#include "RiaApplication.h"

#include "Formations/RimWellFormationsCollection.h"
#include "Formations/RimWellFormationsFile.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportWellFormationsFeature, "RicImportWellFormationsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellFormationsFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "WELLPATHFORMATIONS_DIR" );
    QStringList     filePaths  = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Well Formations",
                                                                  defaultDir,
                                                                  "Well Formations (*.csv);;All Files (*.*)" );

    if ( filePaths.empty() ) return;

    // Remember the path for next time
    app->setLastUsedDialogDirectory( "WELLPATHFORMATIONS_DIR", QFileInfo( filePaths.last() ).absolutePath() );

    RimProject* project = RimProject::current();
    if ( !project || !project->activeOilField() ) return;

    RimWellFormationsCollection* collection = project->activeOilField()->wellFormationsCollection();
    if ( !collection ) return;

    std::vector<RimWellFormationsFile*> importedFiles = collection->importFiles( filePaths );
    collection->updateConnectedEditors();

    if ( !importedFiles.empty() )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( importedFiles.back() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellFormationsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Formations16x16.png" ) );
    actionToSetup->setText( "Import Well Formations..." );
}
