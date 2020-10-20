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

#include "RicWellLogsImportFileFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafPdmUiObjectEditorHandle.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicWellLogsImportFileFeature, "RicWellLogsImportFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RicWellLogsImportFileFeature::importWellLogFiles( const QStringList& wellLogFilePaths,
                                                                               QStringList*       errorMessages )
{
    RiaApplication* app = RiaApplication::instance();

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "WELL_LOGS_DIR", QFileInfo( wellLogFilePaths.last() ).absolutePath() );

    std::vector<RimWellLogFile*> wellLogFiles = app->addWellLogsToModel( wellLogFilePaths, errorMessages );

    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();

    return wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicWellLogsImportFileFeature::wellLogFileNameFilters()
{
    QStringList nameFilters;
    nameFilters << "*.las"
                << "*.LAS";
    return nameFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellLogsImportFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogsImportFileFeature::onActionTriggered( bool isChecked )
{
    // Open dialog box to select well path files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "WELL_LOGS_DIR" );
    QString nameFilterString = QString( "Well Logs (%1);;All Files (*.*)" ).arg( wellLogFileNameFilters().join( " " ) );
    QStringList wellLogFilePaths = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                         "Import Well Logs",
                                                                         defaultDir,
                                                                         nameFilterString );

    if ( wellLogFilePaths.size() >= 1 )
    {
        QStringList errorMessages;
        importWellLogFiles( wellLogFilePaths, &errorMessages );
        if ( !errorMessages.empty() )
        {
            QString displayMessage = "Errors opening the LAS files: \n" + errorMessages.join( "\n" );
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "File open error", displayMessage );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogsImportFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Well &Logs from File" );
    actionToSetup->setIcon( QIcon( ":/LasFile16x16.png" ) );
}
