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

#include "RiaApplication.h"
#include "RiaEnsembleNameTools.h"
#include "RiaFilePathTools.h"
#include "RiaPreferences.h"
#include "RiaSummaryDefines.h"
#include "RiaSummaryTools.h"
#include "RiaTextStringTools.h"

#include "RicCreateSummaryCaseCollectionFeature.h"
#include "RicImportSummaryCasesFeature.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "SummaryPlotCommands/RicNewSummaryEnsembleCurveSetFeature.h"

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QInputDialog>
#include <QRegularExpression>
#include <QStringList>

#include <set>

CAF_CMD_SOURCE_INIT( RicImportEnsembleFeature, "RicImportEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::onActionTriggered( bool isChecked )
{
    QString pathCacheName = "ENSEMBLE_FILES";
    auto    result = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialogWithGrouping( "Import Ensemble", pathCacheName );
    QStringList                                fileNames            = result.files;
    RiaEnsembleNameTools::EnsembleGroupingMode ensembleGroupingMode = result.groupingMode;
    RiaDefines::FileType                       fileType             = RicRecursiveFileSearchDialog::mapSummaryFileType( result.fileType );

    if ( fileNames.isEmpty() ) return;

    if ( ensembleGroupingMode == RiaEnsembleNameTools::EnsembleGroupingMode::NONE )
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
            std::vector<QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByEnsemble( fileNames, ensembleGroupingMode );
            for ( const QStringList& groupedFileNames : groupedByEnsemble )
            {
                bool useEnsembleNameDialog = false;
                importSingleEnsemble( groupedFileNames, useEnsembleNameDialog, ensembleGroupingMode, fileType );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::importSingleEnsemble( const QStringList&                         fileNames,
                                                     bool                                       useEnsembleNameDialog,
                                                     RiaEnsembleNameTools::EnsembleGroupingMode groupingMode,
                                                     RiaDefines::FileType                       fileType,
                                                     const QString&                             defaultEnsembleName )
{
    QString ensembleName = !defaultEnsembleName.isEmpty() ? defaultEnsembleName
                                                          : RiaEnsembleNameTools::findSuitableEnsembleName( fileNames, groupingMode );

    if ( useEnsembleNameDialog ) ensembleName = askForEnsembleName( ensembleName );

    if ( ensembleName.isEmpty() ) return;

    RicImportSummaryCasesFeature::CreateConfig createConfig{ .fileType = fileType, .ensembleOrGroup = true, .allowDialogs = true };
    auto [isOk, cases] = RicImportSummaryCasesFeature::createSummaryCasesFromFiles( fileNames, createConfig );

    if ( !isOk || cases.empty() ) return;

    RimSummaryEnsemble* ensemble = RicCreateSummaryCaseCollectionFeature::groupSummaryCases( cases, ensembleName, true );

    if ( ensemble )
    {
        for ( auto summaryCase : ensemble->allSummaryCases() )
        {
            summaryCase->updateAutoShortName();
        }

        RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( {}, { ensemble } );
    }

    std::vector<RimCase*> allCases = RiaApplication::instance()->project()->allGridCases();
    if ( allCases.empty() )
    {
        RiuMainWindow::closeIfOpen();
    }
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
    std::vector<RimSummaryEnsemble*> groups                    = project->summaryGroups();
    int                              ensemblesStartingWithRoot = std::count_if( groups.begin(),
                                                   groups.end(),
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
