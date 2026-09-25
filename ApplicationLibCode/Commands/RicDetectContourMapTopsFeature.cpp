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

#include "RicDetectContourMapTopsFeature.h"

#include "RicExportContourMapToTextFeature.h"

#include "ContourMap/RigContourMapProjection.h"
#include "ContourMap/RigContourMapTopFinder.h"

#include "ContourMap/RimContourMapProjection.h"
#include "ContourMap/RimContourMapTopsCollection.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "RimGeoMechContourMapView.h"

#include "RiuMainWindow.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>

#include <algorithm>

CAF_CMD_SOURCE_INIT( RicDetectContourMapTopsFeature, "RicDetectContourMapTopsFeature" );

//--------------------------------------------------------------------------------------------------
/// Enabled for all contour map view types (single-realization and ensemble statistics).
//--------------------------------------------------------------------------------------------------
bool RicDetectContourMapTopsFeature::isCommandEnabled() const
{
    auto [existingEclipseContourMap, existingGeoMechContourMap] = RicExportContourMapToTextFeature::findContourMapView();

    return existingEclipseContourMap || existingGeoMechContourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDetectContourMapTopsFeature::onActionTriggered( bool isChecked )
{
    auto [existingEclipseContourMap, existingGeoMechContourMap] = RicExportContourMapToTextFeature::findContourMapView();

    RimContourMapProjection* contourMapProjection = nullptr;
    if ( existingEclipseContourMap ) contourMapProjection = existingEclipseContourMap->contourMapProjection();
    if ( existingGeoMechContourMap ) contourMapProjection = existingGeoMechContourMap->contourMapProjection();
    if ( !contourMapProjection ) return;

    auto rigContourMapProjection = contourMapProjection->mapProjection();
    if ( !rigContourMapProjection ) return;

    bool ok = false;
    int  maxTops =
        QInputDialog::getInt( RiuMainWindow::instance(), "Detect Contour Map Tops", "Number of tops to detect:", 10, 1, 1000, 1, &ok );
    if ( !ok ) return;

    double minDistance =
        QInputDialog::getDouble( RiuMainWindow::instance(), "Detect Contour Map Tops", "Minimum distance between tops:", 0.0, 0.0, 1.0e9, 1, &ok );
    if ( !ok ) return;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops         = maxTops;
    settings.minDistance     = minDistance;
    settings.excludeEdgeTops = true;

    auto tops = rigContourMapProjection->findTops( settings );
    if ( tops.empty() )
    {
        QMessageBox::information( RiuMainWindow::instance(), "Detect Contour Map Tops", "No tops were found in the current contour map." );
        return;
    }

    // Rank the detected tops by value (highest first), independent of the prominence-based criterion
    // used to select which peaks to keep.
    std::sort( tops.begin(), tops.end(), []( const auto& a, const auto& b ) { return a.z > b.z; } );

    auto* topsCollection = contourMapProjection->topsCollection();
    if ( !topsCollection ) return;

    topsCollection->clearTops();

    auto origin3d = rigContourMapProjection->origin3d();
    auto depth    = rigContourMapProjection->topDepthBoundingBox();

    // Sphere radius factor is multiplied by the view's characteristic cell size when rendered.
    const double sphereRadiusFactor = 0.3;

    for ( size_t i = 0; i < tops.size(); ++i )
    {
        const auto& top = tops[i];

        cvf::Vec3d domainPoint( origin3d.x() + top.x, origin3d.y() + top.y, depth );
        topsCollection->addTop( static_cast<int>( i + 1 ), top.z, top.prominence, domainPoint, sphereRadiusFactor );
    }

    topsCollection->updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDetectContourMapTopsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/WellTargetPoint16x16.png" ) );
    actionToSetup->setText( "Detect Contour Map Tops" );
}
