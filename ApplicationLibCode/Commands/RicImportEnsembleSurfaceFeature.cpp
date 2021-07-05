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

#include "RicImportEnsembleSurfaceFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimEnsembleSurface.h"
#include "RimFileSurface.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurfaceCollection.h"

#include "RicRecursiveFileSearchDialog.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportEnsembleSurfaceFeature, "RicImportEnsembleSurfaceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportEnsembleSurfaceFeature::RicImportEnsembleSurfaceFeature()
    : m_pathFilter( "*" )
    , m_fileNameFilter( "*" )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportEnsembleSurfaceFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleSurfaceFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app           = RiaApplication::instance();
    QString         pathCacheName = "ENSEMBLE_SURFACE_FILES";
    QStringList     fileNames     = runRecursiveFileSearchDialog( "Import Ensemble Surface", pathCacheName );
    if ( fileNames.isEmpty() ) return;

    QString ensembleName = "Ensemble Surface";
    if ( ensembleName.isEmpty() ) return;

    std::vector<RimFileSurface*> surfaces;
    for ( QString fileName : fileNames )
    {
        RimFileSurface* fileSurface = new RimFileSurface;
        fileSurface->setSurfaceFilePath( fileName );

        if ( fileSurface->onLoadData() )
        {
            surfaces.push_back( fileSurface );
        }
    }

    if ( surfaces.empty() ) return;

    RimEnsembleSurface* ensemble = new RimEnsembleSurface;
    ensemble->setName( ensembleName );
    for ( auto surface : surfaces )
        ensemble->addFileSurface( surface );

    RimProject::current()->activeOilField()->surfaceCollection->addEnsembleSurface( ensemble );
    RimProject::current()->activeOilField()->surfaceCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleSurfaceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Import Ensemble Surface" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportEnsembleSurfaceFeature::runRecursiveFileSearchDialog( const QString& dialogTitle,
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
                                                                                                            << ".TS"
                                                                                                            << ".ts" );

    // Remember filters
    m_pathFilter     = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if ( !result.ok ) return QStringList();

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return result.files;
}
