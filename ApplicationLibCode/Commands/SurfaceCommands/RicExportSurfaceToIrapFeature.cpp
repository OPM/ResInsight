/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RicExportSurfaceToIrapFeature.h"

#include "RiaApplication.h"

#include "RifSurfio.h"

#include "RimSurface.h"

#include "RicExportSurfaceToGriFeature.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicExportSurfaceToIrapFeature, "RicExportSurfaceToIrapFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSurfaceToIrapFeature::isCommandEnabled() const
{
    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();
    return !surfaces.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToIrapFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "EXPORT_SURFACE" );

    QString fileExtensionFilter = QString( "IRAP Classic Surface (*.irap)" );
    QString defaultAbsFileName  = caf::Utils::constructFullFileName( defaultDir, "surface", ".irap" );

    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();
    for ( RimSurface* surf : surfaces )
    {
        QString selectedExtension;
        QString fileName =
            RiuFileDialogTools::getSaveFileName( nullptr, tr( "Export to File" ), defaultAbsFileName, fileExtensionFilter, &selectedExtension );
        if ( fileName.isEmpty() ) return;

        app->setLastUsedDialogDirectory( "EXPORT_SURFACE", QFileInfo( fileName ).absolutePath() );

        auto exportData = RicExportSurfaceToGriFeature::prepareExportData( surf );
        if ( !exportData ) return;

        RifSurfio::exportToIrap( fileName.toStdString(), exportData->first, exportData->second );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToIrapFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Export Surface to IRAP Classic (text) file" );
}
