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

#include "RicExportKLayerToPtlFeature.h"

#include "RiaApplication.h"

#include "RifSurfaceExporter.h"
#include "RigSurface.h"
#include "RimGridCaseSurface.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicExportKLayerToPtlFeature, "RicExportKLayerToPtlFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportKLayerToPtlFeature::isCommandEnabled()
{
    // std::vector<RimGridCaseSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimGridCaseSurface*>();

    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();

    return !surfaces.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportKLayerToPtlFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "EXPORT_SURFACE" );

    QStringList imageFileExtensions;
    imageFileExtensions << "*.ptl";
    QString fileExtensionFilter = QString( "Surface (%1)" ).arg( imageFileExtensions.join( " " ) );

    QString defaultExtension    = "ptl";
    QString defaultFileBaseName = "surface";

    QString defaultAbsFileName =
        caf::Utils::constructFullFileName( defaultDir, defaultFileBaseName, "." + defaultExtension );

    QString selectedExtension;

    std::vector<RimGridCaseSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimGridCaseSurface*>();

    for ( RimGridCaseSurface* surf : surfaces )
    {
        QString fileName = QFileDialog::getSaveFileName( nullptr,
                                                         tr( "Export to File" ),
                                                         defaultAbsFileName,
                                                         fileExtensionFilter,
                                                         &selectedExtension );
        if ( fileName.isEmpty() ) return;

        app->setLastUsedDialogDirectory( "EXPORT_SURFACE", QFileInfo( fileName ).absolutePath() );

        std::vector<cvf::Vec3d>            vertices;
        std::vector<std::pair<uint, uint>> structGridVertexIndices;

        if ( surf->exportStructSurfaceFromGridCase( &vertices, &structGridVertexIndices ) )
        {
            RifSurfaceExporter::writePetrelPtlFile( fileName, vertices, structGridVertexIndices );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportKLayerToPtlFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Export Grid Case Surface to Petrel ptl-file" );
}
