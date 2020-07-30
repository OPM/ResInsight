/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicExportSurfaceToTsurfFeature.h"

#include "RiaApplication.h"

#include "RifSurfaceExporter.h"
#include "RigSurface.h"
#include "RimSurface.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicExportSurfaceToTsurfFeature, "RicExportSurfaceToTsurfFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSurfaceToTsurfFeature::isCommandEnabled()
{
    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();

    return !surfaces.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToTsurfFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "EXPORT_SURFACE" );

    QStringList imageFileExtensions;
    imageFileExtensions << "*.ts";
    QString fileExtensionFilter = QString( "Surface (%1)" ).arg( imageFileExtensions.join( " " ) );

    QString defaultExtension    = "ts";
    QString defaultFileBaseName = "surface";

    QString defaultAbsFileName =
        caf::Utils::constructFullFileName( defaultDir, defaultFileBaseName, "." + defaultExtension );

    QString selectedExtension;

    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();
    for ( RimSurface* surf : surfaces )
    {
        QString fileName = QFileDialog::getSaveFileName( nullptr,
                                                         tr( "Export to File" ),
                                                         defaultAbsFileName,
                                                         fileExtensionFilter,
                                                         &selectedExtension );
        if ( fileName.isEmpty() ) return;

        app->setLastUsedDialogDirectory( "EXPORT_SURFACE", QFileInfo( fileName ).absolutePath() );

        RigSurface* surface = surf->surfaceData();

        RifSurfaceExporter::writeGocadTSurfFile( fileName,
                                                 surf->userDescription(),
                                                 surface->vertices(),
                                                 surface->triangleIndices() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToTsurfFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Export Surface to GOCAD TSurf file" );
}
