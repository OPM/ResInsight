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
#include <QFileDialog>
#include <QFileInfo>
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
    RimProject* project = RiaApplication::instance()->project();

    if ( project == nullptr ) return;

    QDir snapshotPath( snapshotFolderName );
    if ( !snapshotPath.exists() )
    {
        if ( !snapshotPath.mkpath( "." ) ) return;
    }

    const QString absSnapshotPath = snapshotPath.absolutePath();

    RiaLogging::info( QString( "Exporting snapshot of all views to %1" ).arg( snapshotFolderName ) );

    std::vector<RimCase*> projectCases;
    project->allCases( projectCases );

    for ( size_t i = 0; i < projectCases.size(); i++ )
    {
        RimCase* cas = projectCases[i];
        if ( !cas ) continue;

        bool matchingCaseId = caseId == -1 || caseId == cas->caseId();
        if ( !matchingCaseId ) continue;

        std::vector<Rim3dView*> views = cas->views();

        for ( size_t j = 0; j < views.size(); j++ )
        {
            Rim3dView* riv = views[j];

            if ( riv && riv->viewer() && ( viewId == -1 || viewId == riv->id() ) )
            {
                RiaApplication::instance()->setActiveReservoirView( riv );

                RiuViewer* viewer = riv->viewer();
                Riu3DMainWindowTools::setActiveViewer( viewer->layoutWidget() );

                RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();
                RiaPlotWindowRedrawScheduler::instance()->clearAllScheduledUpdates();

                // riv->updateCurrentTimeStepAndRedraw();
                riv->createDisplayModelAndRedraw();
                viewer->repaint();

                QString fileName = RicSnapshotFilenameGenerator::generateSnapshotFileName( riv );
                if ( !prefix.isEmpty() )
                {
                    fileName = prefix + fileName;
                }

                QString absoluteFileName = caf::Utils::constructFullFileName( absSnapshotPath, fileName, ".png" );

                RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, riv );

                // Statistics dialog

                RimGridView* rigv = dynamic_cast<RimGridView*>( riv );
                if ( rigv )
                {
                    QImage img       = rigv->overlayInfoConfig()->statisticsDialogScreenShotImage();
                    absoluteFileName = caf::Utils::constructFullFileName( absSnapshotPath,
                                                                          fileName + "_Statistics",
                                                                          ".png" );
                    RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, img );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSnapshotAllViewsToFileFeature::isCommandEnabled()
{
    return true;
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
    actionToSetup->setIcon( QIcon( ":/SnapShotSaveViews.png" ) );
}
