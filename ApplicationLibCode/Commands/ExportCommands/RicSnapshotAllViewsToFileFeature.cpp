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
#include "RiaViewRedrawScheduler.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "RicSnapshotFilenameGenerator.h"
#include "RicSnapshotViewToFileFeature.h"

#include "Riu3DMainWindowTools.h"
#include "RiuViewer.h"

#include "RigFemResultPosEnum.h"

#include "cafUtils.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QMdiSubWindow>

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

    exportSnapshotOfViewsIntoFolder( snapshotFolderName );

    QString text = QString( "Exported snapshots to folder : \n%1" ).arg( snapshotFolderName );
    RiaLogging::info( text );
}

//--------------------------------------------------------------------------------------------------
/// Export snapshots of a given view (or viewId == -1 for all views) for the given case (or caseId == -1 for all cases)
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( const QString& snapshotFolderName,
                                                                        const QString& prefix /*= ""*/,
                                                                        int            caseId /*= -1*/,
                                                                        int            viewId /*= -1*/ )
{
    RimProject* project = RimProject::current();
    if ( project == nullptr ) return;

    QDir snapshotPath( snapshotFolderName );
    if ( !snapshotPath.exists() )
    {
        if ( !snapshotPath.mkpath( "." ) ) return;
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

    const QString absSnapshotPath = snapshotPath.absolutePath();
    RiaLogging::info( QString( "Exporting snapshot of all views to %1" ).arg( snapshotFolderName ) );

    for ( auto riv : viewsForSnapshot )
    {
        RiaApplication::instance()->setActiveReservoirView( riv );

        RiuViewer* viewer = riv->viewer();
        Riu3DMainWindowTools::setActiveViewer( viewer->layoutWidget() );

        RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();
        RiaPlotWindowRedrawScheduler::instance()->clearAllScheduledUpdates();

        riv->createDisplayModelAndRedraw();
        viewer->repaint();

        QString fileName = RicSnapshotFilenameGenerator::generateSnapshotFileName( riv );
        if ( !prefix.isEmpty() )
        {
            fileName = prefix + fileName;
        }

        QString absoluteFileName = caf::Utils::constructFullFileName( absSnapshotPath, fileName, ".png" );

        RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, riv );

        if ( RimGridView* rigv = dynamic_cast<RimGridView*>( riv ) )
        {
            QImage img       = rigv->overlayInfoConfig()->statisticsDialogScreenShotImage();
            absoluteFileName = caf::Utils::constructFullFileName( absSnapshotPath, fileName + "_Statistics", ".png" );
            RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, img );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllViewsToFileFeature::onActionTriggered( bool isChecked )
{
    QWidget* currentActiveWidget = nullptr;
    if ( RiaGuiApplication::activeViewWindow() )
    {
        currentActiveWidget = RiaGuiApplication::activeViewWindow()->viewWidget();
    }

    RicSnapshotAllViewsToFileFeature::saveAllViews();

    if ( currentActiveWidget )
    {
        Riu3DMainWindowTools::setActiveViewer( currentActiveWidget );
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
