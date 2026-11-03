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

#include "RicSnapshotAllViewsToFileFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaQStringFormatter.h"
#include "RiaViewRedrawScheduler.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "RicSnapshotFilenameGenerator.h"
#include "RicSnapshotViewToFileFeature.h"

#include "Riu3DMainWindowTools.h"
#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "RigFemResultPosEnum.h"

#include "cafUtils.h"
#include "cafViewer.h"

#include "cvfOpenGLContextGroup.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>

CAF_CMD_SOURCE_INIT( RicSnapshotAllViewsToFileFeature, "RicSnapshotAllViewsToFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllViewsToFileFeature::saveAllViews()
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();
    if ( !proj ) return;

    // Save images in snapshot catalog relative to project directory
    QString snapshotFolderName = app->createAbsolutePathFromProjectRelativePath( "snapshots" );

    if ( auto result = exportSnapshotOfViewsIntoFolder( snapshotFolderName ); !result )
    {
        RiaLogging::error( result.error().toStdString() );
        return;
    }

    QString text = QString( "Exported snapshots to folder : \n%1" ).arg( snapshotFolderName );
    RiaLogging::info( text.toStdString() );
}

//--------------------------------------------------------------------------------------------------
/// Export snapshots of a given view (or viewId == -1 for all views) for the given case (or caseId == -1 for all cases)
/// <= 0 for width and height means to use the existing view size
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( const QString& snapshotFolderName,
                                                                                                int            width,
                                                                                                int            height,
                                                                                                const QString& prefix,
                                                                                                int            caseId,
                                                                                                int            viewId )
{
    RimProject* project = RimProject::current();
    if ( project == nullptr ) return std::unexpected( QString( "No project available" ) );

    QDir snapshotPath( snapshotFolderName );
    if ( !snapshotPath.exists() )
    {
        if ( !snapshotPath.mkpath( "." ) )
        {
            return std::unexpected( QString( "Not able to create snapshot folder %1" ).arg( snapshotFolderName ) );
        }
    }

    std::vector<Rim3dView*> viewsForSnapshot;
    if ( caseId == -1 && viewId == -1 )
    {
        viewsForSnapshot = project->allViews();
    }
    else
    {
        for ( auto gridCase : project->allGridCases() )
        {
            if ( !gridCase ) continue;

            bool matchingCaseId = caseId == -1 || caseId == gridCase->caseId();
            if ( !matchingCaseId ) continue;

            for ( auto view : gridCase->views() )
            {
                if ( view && view->viewer() && ( viewId == -1 || viewId == view->id() ) )
                {
                    viewsForSnapshot.push_back( view );
                }
            }
        }
    }

    // Views without a viewer are not displayed and are skipped, not treated as a failure
    const size_t matchingViewCount = viewsForSnapshot.size();
    std::erase_if( viewsForSnapshot, []( Rim3dView* view ) { return !view || !view->viewer() || !view->viewer()->ownerViewWindow(); } );

    if ( viewsForSnapshot.empty() )
    {
        if ( matchingViewCount > 0 )
        {
            const QString message = QString( "None of the %1 views are displayed, no view snapshots exported" ).arg( matchingViewCount );

            // Closed views are a normal state in the GUI. When running headless, all views are expected to have a
            // viewer, so missing snapshots indicate an error.
            if ( RiaGuiApplication::isHeadless() ) return std::unexpected( message );

            RiaLogging::warning( message.toStdString() );
        }

        return {};
    }

    RiaLogging::info( std::format( "Exporting snapshot of all views to {}", snapshotFolderName ) );

    size_t failedSnapshotCount = 0;

    for ( auto riv : viewsForSnapshot )
    {
        // The error is logged by saveSnapshotAs, only the number of failures is needed here
        if ( !exportSnapshotOfView( riv, snapshotPath.absolutePath(), width, height, prefix ) ) failedSnapshotCount++;
    }

    // No OpenGL context is initialized if every snapshot failed, and the strings would be empty
    if ( failedSnapshotCount < viewsForSnapshot.size() )
    {
        // The OpenGL strings are populated when the first context is initialized, which for hidden viewers happens
        // inside the first snapshot grab. Useful to verify that the expected renderer (e.g. software/llvmpipe) is used.
        const cvf::OpenGLInfo glInfo = caf::Viewer::contextGroup()->info();
        RiaLogging::info( std::format( "OpenGL used for snapshots: {} / {} / {}",
                                       glInfo.version().toStdString(),
                                       glInfo.vendor().toStdString(),
                                       glInfo.renderer().toStdString() ) );
    }

    if ( failedSnapshotCount > 0 )
    {
        return std::unexpected(
            QString( "Failed to export %1 of %2 view snapshots" ).arg( failedSnapshotCount ).arg( viewsForSnapshot.size() ) );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Export a snapshot of a single view into the given folder. The folder is created if it does not exist.
/// <= 0 for width and height means to use the existing view size
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicSnapshotAllViewsToFileFeature::exportSnapshotOfView( Rim3dView*     riv,
                                                                                     const QString& snapshotFolderName,
                                                                                     int            width,
                                                                                     int            height,
                                                                                     const QString& prefix )
{
    if ( !riv ) return std::unexpected( QString( "No view to create a snapshot from" ) );

    // Views without a viewer are not displayed and are skipped, not treated as a failure
    RiuViewer* viewer = riv->viewer();
    if ( !viewer ) return {};
    if ( !viewer->ownerViewWindow() ) return {};

    QDir snapshotPath( snapshotFolderName );
    if ( !snapshotPath.exists() )
    {
        if ( !snapshotPath.mkpath( "." ) )
        {
            return std::unexpected( QString( "Not able to create snapshot folder %1" ).arg( snapshotFolderName ) );
        }
    }

    if ( RiuMainWindow* mainWnd = RiuMainWindow::instance(); RiaGuiApplication::isHeadless() && mainWnd && !mainWnd->isVisible() )
    {
        // When running headless with a hidden main window, Qt selects the global share context for the OpenGL context
        // of the first rendered view widget. The dock widget activation below gives the main window a native window
        // handle, making Qt select a new share context for the view widgets rendered later, which asserts in
        // cvfqt::OpenGLWidget::initializeGL() as the contexts do not share resources. Create the native window up
        // front so the share context selection is identical for all view widgets.
        mainWnd->createWinId();
    }

    RiaApplication::instance()->setActiveReservoirView( riv );

    Riu3DMainWindowTools::setActiveViewer( viewer->ownerViewWindow()->dockWindowName() );

    RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();
    RiaPlotWindowRedrawScheduler::instance()->clearAllScheduledUpdates();

    riv->createDisplayModelAndRedraw();
    viewer->repaint();

    QString fileName = RicSnapshotFilenameGenerator::generateSnapshotFileName( riv );
    if ( !prefix.isEmpty() )
    {
        fileName = prefix + fileName;
    }

    QString absoluteFileName = caf::Utils::constructFullFileName( snapshotPath.absolutePath(), fileName, ".png" );

    return RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, riv, width, height );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllViewsToFileFeature::onActionTriggered( bool isChecked )
{
    QString currentActiveViewerName;
    if ( RiaGuiApplication::activeViewWindow() )
    {
        currentActiveViewerName = RiaGuiApplication::activeViewWindow()->dockWindowName();
    }

    RicSnapshotAllViewsToFileFeature::saveAllViews();

    if ( !currentActiveViewerName.isEmpty() )
    {
        Riu3DMainWindowTools::setActiveViewer( currentActiveViewerName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllViewsToFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Snapshot All Views To File" );
    actionToSetup->setIcon( QIcon( ":/SnapShotSaveViews.svg" ) );
}
