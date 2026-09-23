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

#include "RicImportSimulatorEventsFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"
#include "RiuMainWindow.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicImportSimulatorEventsFeature, "RicImportSimulatorEventsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSimulatorEventsFeature::onActionTriggered( bool isChecked )
{
    // The import runs as a Python child process which applies the events to this instance through
    // gRPC, so both the script server and a Python interpreter must be available.
    RiaApplication* app = RiaApplication::instance();
    if ( !app->activeGrpcPortNumber() )
    {
        QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(),
                              "Import Simulator Events",
                              "The Python script server is not running. Enable it in Preferences -> Scripting and restart ResInsight." );
        return;
    }

    if ( app->pythonPath().isEmpty() )
    {
        QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(),
                              "Import Simulator Events",
                              "No Python executable is configured in Preferences -> Scripting." );
        return;
    }

    QString     defaultDir = app->lastUsedDialogDirectory( "SIMULATOR_EVENTS_DIR" );
    QStringList fileNames  = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Simulator Events",
                                                                  defaultDir,
                                                                  "Simulator Events Files (*.events);;All Files (*.*)" );
    if ( fileNames.isEmpty() ) return;

    app->setLastUsedDialogDirectory( "SIMULATOR_EVENTS_DIR", QFileInfo( fileNames.front() ).absolutePath() );

    // Show the process monitor so the report and any errors from the child process are visible.
    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if ( !mainWindow )
    {
        mainWindow = RiaGuiApplication::instance()->getOrCreateAndShowMainWindow();
    }
    mainWindow->showProcessMonitorDockPanel();

    // Unbuffered output ("-u") so the report streams into the process monitor promptly. The child
    // process finds this instance through the RESINSIGHT_GRPC_PORT environment variable.
    // Multiple files are merged into one document, so matching events from different files are
    // combined into a single timeline event.
    QStringList arguments = { "-u", "-m", "rips.simulator_events", "--apply" };
    arguments.append( fileNames );
    if ( !app->launchProcess( app->pythonPath(), arguments, app->pythonProcessEnvironment() ) )
    {
        RiaLogging::error( "Failed to launch the Python interpreter. Another script may already be running." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSimulatorEventsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Simulator Events" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}
