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

#include "RicImportEnsembleFeature.h"

#include "Ensemble/RiaEnsembleImportTools.h"
#include "RiaApplication.h"
#include "RiaEnsembleNameTools.h"
#include "Summary/RiaSummaryDefines.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RicImportSummaryCasesFeature.h"

#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetCollection.h"
#include "EnsembleFileSet/RimEnsembleFileSetTools.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "Summary/Ensemble/RimSummaryFileSetEnsemble.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include <QAction>
#include <QInputDialog>

CAF_CMD_SOURCE_INIT( RicImportEnsembleFeature, "RicImportEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RicImportEnsembleFeature::createSummaryEnsemble( std::vector<RimSummaryCase*> cases )
{
    if ( cases.empty() ) return nullptr;

    RiaDefines::EnsembleGroupingMode groupingMode = RiaDefines::EnsembleGroupingMode::NONE;
    QString                          name         = "Ensemble";
    if ( cases.front()->ensemble() )
    {
        groupingMode = cases.front()->ensemble()->groupingMode();
        name         = cases.front()->ensemble()->name();
    }

    QStringList fileNames;
    for ( auto summaryCase : cases )
    {
        fileNames.append( summaryCase->summaryHeaderFilename() );
    }

    bool useEnsembleNameDialog = false;
    return importSingleEnsembleFileSet( fileNames, useEnsembleNameDialog, groupingMode, RiaDefines::FileType::SMSPEC, name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::onActionTriggered( bool isChecked )
{
    QString pathCacheName = "ENSEMBLE_FILES";
    auto    result = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialogWithGrouping( "Import Ensemble", pathCacheName );
    QStringList                      fileNames            = result.files;
    RiaDefines::EnsembleGroupingMode ensembleGroupingMode = result.groupingMode;
    RiaDefines::FileType             fileType             = RicRecursiveFileSearchDialog::mapSummaryFileType( result.fileType );

    if ( fileNames.isEmpty() ) return;

    if ( ensembleGroupingMode == RiaDefines::EnsembleGroupingMode::NONE )
    {
        bool useEnsembleNameDialog = true;
        importSingleEnsemble( fileNames, useEnsembleNameDialog, ensembleGroupingMode, fileType );
    }
    else
    {
        if ( fileType == RiaDefines::FileType::STIMPLAN_SUMMARY || fileType == RiaDefines::FileType::REVEAL_SUMMARY )
        {
            std::map<QString, QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByCustomEnsemble( fileNames, fileType );
            for ( const auto& [ensembleName, groupedFileNames] : groupedByEnsemble )
            {
                bool useEnsembleNameDialog = false;
                importSingleEnsemble( groupedFileNames, useEnsembleNameDialog, ensembleGroupingMode, fileType, ensembleName );
            }
        }
        else
        {
            std::vector<RimSummaryEnsemble*> ensembles;
            auto                             grouping = RiaEnsembleNameTools::groupFilesByEnsembleName( fileNames, ensembleGroupingMode );
            for ( const auto& [groupName, fileNames] : grouping )
            {
                bool useEnsembleNameDialog = false;
                if ( auto ensemble = importSingleEnsembleFileSet( fileNames, useEnsembleNameDialog, ensembleGroupingMode, fileType, groupName ) )
                {
                    ensembles.push_back( ensemble );
                }
            }

            RiaSummaryTools::updateSummaryEnsembleNames();

            for ( auto ensemble : ensembles )
            {
                RiaSummaryPlotTools::createAndAppendDefaultSummaryMultiPlot( {}, { ensemble } );
            }
            RiaSummaryTools::summaryCaseMainCollection()->updateConnectedEditors();
        }
    }

    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RicImportEnsembleFeature::importSingleEnsemble( const QStringList&               fileNames,
                                                                    bool                             useEnsembleNameDialog,
                                                                    RiaDefines::EnsembleGroupingMode groupingMode,
                                                                    RiaDefines::FileType             fileType,
                                                                    const QString&                   defaultEnsembleName )
{
    QString ensembleName = !defaultEnsembleName.isEmpty() ? defaultEnsembleName
                                                          : RiaEnsembleNameTools::findSuitableEnsembleName( fileNames, groupingMode );

    if ( useEnsembleNameDialog ) ensembleName = askForEnsembleName( ensembleName );

    if ( ensembleName.isEmpty() ) return nullptr;

    RiaEnsembleImportTools::CreateConfig createConfig{ .fileType              = fileType,
                                                       .ensembleOrGroup       = true,
                                                       .allowDialogs          = true,
                                                       .buildSummaryAddresses = false };

    auto cases = RiaEnsembleImportTools::createSummaryCasesFromFiles( fileNames, createConfig );
    if ( cases.empty() ) return nullptr;

    RimSummaryEnsemble* ensemble = RicImportEnsembleFeature::groupSummaryCases( cases, ensembleName, groupingMode, true );

    if ( ensemble )
    {
        for ( auto summaryCase : ensemble->allSummaryCases() )
        {
            summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME );
            summaryCase->updateAutoShortName();
        }

        RiaSummaryTools::updateSummaryEnsembleNames();

        RiaSummaryPlotTools::createAndAppendDefaultSummaryMultiPlot( {}, { ensemble } );
    }

    std::vector<RimCase*> allCases = RiaApplication::instance()->project()->allGridCases();
    if ( allCases.empty() )
    {
        RiuMainWindow::closeIfOpen();
    }

    return ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RicImportEnsembleFeature::importSingleEnsembleFileSet( const QStringList&               fileNames,
                                                                           bool                             useEnsembleNameDialog,
                                                                           RiaDefines::EnsembleGroupingMode groupingMode,
                                                                           RiaDefines::FileType             fileType,
                                                                           const QString&                   defaultEnsembleName )
{
    auto fileSets  = RimEnsembleFileSetTools::createEnsembleFileSets( fileNames, groupingMode );
    auto ensembles = RimEnsembleFileSetTools::createSummaryEnsemblesFromFileSets( fileSets );
    if ( !ensembles.empty() ) return ensembles.front();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RicImportEnsembleFeature::groupSummaryCases( std::vector<RimSummaryCase*>     cases,
                                                                 const QString&                   groupName,
                                                                 RiaDefines::EnsembleGroupingMode groupingMode,
                                                                 bool                             isEnsemble /*= false */ )
{
    RimSummaryCaseMainCollection* summaryCaseMainCollection = RiaSummaryTools::summaryCaseMainCollection();
    if ( !cases.empty() )
    {
        auto newGroup = summaryCaseMainCollection->addEnsemble( cases, groupName, isEnsemble );
        newGroup->setGroupingMode( groupingMode );
        summaryCaseMainCollection->updateConnectedEditors();

        RiuPlotMainWindowTools::showPlotMainWindow();
        return newGroup;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble.svg" ) );
    actionToSetup->setText( "Import Ensemble" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportEnsembleFeature::askForEnsembleName( const QString& suggestion )
{
    RimProject*                      project                   = RimProject::current();
    std::vector<RimSummaryEnsemble*> ensembles                 = project->summaryEnsembles();
    int                              ensemblesStartingWithRoot = std::count_if( ensembles.begin(),
                                                   ensembles.end(),
                                                   [suggestion]( RimSummaryEnsemble* group )
                                                   { return group->isEnsemble() && group->name().startsWith( suggestion ); } );

    QInputDialog dialog;
    dialog.setInputMode( QInputDialog::TextInput );
    dialog.setWindowTitle( "Ensemble Name" );
    dialog.setLabelText( "Ensemble Name" );
    if ( ensemblesStartingWithRoot > 0 )
    {
        dialog.setTextValue( QString( "%1 %2" ).arg( suggestion ).arg( ensemblesStartingWithRoot + 1 ) );
    }
    else
    {
        dialog.setTextValue( suggestion );
    }

    dialog.resize( 300, 50 );
    dialog.exec();
    return dialog.result() == QDialog::Accepted ? dialog.textValue() : QString( "" );
}
