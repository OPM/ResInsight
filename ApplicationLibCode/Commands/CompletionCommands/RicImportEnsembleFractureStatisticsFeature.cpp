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

#include "RicImportEnsembleFractureStatisticsFeature.h"

#include "RiaEnsembleNameTools.h"
#include "RiaGuiApplication.h"

#include "RicRecursiveFileSearchDialog.h"

#include "RimCompletionTemplateCollection.h"
#include "RimEnsembleFractureStatistics.h"
#include "RimEnsembleFractureStatisticsCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "cafProgressInfo.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportEnsembleFractureStatisticsFeature, "RicImportEnsembleFractureStatisticsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportEnsembleFractureStatisticsFeature::m_pathFilter     = "*";
QString RicImportEnsembleFractureStatisticsFeature::m_fileNameFilter = "*";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportEnsembleFractureStatisticsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFractureStatisticsFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app           = RiaGuiApplication::instance();
    QString            pathCacheName = "INPUT_FILES";
    QStringList        fileNames     = runRecursiveFileSearchDialog( "Import StimPlan Fractures", pathCacheName );

    std::vector<QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByEnsemble( fileNames );
    for ( const QStringList& groupedFileNames : groupedByEnsemble )
    {
        importSingleEnsembleFractureStatistics( groupedFileNames );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFractureStatisticsFeature::importSingleEnsembleFractureStatistics( const QStringList& fileNames )
{
    auto    fractureGroupStatistics = new RimEnsembleFractureStatistics;
    QString ensembleNameSuggestion  = RiaEnsembleNameTools::findSuitableEnsembleName( fileNames );
    fractureGroupStatistics->setName( ensembleNameSuggestion );

    caf::ProgressInfo progInfo( fileNames.size() + 1, "Creating Ensemble Fracture Statistics" );

    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return;

    RimCompletionTemplateCollection* completionTemplateCollection = oilfield->completionTemplateCollection();
    if ( !completionTemplateCollection ) return;

    RimEnsembleFractureStatisticsCollection* fractureGroupStatisticsCollection =
        completionTemplateCollection->fractureGroupStatisticsCollection();
    if ( !fractureGroupStatisticsCollection ) return;

    for ( auto f : fileNames )
    {
        auto task = progInfo.task( "Loading files", 1 );
        fractureGroupStatistics->addFilePath( f );
    }

    {
        auto task = progInfo.task( "Generating statistics", 1 );
        fractureGroupStatistics->loadAndUpdateData();
    }

    fractureGroupStatisticsCollection->addFractureGroupStatistics( fractureGroupStatistics );

    fractureGroupStatisticsCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFractureStatisticsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import StimPlan Fractures Recursively" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportEnsembleFractureStatisticsFeature::runRecursiveFileSearchDialog( const QString& dialogTitle,
                                                                                      const QString& pathCacheName )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( pathCacheName );

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                dialogTitle,
                                                                defaultDir,
                                                                m_pathFilter,
                                                                m_fileNameFilter,
                                                                QStringList( ".xml" ) );

    // Remember filters
    m_pathFilter     = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if ( !result.ok ) return QStringList();

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return result.files;
}
