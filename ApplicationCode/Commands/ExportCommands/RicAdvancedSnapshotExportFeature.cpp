/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicAdvancedSnapshotExportFeature.h"

#include "RiaApplication.h"
#include "RiaViewRedrawScheduler.h"

#include "RicSnapshotViewToFileFeature.h"

#include "RigFemResultPosEnum.h"

#include "Rim3dView.h"
#include "RimAdvancedSnapshotExportDefinition.h"
#include "RimCase.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimProject.h"

#include "RiuAdvancedSnapshotExportWidget.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafFrameAnimationControl.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QDir>

CAF_CMD_SOURCE_INIT( RicAdvancedSnapshotExportFeature, "RicAdvancedSnapshotExportFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAdvancedSnapshotExportFeature::isCommandEnabled()
{
    RimProject* proj = RiaApplication::instance()->project();

    return proj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdvancedSnapshotExportFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RimProject* proj = RiaApplication::instance()->project();

    if ( proj )
    {
        // Enable the command system to be able to assign a value to multiple fields at the same time
        caf::CmdExecCommandSystemActivator activator;

        RiuAdvancedSnapshotExportWidget dlg( nullptr, proj );

        Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
        if ( activeView && proj->multiSnapshotDefinitions.size() == 0 )
        {
            dlg.addSnapshotItemFromActiveView();
            dlg.addEmptySnapshotItems( 4 );
        }

        {
            QString fallbackFolderName =
                RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "snapshots" );
            QString folderName =
                RiaApplication::instance()->lastUsedDialogDirectoryWithFallback( "ADVANCED_SNAPSHOT_EXPORT",
                                                                                 fallbackFolderName );
            dlg.setExportFolder( folderName );
        }

        dlg.exec();

        RiaApplication::instance()->setLastUsedDialogDirectory( "ADVANCED_SNAPSHOT_EXPORT", dlg.exportFolder() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdvancedSnapshotExportFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Advanced Snapshot Export ..." );
    actionToSetup->setIcon( QIcon( ":/SnapShotSaveViews.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdvancedSnapshotExportFeature::exportMultipleSnapshots( const QString& folder, RimProject* project )
{
    if ( !project ) return;

    QDir snapshotPath( folder );
    if ( !snapshotPath.exists() )
    {
        if ( !snapshotPath.mkpath( "." ) ) return;
    }

    for ( RimAdvancedSnapshotExportDefinition* msd : project->multiSnapshotDefinitions() )
    {
        if ( !msd->isActive() ) continue;

        Rim3dView* sourceView = msd->view();
        if ( !sourceView ) continue;
        if ( !sourceView->viewer() ) continue;

        int initialFramIndex = sourceView->viewer()->currentFrameIndex();

        // exportViewVariations(sourceView, msd, folder);

        for ( RimCase* rimCase : msd->additionalCases() )
        {
            RimEclipseCase* eclCase           = dynamic_cast<RimEclipseCase*>( rimCase );
            RimEclipseView* sourceEclipseView = dynamic_cast<RimEclipseView*>( sourceView );
            if ( eclCase && sourceEclipseView )
            {
                RimEclipseView* copyOfEclipseView = eclCase->createCopyAndAddView( sourceEclipseView );
                CVF_ASSERT( copyOfEclipseView );

                copyOfEclipseView->loadDataAndUpdate();

                exportViewVariations( copyOfEclipseView, msd, folder );

                eclCase->reservoirViews().removeChildObject( copyOfEclipseView );

                delete copyOfEclipseView;
            }

            RimGeoMechCase* geomCase          = dynamic_cast<RimGeoMechCase*>( rimCase );
            RimGeoMechView* sourceGeoMechView = dynamic_cast<RimGeoMechView*>( sourceView );
            if ( geomCase && sourceGeoMechView )
            {
                RimGeoMechView* copyOfGeoMechView = dynamic_cast<RimGeoMechView*>(
                    sourceGeoMechView->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
                CVF_ASSERT( copyOfGeoMechView );

                geomCase->geoMechViews().push_back( copyOfGeoMechView );

                copyOfGeoMechView->setGeoMechCase( geomCase );

                // Resolve references after reservoir view has been inserted into Rim structures
                copyOfGeoMechView->resolveReferencesRecursively();
                copyOfGeoMechView->initAfterReadRecursively();

                copyOfGeoMechView->loadDataAndUpdate();

                exportViewVariations( copyOfGeoMechView, msd, folder );

                geomCase->geoMechViews().removeChildObject( copyOfGeoMechView );

                delete copyOfGeoMechView;
            }
        }

        // Set view back to initial state
        sourceView->viewer()->setCurrentFrame( initialFramIndex );
        sourceView->viewer()->animationControl()->setCurrentFrameOnly( initialFramIndex );

        sourceView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdvancedSnapshotExportFeature::exportViewVariations( Rim3dView*                           rimView,
                                                             RimAdvancedSnapshotExportDefinition* msd,
                                                             const QString&                       folder )
{
    if ( msd->selectedEclipseResults().size() > 0 )
    {
        RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>( rimView->ownerCase() );

        RimEclipseView* copyOfView = eclCase->createCopyAndAddView( dynamic_cast<RimEclipseView*>( rimView ) );

        copyOfView->cellResult()->setResultType( msd->eclipseResultType() );

        for ( QString s : msd->selectedEclipseResults() )
        {
            copyOfView->cellResult()->setResultVariable( s );

            copyOfView->loadDataAndUpdate();

            exportViewVariationsToFolder( copyOfView, msd, folder );
        }

        eclCase->reservoirViews().removeChildObject( copyOfView );

        delete copyOfView;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdvancedSnapshotExportFeature::exportViewVariationsToFolder( RimGridView*                         rimView,
                                                                     RimAdvancedSnapshotExportDefinition* msd,
                                                                     const QString&                       folder )
{
    RimCase* rimCase = rimView->ownerCase();
    CVF_ASSERT( rimCase );

    RiuViewer*  viewer    = rimView->viewer();
    QStringList timeSteps = rimCase->timeStepStrings();

    QString resName              = resultName( rimView );
    QString viewCaseResultString = rimCase->caseUserDescription() + "_" + rimView->name() + "_" + resName;
    viewCaseResultString         = caf::Utils::makeValidFileBasename( viewCaseResultString );

    for ( int i = msd->timeStepStart(); i <= msd->timeStepEnd(); i++ )
    {
        QString timeStepIndexString = QString( "%1" ).arg( i, 2, 10, QLatin1Char( '0' ) );

        QString timeStepString = timeStepIndexString + "_" + timeSteps[i].replace( ".", "-" );

        if ( viewer )
        {
            // Force update of scheduled display models modifying the time step
            // This is required due to visualization structures updated by the update functions,
            // and this is not triggered by changing time step only
            RiaViewRedrawScheduler::instance()->updateAndRedrawScheduledViews();

            viewer->setCurrentFrame( i );
            viewer->animationControl()->setCurrentFrameOnly( i );
        }

        if ( msd->sliceDirection == RimAdvancedSnapshotExportDefinition::NO_RANGEFILTER )
        {
            QString fileName = viewCaseResultString + "_" + timeStepString;
            fileName.replace( " ", "_" );

            QString absoluteFileName = caf::Utils::constructFullFileName( folder, fileName, ".png" );

            RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, rimView );
        }
        else
        {
            RimCellRangeFilter* rangeFilter = new RimCellRangeFilter;
            rimView->rangeFilterCollection()->rangeFilters.push_back( rangeFilter );

            bool rangeFilterInitState                  = rimView->rangeFilterCollection()->isActive();
            rimView->rangeFilterCollection()->isActive = true;

            for ( int sliceIndex = msd->startSliceIndex(); sliceIndex <= msd->endSliceIndex(); sliceIndex++ )
            {
                QString rangeFilterString = msd->sliceDirection().text() + "-" + QString::number( sliceIndex );
                QString fileName          = viewCaseResultString + "_" + timeStepString + "_" + rangeFilterString;

                rangeFilter->setDefaultValues();
                if ( msd->sliceDirection == RimAdvancedSnapshotExportDefinition::RANGEFILTER_I )
                {
                    rangeFilter->cellCountI  = 1;
                    rangeFilter->startIndexI = sliceIndex;
                }
                else if ( msd->sliceDirection == RimAdvancedSnapshotExportDefinition::RANGEFILTER_J )
                {
                    rangeFilter->cellCountJ  = 1;
                    rangeFilter->startIndexJ = sliceIndex;
                }
                else if ( msd->sliceDirection == RimAdvancedSnapshotExportDefinition::RANGEFILTER_K )
                {
                    rangeFilter->cellCountK  = 1;
                    rangeFilter->startIndexK = sliceIndex;
                }

                rimView->rangeFilterCollection()->updateDisplayModeNotifyManagedViews( rangeFilter );
                fileName.replace( " ", "_" );

                QString absoluteFileName = caf::Utils::constructFullFileName( folder, fileName, ".png" );

                RicSnapshotViewToFileFeature::saveSnapshotAs( absoluteFileName, rimView );
            }

            rimView->rangeFilterCollection()->rangeFilters.removeChildObject( rangeFilter );
            delete rangeFilter;

            rimView->rangeFilterCollection()->isActive = rangeFilterInitState;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicAdvancedSnapshotExportFeature::resultName( Rim3dView* rimView )
{
    if ( dynamic_cast<RimEclipseView*>( rimView ) )
    {
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( rimView );

        return caf::Utils::makeValidFileBasename( eclView->cellResult()->resultVariableUiShortName() );
    }
    else if ( dynamic_cast<RimGeoMechView*>( rimView ) )
    {
        RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( rimView );

        RimGeoMechCellColors* cellResult = geoMechView->cellResult();

        if ( cellResult )
        {
            QString title = caf::AppEnum<RigFemResultPosEnum>( cellResult->resultPositionType() ).uiText() + "_" +
                            cellResult->resultFieldUiName();

            if ( !cellResult->resultComponentUiName().isEmpty() )
            {
                title += "_" + cellResult->resultComponentUiName();
            }

            return title;
        }
    }

    return "";
}
