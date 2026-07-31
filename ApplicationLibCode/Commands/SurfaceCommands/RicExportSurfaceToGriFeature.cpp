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
#include "RiaQStringFormatter.h"

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
#include <set>

CAF_CMD_SOURCE_INIT( RicExportSurfaceToGriFeature, "RicExportSurfaceToGriFeature" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// Returns the surface if it is a regular surface with a grid matching gridParams exactly, nullptr otherwise
//--------------------------------------------------------------------------------------------------
RimRegularSurface* regularSurfaceWithMatchingGrid( RimSurface* surf, const RigRegularSurfaceData& gridParams )
{
    auto* reg = dynamic_cast<RimRegularSurface*>( surf );
    if ( !reg ) return nullptr;

    if ( reg->nx() == gridParams.nx && reg->ny() == gridParams.ny && reg->originX() == gridParams.originX &&
         reg->originY() == gridParams.originY && reg->incrementX() == gridParams.incrementX && reg->incrementY() == gridParams.incrementY &&
         reg->rotation() == gridParams.rotation )
    {
        return reg;
    }

    return nullptr;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// For a RimRegularSurface whose stored grid matches gridParams exactly, depth values are returned
/// directly. Otherwise the surface is resampled onto the grid via RigSurfaceResampler.
//--------------------------------------------------------------------------------------------------
std::vector<float> RicExportSurfaceToGriFeature::resampleToGrid( RimSurface* surf, const RigRegularSurfaceData& gridParams )
{
    // Regular surface with matching grid: use stored depth values directly (lossless)
    if ( auto* reg = regularSurfaceWithMatchingGrid( surf, gridParams ) )
    {
        return reg->depthValues();
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
std::vector<float>
    RicExportSurfaceToGriFeature::propertyValuesOnGrid( RimSurface* surf, const QString& propertyName, const RigRegularSurfaceData& gridParams )
{
    auto* reg = dynamic_cast<RimRegularSurface*>( surf );
    if ( !reg ) return {};

    // Property values are stored per node on the surface grid and are exported as they are. The export grid must
    // therefore have the same number of nodes as the surface.
    auto values = reg->getProperty( propertyName );
    if ( values.size() != static_cast<size_t>( gridParams.nx * gridParams.ny ) ) return {};

    return values;
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
    ui.setExportFolder( defaultDir );

    RimRegularSurface* regularSurface = ( surfaces.size() == 1 ) ? dynamic_cast<RimRegularSurface*>( surfaces[0] ) : nullptr;

    // Collect the properties available for export from all selected surfaces
    std::set<QString> availableProperties;
    for ( RimSurface* surf : surfaces )
    {
        if ( RigSurface* rig = surf->surfaceData() )
        {
            for ( const auto& name : rig->propertyNames() )
                availableProperties.insert( name );
        }
    }
    ui.setAvailableProperties( { availableProperties.begin(), availableProperties.end() } );

    if ( regularSurface )
    {
        ui.setGridDefaults( regularSurface->nx(),
                            regularSurface->ny(),
                            regularSurface->originX(),
                            regularSurface->originY(),
                            regularSurface->incrementX(),
                            regularSurface->incrementY(),
                            regularSurface->rotation() );
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
        // The estimated grid is a starting point for the user, so round the values to something readable
        ui.setGridDefaults( nx, ny, std::round( bb.min().x() ), std::round( bb.min().y() ), std::round( spacing ), std::round( spacing ), 0.0 );
    }

    caf::PdmUiPropertyViewDialog dialog( nullptr, &ui, "Export Surface to IRAP/GRI", "" );
    dialog.resize( QSize( 400, 450 ) );
    if ( dialog.exec() != QDialog::Accepted ) return;

    const QString exportDir = ui.exportFolder();
    if ( exportDir.isEmpty() ) return;

    app->setLastUsedDialogDirectory( "EXPORT_SURFACE", exportDir );

    const RigRegularSurfaceData gridParams = ui.gridParams();
    const bool                  binary     = ( ui.exportFormat() == RicExportSurfaceToGriUi::ExportFormat::GRI );
    const QString               extension  = binary ? ".gri" : ".irap";

    const auto selectedProperties = ui.selectedProperties();
    if ( selectedProperties.empty() )
    {
        RiaLogging::error( "No values selected for export." );
        return;
    }

    // IRAP/GRI files contain a single value per node, so each exported value is written to a separate file. When only
    // depth values are exported, the file name is the surface name without any suffix.
    const bool useValueNameSuffix = ( selectedProperties.size() > 1 ) ||
                                    ( selectedProperties[0] != RicExportSurfaceToGriUi::depthEntryName() );

    for ( RimSurface* surf : surfaces )
    {
        for ( const QString& valueName : selectedProperties )
        {
            const bool isDepth = ( valueName == RicExportSurfaceToGriUi::depthEntryName() );
            const auto values  = isDepth ? resampleToGrid( surf, gridParams ) : propertyValuesOnGrid( surf, valueName, gridParams );

            if ( values.empty() )
            {
                if ( isDepth )
                {
                    RiaLogging::warning( std::format( "No depth values found for surface '{}', skipping export.", surf->fullName() ) );
                }
                else
                {
                    RiaLogging::warning( std::format( "Skipping export of '{}' for surface '{}'. Properties can only be exported for a "
                                                      "regular surface, and Nx and Ny must match the surface.",
                                                      valueName,
                                                      surf->fullName() ) );
                }
                continue;
            }

            QString baseName = caf::Utils::makeValidFileBasename( surf->fullName() );
            if ( useValueNameSuffix ) baseName += "--" + caf::Utils::makeValidFileBasename( valueName );

            const QString fileName = caf::Utils::constructFullFileName( exportDir, baseName, extension );

            bool ok = binary ? RifSurfio::exportToGri( fileName.toStdString(), gridParams, values )
                             : RifSurfio::exportToIrap( fileName.toStdString(), gridParams, values );

            if ( ok )
                RiaLogging::info( std::format( "Exported surface to: {}", fileName ) );
            else
                RiaLogging::error( std::format( "Failed to export surface to: {}", fileName ) );
        }
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
