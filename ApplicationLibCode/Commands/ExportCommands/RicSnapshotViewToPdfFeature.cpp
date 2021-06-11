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

#include "RicSnapshotViewToPdfFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RicSnapshotFilenameGenerator.h"
#include "RicSnapshotViewToFileFeature.h"

#include "RimPlotWindow.h"

#include <QAction>
#include <QDesktopServices>
#include <QUrl>

CAF_CMD_SOURCE_INIT( RicSnapshotViewToPdfFeature, "RicSnapshotViewToPdfFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSnapshotViewToPdfFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToPdfFeature::onActionTriggered( bool isChecked )
{
    // Get active view window before displaying the file selection dialog
    // If this is done after the file save dialog is displayed (and closed)
    // app->activeViewWindow() returns nullptr on Linux

    RimViewWindow* viewWindow = RiaGuiApplication::activeViewWindow();
    if ( !viewWindow )
    {
        RiaLogging::error( "No view window is available, nothing to do" );

        return;
    }

    RimPlotWindow* plotWindow = dynamic_cast<RimPlotWindow*>( viewWindow );

    if ( plotWindow )
    {
        QString fileExtension   = "pdf";
        QString defaultFileName = RicSnapshotFilenameGenerator::generateSnapshotFileName( viewWindow );
        QString fileName = RicSnapshotViewToFileFeature::generateSaveFileName( defaultFileName, true, fileExtension );
        if ( !fileName.isEmpty() )
        {
            if ( plotWindow && fileName.endsWith( "PDF", Qt::CaseInsensitive ) )
            {
                RicSnapshotViewToFileFeature::savePlotPdfReportAs( fileName, plotWindow );

                if ( RiaPreferences::current()->openExportedPdfInViewer() )
                {
                    QDesktopServices::openUrl( fileName );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToPdfFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export View To PDF" );
    actionToSetup->setIcon( QIcon( ":/PdfSave.svg" ) );
}
