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
#include "RiaLogging.h"

#include "RicCreateGridCaseGroupFromFilesFeature.h"
#include "RicImportFormationNamesFeature.h"
#include "RicNewViewFeature.h"
#include "RicRecursiveFileSearchDialog.h"

#include "RigFormationNames.h"

#include "Formations/RimFormationNames.h"
#include "Formations/RimFormationNamesCollection.h"
#include "Formations/RimFormationTools.h"
#include "Rim3dView.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseResultCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimViewNameConfig.h"

#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicCreateGridCaseEnsemblesFromFilesFeature, "RicCreateGridCaseEnsemblesFromFilesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseEnsemblesFromFilesFeature::onActionTriggered( bool isChecked )
{
    QString pathCacheName             = "INPUT_FILES";
    auto [fileNames, groupByEnsemble] = runRecursiveFileSearchDialog( "Import Grid Ensembles", pathCacheName );

    std::vector<RimEclipseCaseEnsemble*> gridEnsembles;

    if ( groupByEnsemble == RiaDefines::EnsembleGroupingMode::NONE )
    {
        gridEnsembles.push_back( importSingleGridCaseEnsemble( fileNames ) );
    }
    else
    {
        std::vector<QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByEnsemble( fileNames, groupByEnsemble );
        for ( const QStringList& groupedFileNames : groupedByEnsemble )
        {
            gridEnsembles.push_back( importSingleGridCaseEnsemble( groupedFileNames ) );
        }
    }

    if ( gridEnsembles.empty() ) return;

    auto firstEnsemble = gridEnsembles.front();
    if ( firstEnsemble->cases().empty() ) return;

    auto firstCase = firstEnsemble->cases().front();
    if ( !firstCase ) return;

    auto view = RicNewViewFeature::addReservoirView( firstCase, nullptr, firstEnsemble->viewCollection() );
    if ( view )
    {
        // Show the case name in the view title, as this is useful information for a grid case ensemble
        view->nameConfig()->setAddCaseName( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseEnsemblesFromFilesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CreateGridCaseGroup16x16.png" ) );
    actionToSetup->setText( "&Create Grid Case Ensemble" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCaseEnsemble* RicCreateGridCaseEnsemblesFromFilesFeature::importSingleGridCaseEnsemble( const QStringList& fileNames )
{
    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return nullptr;

    auto    eclipseCaseEnsemble = new RimEclipseCaseEnsemble;
    QString ensembleNameSuggestion =
        RiaEnsembleNameTools::findSuitableEnsembleName( fileNames, RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE );
    eclipseCaseEnsemble->setName( ensembleNameSuggestion );

    caf::ProgressInfo progInfo( fileNames.size() + 1, "Creating Grid Ensembles" );

    for ( const auto& caseFileName : fileNames )
    {
        auto task = progInfo.task( "Loading files", 1 );

        auto* rimResultCase = importSingleGridCase( caseFileName );
        eclipseCaseEnsemble->addCase( rimResultCase );
    }

    for ( auto gridCase : eclipseCaseEnsemble->cases() )
    {
        gridCase->updateAutoShortName();
    }

    oilfield->analysisModels()->caseEnsembles.push_back( eclipseCaseEnsemble );
    oilfield->analysisModels()->updateConnectedEditors();

    return eclipseCaseEnsemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicCreateGridCaseEnsemblesFromFilesFeature::importSingleGridCase( const QString& fileName )
{
    QFileInfo gridFileName( fileName );

    QString caseName = gridFileName.completeBaseName();

    auto* rimResultCase = new RimEclipseResultCase();
    rimResultCase->setDisplayNameType( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME );
    rimResultCase->setCaseInfo( caseName, fileName );

    auto               folderNames = RimFormationTools::formationFoldersFromCaseFileName( fileName );
    RimFormationNames* formations  = RimFormationTools::loadFormationNamesFromFolder( folderNames );
    if ( formations != nullptr ) rimResultCase->setFormationNames( formations );

    return rimResultCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QStringList, RiaDefines::EnsembleGroupingMode>
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

    if ( !result.ok ) return std::make_pair( QStringList(), RiaDefines::EnsembleGroupingMode::NONE );

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return std::make_pair( result.files, result.groupingMode );
}
