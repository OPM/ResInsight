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

#include "RicImportSummaryCasesFeature.h"

#include "RiaSummaryDefines.h"
#include "SummaryPlotCommands/RicNewSummaryCurveFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RicRecursiveFileSearchDialog.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "SummaryPlotCommands/RicNewSummaryEnsembleCurveSetFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportSummaryCasesFeature, "RicImportSummaryCasesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportSummaryCasesFeature::m_pathFilter     = "*";
QString RicImportSummaryCasesFeature::m_fileNameFilter = "*";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication*   app           = RiaGuiApplication::instance();
    QString              pathCacheName = "INPUT_FILES";
    auto                 result        = runRecursiveSummaryCaseFileSearchDialog( "Import Summary Cases", pathCacheName );
    QStringList          fileNames     = result.files;
    RiaDefines::FileType fileType      = RicRecursiveFileSearchDialog::mapSummaryFileType( result.fileType );

    std::vector<RimSummaryCase*> cases;

    if ( !fileNames.isEmpty() )
    {
        CreateConfig createConfig{ .fileType = fileType, .ensembleOrGroup = false, .allowDialogs = true };
        auto [isOk, cases] = createSummaryCasesFromFiles( fileNames, createConfig );
    }

    addSummaryCases( cases );
    if ( !cases.empty() )
    {
        for ( auto sumcase : cases )
        {
            RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( { sumcase }, {} );
        }
    }

    addCasesToGroupIfRelevant( cases );

    for ( const auto& rimCase : cases )
        app->addToRecentFiles( rimCase->summaryHeaderFilename() );

    RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if ( mainPlotWindow && !cases.empty() )
    {
        mainPlotWindow->updateMultiPlotToolBar();
    }

    std::vector<RimCase*> allCases = app->project()->allGridCases();
    if ( allCases.empty() )
    {
        RiuMainWindow::closeIfOpen();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryCases16x16.png" ) );
    actionToSetup->setText( "Import Summary Cases Recursively" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<RimSummaryCase*>> RicImportSummaryCasesFeature::createAndAddSummaryCasesFromFiles( const QStringList& fileNames,
                                                                                                               bool doCreateDefaultPlot )

{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    CreateConfig createConfig{ .fileType = RiaDefines::FileType::SMSPEC, .ensembleOrGroup = false, .allowDialogs = true };
    auto [isOk, cases] = createSummaryCasesFromFiles( fileNames, createConfig );
    if ( isOk )
    {
        addSummaryCases( cases );
        if ( !cases.empty() && doCreateDefaultPlot )
        {
            RimSummaryMultiPlot* plotToSelect = nullptr;
            for ( auto sumCase : cases )
            {
                plotToSelect = RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( { sumCase }, {} );
            }

            if ( plotToSelect )
            {
                RiuPlotMainWindowTools::onObjectAppended( plotToSelect );
            }
        }

        RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
        if ( mainPlotWindow && !cases.empty() )
        {
            mainPlotWindow->updateMultiPlotToolBar();

            // Close main window if there are no eclipse cases imported
            std::vector<RimCase*> allCases = app->project()->allGridCases();
            if ( allCases.empty() )
            {
                RiuMainWindow::closeIfOpen();
            }
        }
        return std::make_pair( true, cases );
    }

    return std::make_pair( false, cases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<RimSummaryCase*>> RicImportSummaryCasesFeature::createSummaryCasesFromFiles( const QStringList& fileNames,
                                                                                                         CreateConfig       createConfig )
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;

    std::vector<RimSummaryCase*> newCases;

    if ( !sumCaseColl ) return std::make_pair( false, newCases );

    std::vector<RifSummaryCaseFileResultInfo> importFileInfos;
    if ( createConfig.fileType == RiaDefines::FileType::SMSPEC )
    {
        RifSummaryCaseRestartSelector fileSelector;

        if ( !RiaGuiApplication::isRunning() || !createConfig.allowDialogs )
        {
            fileSelector.showDialog( false );
        }

        fileSelector.setEnsembleOrGroupMode( createConfig.ensembleOrGroup );
        fileSelector.determineFilesToImportFromSummaryFiles( fileNames );

        importFileInfos = fileSelector.summaryFileInfos();

        if ( fileSelector.foundErrors() )
        {
            QString errorMessage = fileSelector.createCombinedErrorMessage();
            RiaLogging::error( errorMessage );
        }
    }
    else
    {
        // No restart files for these file types: just copy to result info
        for ( auto f : fileNames )
        {
            importFileInfos.push_back( RifSummaryCaseFileResultInfo( f, false, createConfig.fileType ) );
        }
    }

    if ( !importFileInfos.empty() )
    {
        std::vector<RimSummaryCase*> sumCases = sumCaseColl->createSummaryCasesFromFileInfos( importFileInfos, true );
        newCases.insert( newCases.end(), sumCases.begin(), sumCases.end() );
    }

    return std::make_pair( true, newCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::addSummaryCases( const std::vector<RimSummaryCase*>& cases )
{
    RiaApplication*               app         = RiaApplication::instance();
    RimProject*                   proj        = app->project();
    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    if ( !sumCaseColl ) return;

    bool expandFirstCase = sumCaseColl->allSummaryCases().empty();
    sumCaseColl->addCases( cases );

    sumCaseColl->updateAllRequiredEditors();

    if ( expandFirstCase && !cases.empty() )
    {
        RiuPlotMainWindowTools::setExpanded( cases.front() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::addCasesToGroupIfRelevant( const std::vector<RimSummaryCase*>& cases )
{
    std::vector<RimSummaryCaseCollection*> selectedColl = caf::selectedObjectsByTypeStrict<RimSummaryCaseCollection*>();

    if ( selectedColl.size() == 1 )
    {
        RimSummaryCaseCollection*     coll     = selectedColl.front();
        RimSummaryCaseMainCollection* mainColl = coll->firstAncestorOrThisOfType<RimSummaryCaseMainCollection>();

        if ( mainColl )
        {
            for ( const auto sumCase : cases )
            {
                mainColl->removeCase( sumCase );
                selectedColl.front()->addCase( sumCase );
            }
            mainColl->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialogResult
    RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialogWithGrouping( const QString& dialogTitle, const QString& pathCacheName )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( pathCacheName );

    auto fileTypes = { RicRecursiveFileSearchDialog::FileType::SMSPEC,
                       RicRecursiveFileSearchDialog::FileType::REVEAL_SUMMARY,
                       RicRecursiveFileSearchDialog::FileType::STIMPLAN_SUMMARY };

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr, dialogTitle, defaultDir, m_pathFilter, m_fileNameFilter, fileTypes );

    // Remember filters
    m_pathFilter     = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if ( !result.ok ) return result;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialogResult RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialog( const QString& dialogTitle,
                                                                                                          const QString& pathCacheName )
{
    return runRecursiveSummaryCaseFileSearchDialogWithGrouping( dialogTitle, pathCacheName );
}
