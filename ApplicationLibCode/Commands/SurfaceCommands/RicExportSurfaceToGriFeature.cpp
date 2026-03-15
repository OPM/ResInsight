/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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
#include "RiaLogging.h"

#include "RifSurfio.h"

#include "Surface/RigSurface.h"
#include "Surface/RigSurfaceResampler.h"

#include "RimRegularSurface.h"
#include "RimSurface.h"

#include "RicExportSurfaceToGriUi.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include "cvfBoundingBox.h"

#include <QAction>
#include <cmath>

CAF_CMD_SOURCE_INIT( RicExportSurfaceToGriFeature, "RicExportSurfaceToGriFeature" );

//--------------------------------------------------------------------------------------------------
/// For a RimRegularSurface whose stored grid matches gridParams exactly, depth values are returned
/// directly. Otherwise the surface is resampled onto the grid via RigSurfaceResampler.
//--------------------------------------------------------------------------------------------------
std::vector<float> RicExportSurfaceToGriFeature::resampleToGrid( RimSurface* surf, const RigRegularSurfaceData& gridParams )
{
    // Regular surface with matching grid: use stored depth values directly (lossless)
    if ( auto* reg = dynamic_cast<RimRegularSurface*>( surf ) )
    {
        if ( reg->nx() == gridParams.nx && reg->ny() == gridParams.ny && reg->originX() == gridParams.originX &&
             reg->originY() == gridParams.originY && reg->incrementX() == gridParams.incrementX &&
             reg->incrementY() == gridParams.incrementY && reg->rotation() == gridParams.rotation )
        {
            return reg->depthValues();
        }
    }

    // Resample the surface onto the requested grid
    RigSurface* rig = surf->surfaceData();
    if ( !rig || rig->vertices().empty() ) return {};

    auto depthValues = RigSurfaceResampler::resampleToRegularGrid( rig,
                                                                   gridParams.nx,
                                                                   gridParams.ny,
                                                                   gridParams.originX,
                                                                   gridParams.originY,
                                                                   gridParams.incrementX,
                                                                   gridParams.incrementY,
                                                                   gridParams.rotation );

    // RigSurface stores Z as negative depth; IRAP format uses positive depth values
    for ( auto& v : depthValues )
        if ( !std::isnan( v ) ) v = -v;

    return depthValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriFeature::exportSurfaces( const std::vector<RimSurface*>& surfaces )
{
    if ( surfaces.empty() ) return;

    RiaApplication* app        = RiaApplication::instance();
    const QString   defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "EXPORT_SURFACE" );

    // Build default grid params
    RicExportSurfaceToGriUi ui;

    if ( surfaces.size() == 1 )
    {
        if ( auto* reg = dynamic_cast<RimRegularSurface*>( surfaces[0] ) )
        {
            ui.setDefaults( defaultDir, reg->nx(), reg->ny(), reg->originX(), reg->originY(), reg->incrementX(), reg->incrementY() );
        }
    }
    else
    {
        cvf::BoundingBox bb;
        size_t           totalVertexCount = 0;
        for ( RimSurface* surf : surfaces )
        {
            RigSurface* rig = surf->surfaceData();
            if ( !rig ) continue;
            for ( const auto& v : rig->vertices() )
                bb.add( v );
            totalVertexCount += rig->vertices().size();
        }

        if ( !bb.isValid() || totalVertexCount == 0 ) return;

        const double areaApprox = bb.extent().x() * bb.extent().y();
        const double spacing    = ( areaApprox > 0.0 ) ? std::sqrt( areaApprox / static_cast<double>( totalVertexCount ) ) : 1.0;
        const int    nx         = std::max( 2, static_cast<int>( std::ceil( bb.extent().x() / spacing ) ) + 1 );
        const int    ny         = std::max( 2, static_cast<int>( std::ceil( bb.extent().y() / spacing ) ) + 1 );
        ui.setDefaults( defaultDir, nx, ny, bb.min().x(), bb.min().y(), spacing, spacing );
    }

    caf::PdmUiPropertyViewDialog dialog( nullptr, &ui, "Export Surface to IRAP/GRI", "" );
    dialog.resize( QSize( 400, 300 ) );
    if ( dialog.exec() != QDialog::Accepted ) return;

    const QString exportDir = ui.exportFolder();
    if ( exportDir.isEmpty() ) return;

    app->setLastUsedDialogDirectory( "EXPORT_SURFACE", exportDir );

    const RigRegularSurfaceData gridParams = ui.gridParams();
    const bool                  binary     = ( ui.exportFormat() == RicExportSurfaceToGriUi::ExportFormat::GRI );
    const QString               extension  = binary ? ".gri" : ".irap";

    for ( RimSurface* surf : surfaces )
    {
        const QString fileName =
            caf::Utils::constructFullFileName( exportDir, caf::Utils::makeValidFileBasename( surf->fullName() ), extension );

        const auto depthValues = resampleToGrid( surf, gridParams );
        if ( depthValues.empty() ) continue;

        bool ok = binary ? RifSurfio::exportToGri( fileName.toStdString(), gridParams, depthValues )
                         : RifSurfio::exportToIrap( fileName.toStdString(), gridParams, depthValues );

        if ( ok )
            RiaLogging::info( QString( "Exported surface to: %1" ).arg( fileName ) );
        else
            RiaLogging::error( QString( "Failed to export surface to: %1" ).arg( fileName ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSurfaceToGriFeature::isCommandEnabled() const
{
    return !caf::selectedObjectsByTypeStrict<RimSurface*>().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriFeature::onActionTriggered( bool isChecked )
{
    exportSurfaces( caf::selectedObjectsByTypeStrict<RimSurface*>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Export Surface to IRAP/GRI..." );
}
