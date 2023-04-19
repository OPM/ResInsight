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

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "SummaryPlotCommands/RicNewSummaryCurveFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RifEclipseSummaryTools.h"
#include "RifReaderSettings.h"
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
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRoffCase.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
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
bool RiaImportEclipseCaseTools::openEclipseCasesFromFile( const QStringList&                 fileNames,
                                                          bool                               createView,
                                                          FileCaseIdMap*                     openedFilesOut,
                                                          bool                               noDialog,
                                                          std::shared_ptr<RifReaderSettings> readerSettings )
{
    RimProject* project = RimProject::current();
    if ( !project ) return false;

    // Get list of files to import
    RifSummaryCaseRestartSelector selector;
    if ( noDialog || !RiaGuiApplication::isRunning() ) selector.showDialog( false );
    selector.determineFilesToImportFromGridFiles( fileNames );
    std::vector<RifSummaryCaseFileResultInfo> summaryFileInfos = selector.summaryFileInfos();

    FileCaseIdMap openedFiles;

    // Import eclipse case files
    for ( const QString& gridCaseFile : selector.gridCaseFiles() )
    {
        int caseId = RiaImportEclipseCaseTools::openEclipseCaseFromFile( gridCaseFile, createView, readerSettings );
        if ( caseId >= 0 )
        {
            openedFiles.insert( std::make_pair( gridCaseFile, caseId ) );
        }
    }

    if ( !openedFiles.empty() )
    {
        RimMainPlotCollection::current()->ensureDefaultFlowPlotsAreCreated();
    }

    // The default value for summary case import is true, but we use the state from RifReaderSettings if defined
    //
    // TODO:
    // Refactor RifReaderSettings, separate the data structure sent to reader from the data structure in
    // preferences. See RifReaderSettings::createGridOnlyReaderSettings() for the only use of importSummaryData flag
    //
    bool importSummaryCases = true;
    if ( readerSettings ) importSummaryCases = readerSettings->importSummaryData;

    if ( importSummaryCases && !summaryFileInfos.empty() )
    {
        RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection()
                                                                              : nullptr;
        if ( sumCaseColl )
        {
            std::vector<RimSummaryCase*> candidateCases = sumCaseColl->createSummaryCasesFromFileInfos( summaryFileInfos );
            std::vector<RimSummaryCase*> duplicatedCases;

            for ( RimSummaryCase* newSumCase : candidateCases )
            {
                RimSummaryCaseCollection* existingCollection = nullptr;
                auto existingSummaryCase = sumCaseColl->findTopLevelSummaryCaseFromFileName( newSumCase->summaryHeaderFilename() );
                if ( existingSummaryCase )
                {
                    existingSummaryCase->firstAncestorOrThisOfType( existingCollection );

                    // Replace file summary case pointers in Rft Curves
                    std::vector<RimWellLogRftCurve*> rftCurves;
                    existingSummaryCase->objectsWithReferringPtrFieldsOfType( rftCurves );
                    for ( RimWellLogRftCurve* curve : rftCurves )
                    {
                        if ( curve->summaryCase() == existingSummaryCase )
                        {
                            curve->setSummaryCase( newSumCase );
                        }
                    }

                    // Replace all occurrences of file sum with ecl sum

                    std::vector<RimSummaryCurve*> objects;
                    existingSummaryCase->objectsWithReferringPtrFieldsOfType( objects );

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
                    sumCaseColl->removeCase( existingSummaryCase );

                    duplicatedCases.push_back( existingSummaryCase );
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

            // Delete cases that already was present
            for ( auto duplicateCase : duplicatedCases )
            {
                candidateCases.erase( std::remove( candidateCases.begin(), candidateCases.end(), duplicateCase ), candidateCases.end() );
                delete duplicateCase;
            }

            if ( !candidateCases.empty() )
            {
                RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( { candidateCases.front() }, {} );
                RiuPlotMainWindowTools::setExpanded( candidateCases.front() );
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

    if ( RiaGuiApplication::isRunning() )
    {
        // Call process events to clear the queue. This make sure that we are able raise the 3D window on top of the
        // plot window. Otherwise the event processing ends up with the plot window on top.
        QApplication::processEvents();
        RiuMainWindow::instance()->activateWindow();
    }

    if ( openedFilesOut )
    {
        *openedFilesOut = openedFiles;
    }

    return !openedFiles.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaImportEclipseCaseTools::openEclipseCaseFromFile( const QString& fileName, bool createView, std::shared_ptr<RifReaderSettings> readerSettings )
{
    if ( !caf::Utils::fileExists( fileName ) ) return -1;

    bool showTimeStepFilter = false;
    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( fileName, showTimeStepFilter, createView, readerSettings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilter( const QString& fileName )
{
    if ( !caf::Utils::fileExists( fileName ) ) return false;

    bool showTimeStepFilter = true;
    bool createView         = true;
    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( fileName, showTimeStepFilter, createView, nullptr ) >= 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaImportEclipseCaseTools::openEclipseInputCaseFromFileNames( const QStringList& fileNames, bool createDefaultView )
{
    auto* rimInputReservoir = new RimEclipseInputCase();

    RimProject* project = RimProject::current();
    if ( !project ) return -1;

    project->assignCaseIdToCase( rimInputReservoir );

    bool gridImportSuccess = rimInputReservoir->openDataFileSet( fileNames );
    if ( !gridImportSuccess )
    {
        RiaLogging::error( "Failed to import grid" );
        return -1;
    }

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if ( analysisModels == nullptr ) return false;

    analysisModels->cases.push_back( rimInputReservoir );

    RimEclipseView* eclipseView = nullptr;
    if ( createDefaultView )
    {
        eclipseView = rimInputReservoir->createAndAddReservoirView();

        eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );

        eclipseView->loadDataAndUpdate();

        if ( !eclipseView->cellResult()->hasResult() )
        {
            eclipseView->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
        }
    }

    analysisModels->updateConnectedEditors();

    if ( eclipseView )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( eclipseView->cellResult() );
    }

    return rimInputReservoir->caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openMockModel( const QString& name )
{
    bool showTimeStepFilter = false;
    bool createView         = true;
    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( name, showTimeStepFilter, createView, nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( const QString&                     fileName,
                                                                      bool                               showTimeStepFilter,
                                                                      bool                               createView,
                                                                      std::shared_ptr<RifReaderSettings> readerSettings )
{
    QFileInfo gridFileName( fileName );
    QString   caseName = gridFileName.completeBaseName();

    auto* rimResultReservoir = new RimEclipseResultCase();
    rimResultReservoir->setCaseInfo( caseName, fileName );
    rimResultReservoir->setReaderSettings( readerSettings );

    RimProject* project = RimProject::current();
    if ( !project ) return -1;

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
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

    if ( createView )
    {
        RimEclipseView* riv = rimResultReservoir->createAndAddReservoirView();

        riv->loadDataAndUpdate();

        if ( !riv->cellResult()->hasResult() )
        {
            riv->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
        }

        analysisModels->updateConnectedEditors();

        if ( RiaGuiApplication::isRunning() )
        {
            if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->selectAsCurrentItem( riv->cellResult() );
        }
    }
    else
    {
        // Make sure the placeholder result entries are created, as this functionality is triggered when creating a
        // view. See RimEclipseView::onLoadDataAndUpdate() and the call to openReserviorCase()
        rimResultReservoir->openReserviorCase();

        analysisModels->updateConnectedEditors();
    }

    return rimResultReservoir->caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::addEclipseCases( const QStringList& fileNames, RimIdenticalGridCaseGroup** resultingCaseGroup /*=nullptr*/ )
{
    if ( fileNames.empty() ) return true;

    // First file is read completely including grid.
    // The main grid from the first case is reused directly in for the other cases.
    // When reading active cell info, only the total cell count is tested for consistency
    RimEclipseResultCase*         mainResultCase = nullptr;
    std::vector<std::vector<int>> mainCaseGridDimensions;
    RimIdenticalGridCaseGroup*    gridCaseGroup = nullptr;

    RimProject* project = RimProject::current();
    if ( !project ) return false;

    {
        QString   firstFileName = fileNames[0];
        QFileInfo gridFileName( firstFileName );

        QString caseName = gridFileName.completeBaseName();

        auto* rimResultReservoir = new RimEclipseResultCase();
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

        auto* rimResultReservoir = new RimEclipseResultCase();
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

    if ( RiaGuiApplication::isRunning() && gridCaseGroup && !gridCaseGroup->statisticsCaseCollection()->reservoirs.empty() )
    {
        if ( RiuMainWindow::instance() )
            RiuMainWindow::instance()->selectAsCurrentItem( gridCaseGroup->statisticsCaseCollection()->reservoirs[0] );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RiaImportEclipseCaseTools::openRoffCasesFromFileNames( const QStringList& fileNames, bool createDefaultView )
{
    CAF_ASSERT( !fileNames.empty() );

    RimProject* project = RimProject::current();
    if ( !project ) return {};

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if ( !analysisModels ) return {};

    std::vector<int> roffCaseIds;
    for ( const auto& fileName : fileNames )
    {
        auto* roffCase = new RimRoffCase();
        project->assignCaseIdToCase( roffCase );
        roffCase->setGridFileName( fileName );

        bool gridImportSuccess = roffCase->openEclipseGridFile();
        if ( !gridImportSuccess )
        {
            const auto errMsg = "Failed to import grid from file: " + fileName.toStdString();
            RiaLogging::error( errMsg.c_str() );
            continue;
        }

        analysisModels->cases.push_back( roffCase );

        RimEclipseView* eclipseView = nullptr;
        if ( createDefaultView )
        {
            eclipseView = roffCase->createAndAddReservoirView();

            eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );

            if ( RiaGuiApplication::isRunning() )
            {
                if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->selectAsCurrentItem( eclipseView->cellResult() );
            }

            eclipseView->loadDataAndUpdate();
        }

        analysisModels->updateConnectedEditors();
        roffCaseIds.push_back( roffCase->caseId() );
    }
    return roffCaseIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRoffCase* RiaImportEclipseCaseTools::openRoffCaseFromFileName( const QString& fileName, bool createDefaultView )
{
    RimProject* project = RimProject::current();
    if ( !project ) return nullptr;

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if ( !analysisModels ) return nullptr;

    auto* roffCase = new RimRoffCase();
    project->assignCaseIdToCase( roffCase );
    roffCase->setGridFileName( fileName );

    bool gridImportSuccess = roffCase->openEclipseGridFile();
    if ( !gridImportSuccess )
    {
        const auto errMsg = "Failed to import grid from file: " + fileName.toStdString();
        RiaLogging::error( errMsg.c_str() );
        return nullptr;
    }

    analysisModels->cases.push_back( roffCase );

    RimEclipseView* eclipseView = nullptr;
    if ( createDefaultView )
    {
        eclipseView = roffCase->createAndAddReservoirView();

        eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );

        if ( RiaGuiApplication::isRunning() )
        {
            if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->selectAsCurrentItem( eclipseView->cellResult() );
        }

        eclipseView->loadDataAndUpdate();
    }

    analysisModels->updateConnectedEditors();

    return roffCase;
}
