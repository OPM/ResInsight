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

#include "RicRestoreProjectVersionsFeature.h"
#include "RicRestoreProjectVersionsUi.h"

#include "RiaApplication.h"
#include "RiaProjectBackupTools.h"

#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuMessageDialog.h"
#include "RiuRecentFileActionProvider.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicRestoreProjectVersionsFeature, "RicRestoreProjectVersionsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRestoreProjectVersionsFeature::onActionTriggered( bool isChecked )
{
    RicRestoreProjectVersionsUi featureUi;

    RimProject* project = RiaApplication::instance()->project();

    QString projectFileName = ( project ) ? project->fileName() : QString();
    if ( projectFileName.isEmpty() )
    {
        projectFileName = RiuRecentFileActionProvider::lastUsedProjectFileName();
    }

    if ( !projectFileName.isEmpty() )
    {
        featureUi.setProjectFile( projectFileName );
        featureUi.setExportFolder( QFileInfo( projectFileName ).absolutePath() );
    }

    caf::PdmUiPropertyViewDialog propertyDialog( RiuMainWindow::instance(),
                                                 &featureUi,
                                                 "Restore Previous Project Versions",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    propertyDialog.resize( QSize( 560, 260 ) );

    if ( propertyDialog.exec() != QDialog::Accepted ) return;

    QString projectFile  = featureUi.projectFile();
    QString exportFolder = featureUi.exportFolder();
    if ( projectFile.isEmpty() || exportFolder.isEmpty() ) return;

    QString databaseFilePath = projectFile + "db";
    if ( !QFile::exists( databaseFilePath ) )
    {
        RiuMessageDialog::showError( RiuMainWindow::instance(),
                                     "Restore Previous Project Versions",
                                     "No backup database was found for the selected project file.\n\n" + databaseFilePath );
        return;
    }

    int     numberOfFilesWritten = 0;
    QString errorMessage;
    if ( !RiaProjectBackupTools::extractVersionsToFolder( databaseFilePath, exportFolder, &numberOfFilesWritten, &errorMessage ) )
    {
        RiuMessageDialog::showError( RiuMainWindow::instance(), "Restore Previous Project Versions", errorMessage );
        return;
    }

    QMessageBox::information( RiuMainWindow::instance(),
                              "Restore Previous Project Versions",
                              QString( "Restored %1 project version(s) to:\n%2" ).arg( numberOfFilesWritten ).arg( exportFolder ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRestoreProjectVersionsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Restore Previous Project Versions..." );
    actionToSetup->setIcon( QIcon( ":/OpenFolder.svg" ) );
}
