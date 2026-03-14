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

#include "RicExportSurfaceToGriFeature.h"

#include "RiaApplication.h"

#include "RifSurfio.h"

#include "Surface/RigSurface.h"
#include "Surface/RigSurfaceResampler.h"

#include "RimRegularSurface.h"
#include "RimSurface.h"

#include "RicExportSurfaceToGriDialog.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include "cvfBoundingBox.h"

#include <QAction>
#include <QFileInfo>
#include <cmath>

CAF_CMD_SOURCE_INIT( RicExportSurfaceToGriFeature, "RicExportSurfaceToGriFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<RigRegularSurfaceData, std::vector<float>>> RicExportSurfaceToGriFeature::prepareExportData( RimSurface* surf )
{
    if ( auto* regularSurface = dynamic_cast<RimRegularSurface*>( surf ) )
    {
        // RimRegularSurface and RimRegularFileSurface: use stored grid parameters directly
        RigRegularSurfaceData gridParams;
        gridParams.nx         = regularSurface->nx();
        gridParams.ny         = regularSurface->ny();
        gridParams.originX    = regularSurface->originX();
        gridParams.originY    = regularSurface->originY();
        gridParams.incrementX = regularSurface->incrementX();
        gridParams.incrementY = regularSurface->incrementY();
        gridParams.rotation   = regularSurface->rotation();
        return std::make_pair( gridParams, regularSurface->depthValues() );
    }

    // Unstructured surface: determine grid parameters from bounding box and resample
    RigSurface* rigSurface = surf->surfaceData();
    if ( !rigSurface || rigSurface->vertices().empty() ) return std::nullopt;

    cvf::BoundingBox bb;
    for ( const auto& v : rigSurface->vertices() )
        bb.add( v );

    const double incX = rigSurface->maxExtentTriangleInXDirection();
    const double incY = rigSurface->maxExtentTriangleInYDirection();

    RicGriExportGridParams defaults;
    defaults.originX    = bb.min().x();
    defaults.originY    = bb.min().y();
    defaults.incrementX = incX > 0.0 ? incX : 1.0;
    defaults.incrementY = incY > 0.0 ? incY : 1.0;
    defaults.nx         = std::max( 2, static_cast<int>( std::ceil( bb.extent().x() / defaults.incrementX ) ) + 1 );
    defaults.ny         = std::max( 2, static_cast<int>( std::ceil( bb.extent().y() / defaults.incrementY ) ) + 1 );

    auto params = RicExportSurfaceToGriDialog::openDialog( nullptr, defaults );
    if ( !params.accepted ) return std::nullopt;

    RigRegularSurfaceData gridParams;
    gridParams.nx         = params.nx;
    gridParams.ny         = params.ny;
    gridParams.originX    = params.originX;
    gridParams.originY    = params.originY;
    gridParams.incrementX = params.incrementX;
    gridParams.incrementY = params.incrementY;
    gridParams.rotation   = 0.0;

    auto depthValues = RigSurfaceResampler::resampleToRegularGrid( rigSurface,
                                                                   params.nx,
                                                                   params.ny,
                                                                   params.originX,
                                                                   params.originY,
                                                                   params.incrementX,
                                                                   params.incrementY,
                                                                   0.0 );

    // RigSurface stores Z as negative depth; IRAP format uses positive depth values
    for ( auto& v : depthValues )
    {
        if ( !std::isnan( v ) ) v = -v;
    }

    return std::make_pair( gridParams, depthValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSurfaceToGriFeature::isCommandEnabled() const
{
    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();
    return !surfaces.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "EXPORT_SURFACE" );

    QString fileExtensionFilter = QString( "IRAP Binary Surface (*.gri)" );
    QString defaultAbsFileName  = caf::Utils::constructFullFileName( defaultDir, "surface", ".gri" );

    std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();
    for ( RimSurface* surf : surfaces )
    {
        QString selectedExtension;
        QString fileName =
            RiuFileDialogTools::getSaveFileName( nullptr, tr( "Export to File" ), defaultAbsFileName, fileExtensionFilter, &selectedExtension );
        if ( fileName.isEmpty() ) return;

        app->setLastUsedDialogDirectory( "EXPORT_SURFACE", QFileInfo( fileName ).absolutePath() );

        auto exportData = prepareExportData( surf );
        if ( !exportData ) return;

        RifSurfio::exportToGri( fileName.toStdString(), exportData->first, exportData->second );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Export Surface to IRAP Binary (GRI) file" );
}
