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
#include "RiaGuiApplication.h"
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
#include <QInputDialog>
#include <QMessageBox>

#include <algorithm>

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

    auto result = RiaProjectBackupTools::extractVersionsToFolder( databaseFilePath, exportFolder );
    if ( !result )
    {
        RiuMessageDialog::showError( RiuMainWindow::instance(), "Restore Previous Project Versions", result.error() );
        return;
    }

    QStringList restoredFiles = result.value();

    if ( restoredFiles.isEmpty() )
    {
        QMessageBox::information( RiuMainWindow::instance(),
                                  "Restore Previous Project Versions",
                                  QString( "No project versions were found in the backup database.\n%1" ).arg( databaseFilePath ) );
        return;
    }

    // Newest version first
    std::reverse( restoredFiles.begin(), restoredFiles.end() );

    QStringList displayNames;
    for ( const QString& file : restoredFiles )
    {
        displayNames.push_back( QFileInfo( file ).fileName() );
    }

    bool          accepted = false;
    const QString selected = QInputDialog::getItem( RiuMainWindow::instance(),
                                                    "Restore Previous Project Versions",
                                                    QString( "Restored %1 project version(s) to:\n%2\n\nSelect a version to open:" )
                                                        .arg( restoredFiles.size() )
                                                        .arg( exportFolder ),
                                                    displayNames,
                                                    0,
                                                    false,
                                                    &accepted );
    if ( !accepted ) return;

    const int index = displayNames.indexOf( selected );
    if ( index < 0 ) return;

    RiaGuiApplication* app = RiaGuiApplication::instance();
    if ( !app->checkWithUserBeforeClose() ) return;

    const QString fileToOpen = restoredFiles[index];
    if ( app->loadProject( fileToOpen ) )
    {
        app->addToRecentFiles( fileToOpen );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRestoreProjectVersionsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Restore Previous Project Versions..." );
    actionToSetup->setIcon( QIcon( ":/OpenFolder.svg" ) );
}
