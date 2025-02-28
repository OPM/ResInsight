/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicImportEnsembleWellLogsFeature.h"

#include "RiaApplication.h"
#include "RiaEnsembleNameTools.h"
#include "RiaLogging.h"

#include "RimEnsembleWellLogs.h"
#include "RimEnsembleWellLogsCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogLasFile.h"

#include "RicRecursiveFileSearchDialog.h"
#include "WellLogCommands/RicWellLogsImportFileFeature.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportEnsembleWellLogsFeature, "RicImportEnsembleWellLogsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportEnsembleWellLogsFeature::RicImportEnsembleWellLogsFeature()
    : m_pathFilter( "*" )
    , m_fileNameFilter( "*" )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleWellLogsFeature::onActionTriggered( bool isChecked )
{
    QString pathCacheName             = "ENSEMBLE_WELL_LOGS_FILES";
    auto [fileNames, groupByEnsemble] = runRecursiveFileSearchDialog( "Import Ensemble Well Logs", pathCacheName );
    if ( fileNames.isEmpty() ) return;

    createEnsembleWellLogsFromFiles( fileNames, groupByEnsemble );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleWellLogs*>
    RicImportEnsembleWellLogsFeature::createEnsembleWellLogsFromFiles( const QStringList&                         fileNames,
                                                                       RiaDefines::EnsembleGroupingMode groupingMode )
{
    std::vector<RimEnsembleWellLogs*> ensembleWellLogs;

    std::vector<QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByEnsemble( fileNames, groupingMode );
    for ( const QStringList& groupedFileNames : groupedByEnsemble )
    {
        auto ensembleWellLog = createSingleEnsembleWellLogsFromFiles( groupedFileNames, groupingMode );
        ensembleWellLogs.push_back( ensembleWellLog );
    }

    return ensembleWellLogs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogs*
    RicImportEnsembleWellLogsFeature::createSingleEnsembleWellLogsFromFiles( const QStringList&                         fileNames,
                                                                             RiaDefines::EnsembleGroupingMode groupingMode )
{
    if ( fileNames.isEmpty() ) return nullptr;

    std::vector<RimWellLogLasFile*> cases;
    for ( QString fileNames : fileNames )
    {
        QString            errorMessage;
        RimWellLogLasFile* logFileInfo = RimWellLogLasFile::readWellLogFile( fileNames, &errorMessage );
        cases.push_back( logFileInfo );
        if ( !errorMessage.isEmpty() )
        {
            RiaLogging::warning( errorMessage );
        }
    }

    if ( cases.empty() ) return nullptr;

    RimEnsembleWellLogs* ensemble = new RimEnsembleWellLogs;

    QString ensembleName = RiaEnsembleNameTools::findSuitableEnsembleName( fileNames, groupingMode );
    ensemble->setName( ensembleName );
    for ( auto wellLogFile : cases )
        ensemble->addWellLogFile( wellLogFile );

    RimProject::current()->activeOilField()->ensembleWellLogsCollection->addEnsembleWellLogs( ensemble );
    RimProject::current()->activeOilField()->ensembleWellLogsCollection->updateConnectedEditors();

    return ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleWellLogsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/LasFile16x16.png" ) );
    actionToSetup->setText( "Import Ensemble Well Logs" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QStringList, RiaDefines::EnsembleGroupingMode>
    RicImportEnsembleWellLogsFeature::runRecursiveFileSearchDialog( const QString& dialogTitle, const QString& pathCacheName )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( pathCacheName );

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                dialogTitle,
                                                                defaultDir,
                                                                m_pathFilter,
                                                                m_fileNameFilter,
                                                                { RicRecursiveFileSearchDialog::FileType::LAS } );

    // Remember filters
    m_pathFilter     = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if ( !result.ok ) return std::make_pair( QStringList(), RiaDefines::EnsembleGroupingMode::NONE );

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return std::make_pair( result.files, result.groupingMode );
}
