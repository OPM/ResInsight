/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RiaImportEclipseCaseTools.h"

#include "ApplicationCommands/RicShowMainWindowFeature.h"

#include "SummaryPlotCommands/RicNewSummaryCurveFeature.h"
#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RifEclipseSummaryTools.h"
#include "RifSummaryCaseRestartSelector.h"

#include "RigGridManager.h"

#include "RimCaseCollection.h"
#include "RimCompletionTemplateCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFileSummaryCase.h"
#include "RimGridSummaryCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimWellLogRftCurve.h"

#include "Riu3DMainWindowTools.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafProgressInfo.h"
#include "cafUtils.h"

#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEclipseCasesFromFile( const QStringList& fileNames,
                                                          FileCaseIdMap*     openedFilesOut,
                                                          bool               noDialog )
{
    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    // Get list of files to import
    RifSummaryCaseRestartSelector selector;
    if ( noDialog ) selector.showDialog( false );
    selector.determineFilesToImportFromGridFiles( fileNames );
    std::vector<RifSummaryCaseFileResultInfo> summaryFileInfos = selector.summaryFileInfos();

    FileCaseIdMap openedFiles;

    // Import eclipse case files
    for ( const QString& gridCaseFile : selector.gridCaseFiles() )
    {
        int caseId = RiaImportEclipseCaseTools::openEclipseCaseFromFile( gridCaseFile );
        if ( caseId >= 0 )
        {
            openedFiles.insert( std::make_pair( gridCaseFile, caseId ) );
        }
    }

    if ( !openedFiles.empty() )
    {
        if ( project && project->mainPlotCollection() )
        {
            project->mainPlotCollection()->ensureDefaultFlowPlotsAreCreated();
        }
    }

    // Import summary cases
    if ( !summaryFileInfos.empty() )
    {
        RimSummaryCaseMainCollection* sumCaseColl =
            project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
        if ( sumCaseColl )
        {
            std::vector<RimSummaryCase*> newSumCases = sumCaseColl->createSummaryCasesFromFileInfos( summaryFileInfos );
            for ( RimSummaryCase* newSumCase : newSumCases )
            {
                RimSummaryCaseCollection* existingCollection = nullptr;
                QString                   gridCaseFile =
                    RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile( newSumCase->summaryHeaderFilename() );
                RimEclipseCase* gridCase = project->eclipseCaseFromGridFileName( gridCaseFile );
                if ( gridCase )
                {
                    RimSummaryCase* existingSummaryCase =
                        sumCaseColl->findSummaryCaseFromFileName( newSumCase->summaryHeaderFilename() );
                    RimGridSummaryCase* existingGridSummaryCase = dynamic_cast<RimGridSummaryCase*>( existingSummaryCase );
                    RimFileSummaryCase* existingFileSummaryCase = dynamic_cast<RimFileSummaryCase*>( existingSummaryCase );
                    if ( existingGridSummaryCase )
                    {
                        delete newSumCase; // No need to add anything new. Already have one.
                        continue;
                    }
                    else if ( existingFileSummaryCase )
                    {
                        existingFileSummaryCase->firstAncestorOrThisOfType( existingCollection );

                        // Replace file summary case pointers in Rft Curves
                        std::vector<RimWellLogRftCurve*> rftCurves;
                        existingFileSummaryCase->objectsWithReferringPtrFieldsOfType( rftCurves );
                        for ( RimWellLogRftCurve* curve : rftCurves )
                        {
                            if ( curve->summaryCase() == existingSummaryCase )
                            {
                                curve->setSummaryCase( newSumCase );
                            }
                        }

                        // Replace all occurrences of file sum with ecl sum

                        std::vector<RimSummaryCurve*> objects;
                        existingFileSummaryCase->objectsWithReferringPtrFieldsOfType( objects );

                        // UI settings of a curve filter is updated based
                        // on the new case association for the curves in the curve filter
                        // UI is updated by loadDataAndUpdate()

                        for ( RimSummaryCurve* summaryCurve : objects )
                        {
                            if ( summaryCurve )
                            {
                                if ( summaryCurve->summaryCaseX() == existingSummaryCase )
                                {
                                    summaryCurve->setSummaryCaseX( newSumCase );
                                }
                                if ( summaryCurve->summaryCaseY() == existingSummaryCase )
                                {
                                    summaryCurve->setSummaryCaseY( newSumCase );
                                }

                                RimSummaryCurveCollection* parentCollection = nullptr;
                                summaryCurve->firstAncestorOrThisOfType( parentCollection );
                                if ( parentCollection )
                                {
                                    parentCollection->loadDataAndUpdate( true );
                                    parentCollection->updateConnectedEditors();
                                    break;
                                }
                            }
                        }

                        // Remove existing case
                        sumCaseColl->removeCase( existingFileSummaryCase );
                        delete existingFileSummaryCase;
                    }
                }
                if ( existingCollection )
                {
                    existingCollection->addCase( newSumCase );
                }
                else
                {
                    sumCaseColl->addCase( newSumCase );
                }
                sumCaseColl->updateAllRequiredEditors();
            }

            if ( !newSumCases.empty() )
            {
                RimSummaryPlotCollection* summaryPlotColl = project->mainPlotCollection()->summaryPlotCollection();

                RicSummaryPlotFeatureImpl::ensureAtLeastOnePlot( summaryPlotColl, newSumCases.front() );

                RiuPlotMainWindowTools::setExpanded( newSumCases.front() );
            }
        }
    }

    if ( selector.foundErrors() )
    {
        QString errorMessage = selector.createCombinedErrorMessage();
        RiaLogging::error( errorMessage );
    }

    project->activeOilField()->completionTemplateCollection()->setDefaultUnitSystemBasedOnLoadedCases();

    RiuPlotMainWindowTools::refreshToolbars();

    if ( openedFilesOut )
    {
        *openedFilesOut = openedFiles;
    }

    return !openedFiles.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaImportEclipseCaseTools::openEclipseCaseFromFile( const QString& fileName )
{
    if ( !caf::Utils::fileExists( fileName ) ) return -1;

    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( fileName, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilter( const QString& fileName )
{
    if ( !caf::Utils::fileExists( fileName ) ) return false;

    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( fileName, true ) >= 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaImportEclipseCaseTools::openEclipseInputCaseFromFileNames( const QStringList& fileNames,
                                                                  QString*           fileContainingGrid /*=nullptr*/ )
{
    RimEclipseInputCase* rimInputReservoir = new RimEclipseInputCase();

    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    project->assignCaseIdToCase( rimInputReservoir );

    bool gridImportSuccess = rimInputReservoir->openDataFileSet( fileNames );
    if ( !gridImportSuccess )
    {
        RiaLogging::error( "Failed to import grid" );
        return -1;
    }

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels()
                                                                         : nullptr;
    if ( analysisModels == nullptr ) return false;

    analysisModels->cases.push_back( rimInputReservoir );

    RimEclipseView* riv = rimInputReservoir->createAndAddReservoirView();

    riv->cellResult()->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );

    riv->loadDataAndUpdate();

    if ( !riv->cellResult()->hasResult() )
    {
        riv->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
    }

    analysisModels->updateConnectedEditors();

    Riu3DMainWindowTools::selectAsCurrentItem( riv->cellResult() );

    if ( fileContainingGrid )
    {
        *fileContainingGrid = rimInputReservoir->gridFileName();
    }

    return rimInputReservoir->caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openMockModel( const QString& name )
{
    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( name, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( const QString& fileName, bool showTimeStepFilter )
{
    QFileInfo gridFileName( fileName );
    QString   caseName = gridFileName.completeBaseName();

    RimEclipseResultCase* rimResultReservoir = new RimEclipseResultCase();
    rimResultReservoir->setCaseInfo( caseName, fileName );

    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels()
                                                                         : nullptr;
    if ( analysisModels == nullptr )
    {
        delete rimResultReservoir;
        return -1;
    }

    if ( RiaGuiApplication::isRunning() )
    {
        RicShowMainWindowFeature::showMainWindow();
    }

    analysisModels->cases.push_back( rimResultReservoir );

    if ( !rimResultReservoir->importGridAndResultMetaData( showTimeStepFilter ) )
    {
        analysisModels->removeCaseFromAllGroups( rimResultReservoir );

        delete rimResultReservoir;

        return -1;
    }

    RimEclipseView* riv = rimResultReservoir->createAndAddReservoirView();

    riv->loadDataAndUpdate();

    if ( !riv->cellResult()->hasResult() )
    {
        riv->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
    }

    analysisModels->updateConnectedEditors();

    if ( RiaGuiApplication::isRunning() )
    {
        RiuMainWindow::instance()->selectAsCurrentItem( riv->cellResult() );
    }
    return rimResultReservoir->caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::addEclipseCases( const QStringList&          fileNames,
                                                 RimIdenticalGridCaseGroup** resultingCaseGroup /*=nullptr*/ )
{
    if ( fileNames.size() == 0 ) return true;

    // First file is read completely including grid.
    // The main grid from the first case is reused directly in for the other cases.
    // When reading active cell info, only the total cell count is tested for consistency
    RimEclipseResultCase*         mainResultCase = nullptr;
    std::vector<std::vector<int>> mainCaseGridDimensions;
    RimIdenticalGridCaseGroup*    gridCaseGroup = nullptr;

    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    {
        QString   firstFileName = fileNames[0];
        QFileInfo gridFileName( firstFileName );

        QString caseName = gridFileName.completeBaseName();

        RimEclipseResultCase* rimResultReservoir = new RimEclipseResultCase();
        rimResultReservoir->setCaseInfo( caseName, firstFileName );
        if ( !rimResultReservoir->openEclipseGridFile() )
        {
            delete rimResultReservoir;

            return false;
        }

        rimResultReservoir->readGridDimensions( mainCaseGridDimensions );

        mainResultCase        = rimResultReservoir;
        RimOilField* oilField = project->activeOilField();
        if ( oilField && oilField->analysisModels() )
        {
            gridCaseGroup = oilField->analysisModels->createIdenticalCaseGroupFromMainCase( mainResultCase );
        }
    }

    caf::ProgressInfo info( fileNames.size(), "Reading Active Cell data" );

    for ( int i = 1; i < fileNames.size(); i++ )
    {
        QString   caseFileName = fileNames[i];
        QFileInfo gridFileName( caseFileName );

        QString caseName = gridFileName.completeBaseName();

        RimEclipseResultCase* rimResultReservoir = new RimEclipseResultCase();
        rimResultReservoir->setCaseInfo( caseName, caseFileName );

        std::vector<std::vector<int>> caseGridDimensions;
        rimResultReservoir->readGridDimensions( caseGridDimensions );

        bool identicalGrid = RigGridManager::isGridDimensionsEqual( mainCaseGridDimensions, caseGridDimensions );
        if ( identicalGrid )
        {
            if ( rimResultReservoir->openAndReadActiveCellData( mainResultCase->eclipseCaseData() ) )
            {
                RimOilField* oilField = project->activeOilField();
                if ( oilField && oilField->analysisModels() )
                {
                    oilField->analysisModels()->insertCaseInCaseGroup( gridCaseGroup, rimResultReservoir );
                }
            }
            else
            {
                delete rimResultReservoir;
            }
        }
        else
        {
            delete rimResultReservoir;
        }

        info.setProgress( i );
    }

    if ( gridCaseGroup )
    {
        // Create placeholder results and propagate results info from main case to all other cases
        gridCaseGroup->loadMainCaseAndActiveCellInfo();

        if ( resultingCaseGroup )
        {
            *resultingCaseGroup = gridCaseGroup;
        }
    }

    project->activeOilField()->analysisModels()->updateConnectedEditors();

    if ( RiaGuiApplication::isRunning() && gridCaseGroup && gridCaseGroup->statisticsCaseCollection()->reservoirs.size() > 0 )
    {
        RiuMainWindow::instance()->selectAsCurrentItem( gridCaseGroup->statisticsCaseCollection()->reservoirs[0] );
    }

    return true;
}
