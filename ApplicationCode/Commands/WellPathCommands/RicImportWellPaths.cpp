/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicImportWellPaths.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicImportWellPaths, "RicWellPathsImportFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFileWellPath*> RicImportWellPaths::importWellPaths( const QStringList& wellPathFilePaths,
                                                                   QStringList*       errorMessages )
{
    RiaApplication* app = RiaApplication::instance();

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "WELLPATH_DIR", QFileInfo( wellPathFilePaths.last() ).absolutePath() );

    std::vector<RimFileWellPath*> wellPaths = app->addWellPathsToModel( wellPathFilePaths, errorMessages );

    RimProject* project = app->project();

    if ( project )
    {
        project->scheduleCreateDisplayModelAndRedrawAllViews();
        RimOilField* oilField = project->activeOilField();

        if ( oilField && oilField->wellPathCollection->wellPaths().size() > 0 )
        {
            RimWellPath* wellPath = oilField->wellPathCollection->mostRecentlyUpdatedWellPath();
            if ( wellPath )
            {
                Riu3DMainWindowTools::selectAsCurrentItem( wellPath );
            }
        }
    }
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportWellPaths::wellPathNameFilters()
{
    QStringList nameFilters;
    nameFilters << "*.json"
                << "*.asc"
                << " *.asci"
                << "*.ascii"
                << "*.dev";
    return nameFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportWellPaths::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellPaths::onActionTriggered( bool isChecked )
{
    // Open dialog box to select well path files
    RiaApplication* app                = RiaApplication::instance();
    QString         lastUsedGridFolder = app->lastUsedDialogDirectory( "BINARY_GRID" );
    QString         defaultDir         = app->lastUsedDialogDirectoryWithFallback( "WELLPATH_DIR", lastUsedGridFolder );

    QString nameList = QString( "Well Paths (%1);;All Files (*.*)" ).arg( wellPathNameFilters().join( " " ) );

    QStringList wellPathFilePaths = QFileDialog::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                   "Import Well Paths",
                                                                   defaultDir,
                                                                   nameList );

    if ( wellPathFilePaths.size() >= 1 )
    {
        QStringList errorMessages;
        importWellPaths( wellPathFilePaths, &errorMessages );

        if ( !errorMessages.empty() )
        {
            QString displayMessage = "Errors loading well path files: \n" + errorMessages.join( "\n" );

            if ( RiaGuiApplication::isRunning() )
            {
                QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(), "File open error", displayMessage );
            }
            RiaLogging::warning( displayMessage );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellPaths::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import &Well Paths from File" );
    actionToSetup->setIcon( QIcon( ":/Well.png" ) );
}
