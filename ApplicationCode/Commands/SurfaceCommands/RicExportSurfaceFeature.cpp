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

#include "RicExportSurfaceFeature.h"

#include "RiaApplication.h"

#include "RigSurface.h"
#include "RimSurface.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicExportSurfaceFeature, "RicExportSurfaceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSurfaceFeature::isCommandEnabled()
{
    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();

    return !surfaces.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceFeature::onActionTriggered( bool isChecked )
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

        writePolygonsToFile( fileName, surf->fullName(), surface->vertices(), surface->triangleIndices() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Export Surface" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSurfaceFeature::writePolygonsToFile( const QString&                 fileName,
                                                   const QString&                 headerText,
                                                   const std::vector<cvf::Vec3d>& vertices,
                                                   const std::vector<unsigned>&   triangleIndices )
{
    QFile exportFile( fileName );
    if ( exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &exportFile );

        QString headerForExport = headerText;
        if ( headerText.isEmpty() ) headerForExport = "surface";

        out << "GOCAD TSurf 1 \n";
        out << "HEADER { \n";
        out << "name:" + headerText + " \n";
        out << "} \n";
        out << "GOCAD_ORIGINAL_COORDINATE_SYSTEM \n";
        out << "NAME Default  \n";
        out << "AXIS_NAME \"X\" \"Y\" \"Z\" \n";
        out << "AXIS_UNIT \"m\" \"m\" \"m\" \n";
        out << "ZPOSITIVE Depth \n";
        out << "END_ORIGINAL_COORDINATE_SYSTEM \n";

        out << "TFACE \n";

        size_t i = 1;
        for ( auto v : vertices )
        {
            out << "VRTX " << i << " ";
            out << v.x() << " ";
            out << v.y() << " ";
            out << -v.z() << " ";
            out << "CNXYZ\n";

            i++;
        }

        for ( size_t triIndex = 0; triIndex < triangleIndices.size(); triIndex += 3 )
        {
            out << "TRGL ";
            out << " " << 1 + triangleIndices[triIndex + 0];
            out << " " << 1 + triangleIndices[triIndex + 1];
            out << " " << 1 + triangleIndices[triIndex + 2];
            out << " \n";
        }

        out << "END\n";

        return true;
    }
    else
    {
        return false;
    }
}
