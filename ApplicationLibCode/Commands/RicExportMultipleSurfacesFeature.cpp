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

#include "RicExportMultipleSurfacesFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "CommandRouter/RimcExtractSurfaces.h"
#include "RicExportMultipleSurfacesUi.h"
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

CAF_CMD_SOURCE_INIT( RicExportMultipleSurfacesFeature, "RicExportMultipleSurfacesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSurfacesFeature::openDialogAndExecuteCommand()
{
    // Get the list of egrid files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "INPUT_FILES" );

    QString pathFilter( "*" );
    QString fileNameFilter( "*" );

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                "Find ",
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

    RicExportMultipleSurfacesUi* ui = RimProject::current()->dialogData()->generateEnsembleSurfacesUi();
    ui->setLayersMinMax( minLayerK, maxLayerK );

    RiuPropertyViewTabWidget propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                             ui,
                                             "Export Multiple Surfaces",
                                             ui->tabNames() );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        executeCommand( *ui, result.files.toStdList() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSurfacesFeature::executeCommand( const RicExportMultipleSurfacesUi& ui,
                                                       const std::list<QString>&          fileNames )
{
    std::vector layers = ui.layers();

    caf::ProgressInfo progress( fileNames.size(), "Generating ensemble surfaces" );

    QStringList allSurfaceFileNames;
    for ( auto fileName : fileNames )
    {
        auto task                     = progress.task( QString( "Extracting surfaces for %1" ).arg( fileName ) );
        auto [isOk, surfaceFileNames] = RimcCommandRouter_extractSurfaces::extractSurfaces( fileName, layers );
        if ( isOk ) allSurfaceFileNames << surfaceFileNames;
    }

    if ( ui.autoCreateEnsembleSurfaces() )
        RicImportEnsembleSurfaceFeature::importEnsembleSurfaceFromFiles( allSurfaceFileNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportMultipleSurfacesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSurfacesFeature::onActionTriggered( bool isChecked )
{
    openDialogAndExecuteCommand();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSurfacesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Multiple Surfaces..." );
}
