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
#include "RimWellLogFile.h"

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
bool RicImportEnsembleWellLogsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleWellLogsFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app           = RiaApplication::instance();
    QString         pathCacheName = "ENSEMBLE_WELL_LOGS_FILES";
    QStringList     fileNames     = runRecursiveFileSearchDialog( "Import Ensemble Well Logs", pathCacheName );
    if ( fileNames.isEmpty() ) return;

    createEnsembleWellLogsFromFiles( fileNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogs* RicImportEnsembleWellLogsFeature::createEnsembleWellLogsFromFiles( const QStringList& fileNames )
{
    if ( fileNames.isEmpty() ) return nullptr;

    std::vector<RimWellLogFile*> cases;
    for ( QString fileNames : fileNames )
    {
        QString         errorMessage;
        RimWellLogFile* logFileInfo = RimWellLogFile::readWellLogFile( fileNames, &errorMessage );
        cases.push_back( logFileInfo );
        if ( !errorMessage.isEmpty() )
        {
            RiaLogging::warning( errorMessage );
        }
    }

    if ( cases.empty() ) return nullptr;

    RimEnsembleWellLogs* ensemble = new RimEnsembleWellLogs;

    QString ensembleName = RiaEnsembleNameTools::findSuitableEnsembleName( fileNames );
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
QStringList RicImportEnsembleWellLogsFeature::runRecursiveFileSearchDialog( const QString& dialogTitle,
                                                                            const QString& pathCacheName )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( pathCacheName );

    RicRecursiveFileSearchDialogResult result = RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                                                        dialogTitle,
                                                                                                        defaultDir,
                                                                                                        m_pathFilter,
                                                                                                        m_fileNameFilter,
                                                                                                        QStringList()
                                                                                                            << ".LAS"
                                                                                                            << ".las" );

    // Remember filters
    m_pathFilter     = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if ( !result.ok ) return QStringList();

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return result.files;
}
