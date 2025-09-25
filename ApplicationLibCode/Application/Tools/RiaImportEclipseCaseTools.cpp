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

#include "Summary/RiaSummaryPlotTools.h"
#include "SummaryPlotCommands/RicNewSummaryCurveFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferencesGrid.h"
#include "RiaResultNames.h"
#include "RiaViewRedrawScheduler.h"

#include "RifEclipseSummaryTools.h"
#include "RifSummaryCaseRestartSelector.h"
#include "RifSummaryReaderInterface.h"

#include "RigGridManager.h"

#include "RimCaseCollection.h"
#include "RimCompletionTemplateCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEmCase.h"
#include "RimFileSummaryCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRoffCase.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryEnsemble.h"
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
bool RiaImportEclipseCaseTools::openEclipseCasesFromFile( const QStringList& fileNames, bool createView, FileCaseIdMap* openedFilesOut, bool noDialog )
{
    RifReaderSettings rs = RiaPreferencesGrid::current()->readerSettings();
    return openEclipseCasesFromFile( fileNames, createView, openedFilesOut, noDialog, rs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEclipseCasesFromFile( const QStringList& fileNames,
                                                          bool               createView,
                                                          FileCaseIdMap*     openedFilesOut,
                                                          bool               noDialog,
                                                          RifReaderSettings& readerSettings )
{
    RimProject* project = RimProject::current();
    if ( !project ) return false;

    // Get list of files to import
    RifSummaryCaseRestartSelector selector;
    if ( noDialog || !RiaGuiApplication::isRunning() ) selector.showDialog( false );
    selector.determineFilesToImportFromGridFiles( fileNames );
    std::vector<RifSummaryCaseFileResultInfo> summaryFileInfos = selector.summaryFileInfos();

    FileCaseIdMap openedFiles;

    // Block updates until import of summary data is completed. QApplication::processEvents() is called during import of summary data, and
    // this will trigger redraw of the 3D views in RiaViewRedrawScheduler
    RiaViewRedrawScheduler::instance()->blockUpdate( true );

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

    if ( readerSettings.importSummaryData && !summaryFileInfos.empty() )
    {
        RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection()
                                                                              : nullptr;
        if ( sumCaseColl )
        {
            const bool                   readStateFromFirstFile = false;
            std::vector<RimSummaryCase*> candidateCases =
                sumCaseColl->createSummaryCasesFromFileInfos( summaryFileInfos, readStateFromFirstFile );
            std::vector<RimSummaryCase*> duplicatedCases;

            for ( RimSummaryCase* newSumCase : candidateCases )
            {
                if ( auto reader = newSumCase->summaryReader() )
                {
                    reader->createAddressesIfRequired();
                }

                RimSummaryEnsemble* existingCollection = nullptr;
                auto existingSummaryCase = sumCaseColl->findTopLevelSummaryCaseFromFileName( newSumCase->summaryHeaderFilename() );
                if ( existingSummaryCase )
                {
                    existingCollection = existingSummaryCase->firstAncestorOrThisOfType<RimSummaryEnsemble>();

                    // Replace file summary case pointers in Rft Curves
                    auto rftCurves = existingSummaryCase->objectsWithReferringPtrFieldsOfType<RimWellLogRftCurve>();
                    for ( RimWellLogRftCurve* curve : rftCurves )
                    {
                        if ( curve->summaryCase() == existingSummaryCase )
                        {
                            curve->setSummaryCase( newSumCase );
                        }
                    }

                    // Replace all occurrences of file sum with ecl sum

                    auto objects = existingSummaryCase->objectsWithReferringPtrFieldsOfType<RimSummaryCurve>();

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

                            auto parentCollection = summaryCurve->firstAncestorOrThisOfType<RimSummaryCurveCollection>();
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

            if ( !candidateCases.empty() && RiaGuiApplication::isRunning() && RiuPlotMainWindow::instance()->isVisible() )
            {
                RiaSummaryPlotTools::createAndAppendDefaultSummaryMultiPlot( { candidateCases.front() }, {} );
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

    RiaViewRedrawScheduler::instance()->blockUpdate( false );

    if ( RiaGuiApplication::isRunning() )
    {
        if ( RiuPlotMainWindow::instance()->isVisible() ) RiuPlotMainWindowTools::refreshToolbars();

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
int RiaImportEclipseCaseTools::openEclipseCaseFromFile( const QString& fileName, bool createView, RifReaderSettings& readerSettings )
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

    bool              showTimeStepFilter = true;
    bool              createView         = true;
    RifReaderSettings rs                 = RiaPreferencesGrid::current()->readerSettings();
    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( fileName, showTimeStepFilter, createView, rs ) >= 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaImportEclipseCaseTools::CaseFileNameAndId
    RiaImportEclipseCaseTools::openEclipseInputCaseAndPropertiesFromFileNames( const QStringList& fileNames, bool createDefaultView )
{
    RimProject* project = RimProject::current();
    if ( !project ) return { QString(), -1 };

    auto* rimInputReservoir = new RimEclipseInputCase();
    project->assignCaseIdToCase( rimInputReservoir );

    bool gridImportSuccess = rimInputReservoir->openDataFileSet( fileNames );
    if ( !gridImportSuccess )
    {
        RiaLogging::error( "Failed to import grid" );
        delete rimInputReservoir;
        return { QString(), -1 };
    }

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if ( analysisModels == nullptr ) return { QString(), -1 };

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

    return { rimInputReservoir->gridFileName(), rimInputReservoir->caseId() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RiaImportEclipseCaseTools::openEclipseInputCasesFromFileNames( const QStringList& fileNames, bool createDefaultView )
{
    std::vector<int> eclipseCaseIds;
    for ( const auto& fileName : fileNames )
    {
        // Open with single file
        auto [caseFileName, caseId] = openEclipseInputCaseAndPropertiesFromFileNames( { fileName }, createDefaultView );
        eclipseCaseIds.push_back( caseId );
    }
    return eclipseCaseIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openMockModel( const QString& name )
{
    bool              showTimeStepFilter = false;
    bool              createView         = true;
    RifReaderSettings rs                 = RiaPreferencesGrid::current()->readerSettings();
    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( name, showTimeStepFilter, createView, rs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl( const QString&     fileName,
                                                                      bool               showTimeStepFilter,
                                                                      bool               createView,
                                                                      RifReaderSettings& readerSettings )
{
    RimProject* project = RimProject::current();
    if ( !project ) return -1;

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if ( analysisModels == nullptr ) return -1;

    QFileInfo gridFileName( fileName );
    QString   caseName = gridFileName.completeBaseName();

    auto* rimResultReservoir = new RimEclipseResultCase();
    rimResultReservoir->setCaseInfo( caseName, fileName );
    rimResultReservoir->setReaderSettings( readerSettings );

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

    RimMainPlotCollection::current()->ensureDefaultFlowPlotsAreCreated();

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
        // view. See RimEclipseView::onLoadDataAndUpdate() and the call to openReservoirCase()
        rimResultReservoir->openReservoirCase();

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
            delete roffCase;
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
        delete roffCase;
        return nullptr;
    }

    analysisModels->cases.push_back( roffCase );
    analysisModels->updateConnectedEditors();

    RimEclipseView* eclipseView = nullptr;
    if ( createDefaultView )
    {
        eclipseView = roffCase->createAndAddReservoirView();

        eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );
        eclipseView->loadDataAndUpdate();

        roffCase->updateAllRequiredEditors();
        if ( RiaGuiApplication::isRunning() )
        {
            if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->selectAsCurrentItem( eclipseView->cellResult() );

            // Make sure the call to setExpanded is done after the call to selectAsCurrentItem
            Riu3DMainWindowTools::setExpanded( eclipseView );
        }
    }

    return roffCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEmFilesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds )
{
    if ( fileNames.empty() ) return false;

    RimProject* project = RimProject::current();
    if ( !project ) return false;

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if ( !analysisModels ) return false;

    for ( auto fileName : fileNames )
    {
        auto* emCase = new RimEmCase();
        project->assignCaseIdToCase( emCase );
        emCase->setGridFileName( fileName );

        bool gridImportSuccess = emCase->openEclipseGridFile();
        if ( !gridImportSuccess )
        {
            const auto errMsg = "Failed to import grid from file: " + fileName.toStdString();
            RiaLogging::error( errMsg.c_str() );
            delete emCase;
            continue;
        }

        analysisModels->cases.push_back( emCase );
        analysisModels->updateConnectedEditors();

        RimEclipseView* eclipseView = nullptr;
        if ( createDefaultView )
        {
            eclipseView = emCase->createAndAddReservoirView();

            eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );
            eclipseView->loadDataAndUpdate();

            emCase->updateAllRequiredEditors();
            if ( RiaGuiApplication::isRunning() )
            {
                if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->selectAsCurrentItem( eclipseView->cellResult() );

                // Make sure the call to setExpanded is done after the call to selectAsCurrentItem
                Riu3DMainWindowTools::setExpanded( eclipseView );
            }
        }
    }

    return true;
}
