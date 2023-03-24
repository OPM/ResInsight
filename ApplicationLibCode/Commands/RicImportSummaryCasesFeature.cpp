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
bool RicImportSummaryCasesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app           = RiaGuiApplication::instance();
    QString            pathCacheName = "INPUT_FILES";
    QStringList        fileNames     = runRecursiveSummaryCaseFileSearchDialog( "Import Summary Cases", pathCacheName );

    std::vector<RimSummaryCase*> cases;
    if ( !fileNames.isEmpty() ) createSummaryCasesFromFiles( fileNames, &cases );

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

    std::vector<RimCase*> allCases;
    app->project()->allCases( allCases );

    if ( allCases.size() == 0 )
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
bool RicImportSummaryCasesFeature::createAndAddSummaryCasesFromFiles( const QStringList&            fileNames,
                                                                      bool                          doCreateDefaultPlot,
                                                                      std::vector<RimSummaryCase*>* newCases )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    std::vector<RimSummaryCase*>  temp;
    std::vector<RimSummaryCase*>* cases = newCases ? newCases : &temp;
    if ( createSummaryCasesFromFiles( fileNames, cases ) )
    {
        addSummaryCases( *cases );
        if ( !cases->empty() && doCreateDefaultPlot )
        {
            RimSummaryMultiPlot* plotToSelect = nullptr;
            for ( auto sumCase : *cases )
            {
                plotToSelect = RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( { sumCase }, {} );
            }

            if ( plotToSelect )
            {
                RiuPlotMainWindowTools::onObjectAppended( plotToSelect );
            }
        }

        RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
        if ( mainPlotWindow && !cases->empty() )
        {
            mainPlotWindow->updateMultiPlotToolBar();

            // Close main window if there are no eclipse cases imported
            std::vector<RimCase*> allCases;
            app->project()->allCases( allCases );

            if ( allCases.size() == 0 )
            {
                RiuMainWindow::closeIfOpen();
            }
        }
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::createSummaryCasesFromFiles( const QStringList&            fileNames,
                                                                std::vector<RimSummaryCase*>* newCases,
                                                                bool                          ensembleOrGroup,
                                                                bool                          allowDialogs )
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;

    if ( newCases ) newCases->clear();
    if ( !sumCaseColl ) return false;

    RifSummaryCaseRestartSelector fileSelector;

    if ( !RiaGuiApplication::isRunning() || !allowDialogs )
    {
        fileSelector.showDialog( false );
    }

    fileSelector.setEnsembleOrGroupMode( ensembleOrGroup );
    fileSelector.determineFilesToImportFromSummaryFiles( fileNames );

    std::vector<RifSummaryCaseFileResultInfo> importFileInfos = fileSelector.summaryFileInfos();

    if ( !importFileInfos.empty() )
    {
        std::vector<RimSummaryCase*> sumCases = sumCaseColl->createSummaryCasesFromFileInfos( importFileInfos, true );
        if ( newCases ) newCases->insert( newCases->end(), sumCases.begin(), sumCases.end() );
    }

    if ( fileSelector.foundErrors() )
    {
        QString errorMessage = fileSelector.createCombinedErrorMessage();
        RiaLogging::error( errorMessage );
    }

    return true;
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
        RimSummaryCaseCollection*     coll = selectedColl.front();
        RimSummaryCaseMainCollection* mainColl;
        coll->firstAncestorOrThisOfType( mainColl );

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
std::pair<QStringList, RiaEnsembleNameTools::EnsembleGroupingMode>
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

    if ( !result.ok ) return std::make_pair( QStringList(), RiaEnsembleNameTools::EnsembleGroupingMode::NONE );

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return std::make_pair( result.files, result.groupingMode );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialog( const QString& dialogTitle, const QString& pathCacheName )
{
    auto result = runRecursiveSummaryCaseFileSearchDialogWithGrouping( dialogTitle, pathCacheName );
    return result.first;
}
