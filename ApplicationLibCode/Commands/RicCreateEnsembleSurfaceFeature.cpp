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

#include "RicCreateEnsembleSurfaceFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "CommandRouter/RimcExtractSurfaces.h"
#include "RicCreateEnsembleSurfaceUi.h"
#include "RicImportEnsembleSurfaceFeature.h"
#include "RicRecursiveFileSearchDialog.h"
#include "RimDialogData.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "RiuPropertyViewTabWidget.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmSettings.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicCreateEnsembleSurfaceFeature, "RicCreateEnsembleSurfaceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleSurfaceFeature::openDialogAndExecuteCommand()
{
    // Get the list of egrid files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );

    QString pathFilter( "*" );
    QString fileNameFilter( "*" );

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                "Choose Eclipse Cases",
                                                                defaultDir,
                                                                pathFilter,
                                                                fileNameFilter,
                                                                QStringList( ".EGRID" ) );

    if ( !result.ok || result.files.isEmpty() )
    {
        return;
    }

    // Read min/max k layer from first grid case
    int minLayerK = -1;
    int maxLayerK = -1;
    if ( !RimcCommandRouter_extractSurfaces::readMinMaxLayerFromGridFile( result.files[0], minLayerK, maxLayerK ) )
        return;

    RicCreateEnsembleSurfaceUi* ui = RimProject::current()->dialogData()->createEnsembleSurfaceUi();
    ui->setLayersMinMax( minLayerK, maxLayerK );

    RiuPropertyViewTabWidget propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                             ui,
                                             "Create Ensemble Surface",
                                             ui->tabNames() );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        executeCommand( *ui, result.files.toVector().toStdVector() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleSurfaceFeature::executeCommand( const RicCreateEnsembleSurfaceUi& ui,
                                                      const std::vector<QString>&       fileNames )
{
    std::vector layers = ui.layers();

    caf::ProgressInfo progress( fileNames.size(), "Generating ensemble surface" );

    QStringList allSurfaceFileNames;

    auto fileCount = static_cast<int>( fileNames.size() );
#pragma omp parallel for
    for ( int i = 0; i < fileCount; i++ )
    {
        auto fileName = fileNames[i];

        // Not possible to use structured bindings here due to a bug in clang
        auto surfaceResult    = RimcCommandRouter_extractSurfaces::extractSurfaces( fileName, layers );
        auto isOk             = surfaceResult.first;
        auto surfaceFileNames = surfaceResult.second;

#pragma omp critical( RicCreateEnsembleSurfaceFeature )
        {
            auto task = progress.task( QString( "Extracting surfaces for %1" ).arg( fileName ) );
            if ( isOk ) allSurfaceFileNames << surfaceFileNames;
        }
    }
    progress.setProgress( fileNames.size() );

    if ( ui.autoCreateEnsembleSurfaces() )
        RicImportEnsembleSurfaceFeature::importEnsembleSurfaceFromFiles( allSurfaceFileNames,
                                                                         RiaEnsembleNameTools::EnsembleGroupingMode::FMU_FOLDER_MODE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateEnsembleSurfaceFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleSurfaceFeature::onActionTriggered( bool isChecked )
{
    openDialogAndExecuteCommand();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleSurfaceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Ensemble Surface..." );
}
