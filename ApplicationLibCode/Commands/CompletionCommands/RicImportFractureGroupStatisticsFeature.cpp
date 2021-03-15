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

#include "RicImportFractureGroupStatisticsFeature.h"

#include "RiaGuiApplication.h"

#include "RicRecursiveFileSearchDialog.h"

#include "RimCompletionTemplateCollection.h"
#include "RimFractureGroupStatistics.h"
#include "RimFractureGroupStatisticsCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportFractureGroupStatisticsFeature, "RicImportFractureGroupStatisticsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportFractureGroupStatisticsFeature::m_pathFilter     = "*";
QString RicImportFractureGroupStatisticsFeature::m_fileNameFilter = "*";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFractureGroupStatisticsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFractureGroupStatisticsFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app           = RiaGuiApplication::instance();
    QString            pathCacheName = "INPUT_FILES";
    QStringList        fileNames     = runRecursiveFileSearchDialog( "Import Summary Cases", pathCacheName );

    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return;

    RimCompletionTemplateCollection* completionTemplateCollection = oilfield->completionTemplateCollection();
    if ( !completionTemplateCollection ) return;

    RimFractureGroupStatisticsCollection* fractureGroupStatisticsCollection =
        completionTemplateCollection->fractureGroupStatisticsCollection();
    if ( !fractureGroupStatisticsCollection ) return;

    auto fractureGroupStatistics = new RimFractureGroupStatistics;
    fractureGroupStatistics->setName( "Imported Fracture Group Statistics" );

    for ( auto f : fileNames )
    {
        fractureGroupStatistics->addFilePath( f );
    }

    fractureGroupStatisticsCollection->addFractureGroupStatistics( fractureGroupStatistics );

    fractureGroupStatisticsCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFractureGroupStatisticsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import StimPlan Fractures Recursively" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportFractureGroupStatisticsFeature::runRecursiveFileSearchDialog( const QString& dialogTitle,
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
