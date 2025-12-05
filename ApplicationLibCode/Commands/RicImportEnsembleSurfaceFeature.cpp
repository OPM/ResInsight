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
#include "RiaEnsembleNameTools.h"
#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "RicImportEnsembleFeature.h"
#include "RicRecursiveFileSearchDialog.h"

#include "RimEnsembleSurface.h"
#include "RimFileSurface.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

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
void RicImportEnsembleSurfaceFeature::onActionTriggered( bool isChecked )
{
    QString pathCacheName             = "ENSEMBLE_SURFACE_FILES";
    auto [fileNames, groupByEnsemble] = runRecursiveFileSearchDialog( "Import Ensemble Surface", pathCacheName );

    importEnsembleSurfaceFromFiles( fileNames, groupByEnsemble );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleSurfaceFeature::importEnsembleSurfaceFromFiles( const QStringList&               fileNames,
                                                                      RiaDefines::EnsembleGroupingMode groupingMode )
{
    if ( fileNames.isEmpty() ) return;

    std::vector<QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByEnsemble( fileNames, groupingMode );
    for ( const QStringList& groupedFileNames : groupedByEnsemble )
    {
        importSingleEnsembleSurfaceFromFiles( groupedFileNames, groupingMode );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleSurfaceFeature::importSingleEnsembleSurfaceFromFiles( const QStringList&               fileNames,
                                                                            RiaDefines::EnsembleGroupingMode groupingMode )
{
    // Create a list of file names for each layer
    std::map<QString, QStringList> fileNamesForEachLayer;
    for ( const auto& fileName : fileNames )
    {
        QFileInfo fi( fileName );

        auto layerName = fi.baseName();
        fileNamesForEachLayer[layerName].push_back( fileName );
    }

    RimSurfaceCollection* surfColl = RimTools::surfaceCollection();

    RimEnsembleSurface* ensembleToSelect = nullptr;
    for ( const auto& fileNamesForLayer : fileNamesForEachLayer )
    {
        // NB! This must be a const reference to avoid threading issues
        const QStringList& layerFileNames = fileNamesForLayer.second;

        QString ensembleName = RiaEnsembleNameTools::findSuitableEnsembleName( layerFileNames, groupingMode );
        QString layerName    = fileNamesForLayer.first;
        if ( !layerName.isEmpty() )
        {
            ensembleName += QString( " : %1" ).arg( layerName );
        }

        if ( ensembleName.isEmpty() ) ensembleName = "Ensemble Surface";

        std::map<QString, QStringList> keyFileComponentsForAllFiles = RiaFilePathTools::keyPathComponentsForEachFilePath( layerFileNames );

        std::vector<RimSurface*> surfaces;
        for ( int i = 0; i < layerFileNames.size(); i++ )
        {
            surfaces.push_back( RimSurfaceCollection::createSurfaceFromFile( layerFileNames[i] ) );
        }

        auto fileCount = static_cast<int>( layerFileNames.size() );
#pragma omp parallel for
        for ( int i = 0; i < fileCount; i++ )
        {
            auto fileName = layerFileNames[i];

            auto surface = surfaces[i];

            auto shortName = RiaEnsembleNameTools::uniqueShortNameFromComponents( fileName, keyFileComponentsForAllFiles, ensembleName );
            surface->setUserDescription( shortName );

            auto isOk = surface->onLoadData();
            if ( !isOk )
            {
                delete surface;
                surfaces[i] = nullptr;
            }
        }

        {
            // Remove null pointers from vector of surfaces
            std::vector<RimSurface*> tmp;
            for ( auto s : surfaces )
            {
                if ( s != nullptr ) tmp.push_back( s );
            }

            surfaces.swap( tmp );
        }

        if ( surfaces.empty() ) return;

        auto ensemble = new RimEnsembleSurface;
        ensemble->setCollectionName( ensembleName );
        for ( auto surface : surfaces )
            ensemble->addSurface( surface );

        ensemble->loadDataAndUpdate();

        surfColl->addEnsembleSurface( ensemble );

        ensembleToSelect = ensemble;
    }

    surfColl->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( ensembleToSelect );
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
std::pair<QStringList, RiaDefines::EnsembleGroupingMode>
    RicImportEnsembleSurfaceFeature::runRecursiveFileSearchDialog( const QString& dialogTitle, const QString& pathCacheName )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( pathCacheName );

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                dialogTitle,
                                                                defaultDir,
                                                                m_pathFilter,
                                                                m_fileNameFilter,
                                                                { RicRecursiveFileSearchDialog::FileType::SURFACE } );

    // Remember filters
    m_pathFilter     = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if ( !result.ok ) return std::pair( QStringList(), RiaDefines::EnsembleGroupingMode::NONE );

    // Remember the path to next time
    app->setLastUsedDialogDirectory( pathCacheName, QFileInfo( result.rootDir ).absoluteFilePath() );

    return std::pair( result.files, result.groupingMode );
}
