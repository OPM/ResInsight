/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicCreateGridCaseEnsemblesFromFilesFeature.h"

#include "RiaApplication.h"
#include "RiaImportEclipseCaseTools.h"

#include "RicCreateGridCaseGroupFromFilesFeature.h"
#include "RicRecursiveFileSearchDialog.h"

#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseResultCase.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicCreateGridCaseEnsemblesFromFilesFeature, "RicCreateGridCaseEnsemblesFromFilesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseEnsemblesFromFilesFeature::onActionTriggered( bool isChecked )
{
    QString pathCacheName             = "INPUT_FILES";
    auto [fileNames, groupByEnsemble] = runRecursiveFileSearchDialog( "Import Grid Ensembles", pathCacheName );

    if ( groupByEnsemble == RiaEnsembleNameTools::EnsembleGroupingMode::NONE )
    {
        importSingleGridCaseEnsemble( fileNames );
    }
    else
    {
        std::vector<QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByEnsemble( fileNames, groupByEnsemble );
        for ( const QStringList& groupedFileNames : groupedByEnsemble )
        {
            importSingleGridCaseEnsemble( groupedFileNames );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseEnsemblesFromFilesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CreateGridCaseGroup16x16.png" ) );
    actionToSetup->setText( "&Create Grid Case Ensembles" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseEnsemblesFromFilesFeature::importSingleGridCaseEnsemble( const QStringList& fileNames )
{
    auto    eclipseCaseEnsemble = new RimEclipseCaseEnsemble;
    QString ensembleNameSuggestion =
        RiaEnsembleNameTools::findSuitableEnsembleName( fileNames, RiaEnsembleNameTools::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE );
    eclipseCaseEnsemble->setName( ensembleNameSuggestion );

    caf::ProgressInfo progInfo( fileNames.size() + 1, "Creating Grid Ensembles" );

    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return;

    for ( auto caseFileName : fileNames )
    {
        auto task = progInfo.task( "Loading files", 1 );

        QFileInfo gridFileName( caseFileName );

        QString caseName = gridFileName.completeBaseName();

        auto* rimResultReservoir = new RimEclipseResultCase();
        rimResultReservoir->setCaseInfo( caseName, caseFileName );
        eclipseCaseEnsemble->addCase( rimResultReservoir );
    }

    oilfield->analysisModels()->caseEnsembles.push_back( eclipseCaseEnsemble );
    oilfield->analysisModels()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QStringList, RiaEnsembleNameTools::EnsembleGroupingMode>
    RicCreateGridCaseEnsemblesFromFilesFeature::runRecursiveFileSearchDialog( const QString& dialogTitle, const QString& pathCacheName )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( pathCacheName );

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                dialogTitle,
                                                                defaultDir,
                                                                m_pathFilter,
                                                                m_fileNameFilter,
                                                                { RicRecursiveFileSearchDialog::FileType::EGRID } );

    // Remember filters
    m_pathFilter     = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if ( !result.ok ) return std::make_pair( QStringList(), RiaEnsembleNameTools::EnsembleGroupingMode::NONE );

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return std::make_pair( result.files, result.groupingMode );
}
