/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicSnapshotAllPlotsToFileFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "RicSnapshotFilenameGenerator.h"
#include "RicSnapshotViewToFileFeature.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuTools.h"

#include "cafUtils.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicSnapshotAllPlotsToFileFeature, "RicSnapshotAllPlotsToFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::saveAllPlots()
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    RiuPlotMainWindow* mainPlotWindow = app->mainPlotWindow();
    if ( !mainPlotWindow ) return;

    RimProject* proj = app->project();
    if ( !proj ) return;

    // Save images in snapshot catalog relative to project directory
    QString snapshotFolderName = app->createAbsolutePathFromProjectRelativePath( "snapshots" );

    // save using existing plot sizes
    if ( auto result = exportSnapshotOfPlotsIntoFolder( snapshotFolderName, -1, -1, true /* activateWidget */ ); !result )
    {
        RiaLogging::error( result.error().toStdString() );
        return;
    }

    QString text = QString( "Exported snapshots to folder : \n%1" ).arg( snapshotFolderName );
    RiaLogging::info( text.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlotsIntoFolder( const QString& snapshotFolderName,
                                                                                                int            width,
                                                                                                int            height,
                                                                                                bool           activateWidget,
                                                                                                const QString& prefix,
                                                                                                int            viewId,
                                                                                                const QString& preferredFileSuffix /*=".png"*/ )
{
    RiaApplication* app = RiaApplication::instance();

    RimProject* proj = app->project();
    if ( !proj ) return std::unexpected( QString( "No project available" ) );

    QDir snapshotPath( snapshotFolderName );
    if ( !snapshotPath.exists() )
    {
        if ( !snapshotPath.mkpath( "." ) )
        {
            return std::unexpected( QString( "Not able to create snapshot folder %1" ).arg( snapshotFolderName ) );
        }
    }

    size_t attemptedSnapshotCount = 0;
    size_t failedSnapshotCount    = 0;

    // Lay out the hidden plot window once, before the plot collection is iterated
    layoutHiddenPlotWindow();

    std::vector<RimViewWindow*> viewWindows = RimMainPlotCollection::current()->descendantsIncludingThisOfType<RimViewWindow>();
    for ( auto viewWindow : viewWindows )
    {
        if ( viewWindow->isMainDockedWindow() && viewWindow->viewWidget() && ( viewId == -1 || viewId == viewWindow->id() ) )
        {
            attemptedSnapshotCount++;

            // The error is logged by saveSnapshotAs, only the number of failures is needed here
            if ( !exportSnapshotOfPlotWithoutLayout( viewWindow, snapshotPath.absolutePath(), width, height, activateWidget, prefix, preferredFileSuffix ) )
            {
                failedSnapshotCount++;
            }
        }
    }

    if ( failedSnapshotCount > 0 )
    {
        return std::unexpected( QString( "Failed to export %1 of %2 plot snapshots" ).arg( failedSnapshotCount ).arg( attemptedSnapshotCount ) );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Export a snapshot of a single plot window into the given folder. The folder is created if it does not exist.
/// <= 0 for width and height means to use the existing size
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlot( RimViewWindow* viewWindow,
                                                                                     const QString& snapshotFolderName,
                                                                                     int            width,
                                                                                     int            height,
                                                                                     bool           activateWidget,
                                                                                     const QString& prefix,
                                                                                     const QString& preferredFileSuffix )
{
    layoutHiddenPlotWindow();

    return exportSnapshotOfPlotWithoutLayout( viewWindow, snapshotFolderName, width, height, activateWidget, prefix, preferredFileSuffix );
}

//--------------------------------------------------------------------------------------------------
/// In headless mode the plot window has never been shown, so the dock widgets and plot widgets have never been laid
/// out. Deliver the deferred resize events so all plot widgets get their proper sizes before rendering.
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::layoutHiddenPlotWindow()
{
    // RiaGuiApplication::instance() asserts when running as a console application, where command files can also
    // trigger this export
    if ( !RiaGuiApplication::isRunning() ) return;

    if ( RiuPlotMainWindow* plotWindow = RiaGuiApplication::instance()->mainPlotWindow(); plotWindow && !plotWindow->isVisible() )
    {
        RiuTools::sendDeferredResizeEvents( plotWindow );
        QApplication::processEvents();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlotWithoutLayout( RimViewWindow* viewWindow,
                                                                                                  const QString& snapshotFolderName,
                                                                                                  int            width,
                                                                                                  int            height,
                                                                                                  bool           activateWidget,
                                                                                                  const QString& prefix,
                                                                                                  const QString& preferredFileSuffix )
{
    if ( !viewWindow ) return std::unexpected( QString( "No plot to create a snapshot from" ) );

    QDir snapshotPath( snapshotFolderName );
    if ( !snapshotPath.exists() )
    {
        if ( !snapshotPath.mkpath( "." ) )
        {
            return std::unexpected( QString( "Not able to create snapshot folder %1" ).arg( snapshotFolderName ) );
        }
    }

    QString fileName = RicSnapshotFilenameGenerator::generateSnapshotFileName( viewWindow );
    if ( !prefix.isEmpty() )
    {
        fileName = prefix + fileName;
    }

    fileName.replace( " ", "_" );

    if ( activateWidget )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( viewWindow );
        QApplication::processEvents();
    }

    QString absoluteFileName = caf::Utils::constructFullFileName( snapshotPath.absolutePath(), fileName, preferredFileSuffix );

    return RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, viewWindow, width, height );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::onActionTriggered( bool isChecked )
{
    QString currentActiveViewerName;
    if ( RiaGuiApplication::activeViewWindow() )
    {
        currentActiveViewerName = RiaGuiApplication::activeViewWindow()->dockWindowName();
    }

    RicSnapshotAllPlotsToFileFeature::saveAllPlots();

    if ( !currentActiveViewerName.isEmpty() )
    {
        RiuPlotMainWindowTools::setActiveViewer( currentActiveViewerName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Snapshot All Plots To File" );
    actionToSetup->setIcon( QIcon( ":/SnapShotSaveViews.svg" ) );
}
