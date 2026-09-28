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

#include "RicComputeContourMapPeaksFeature.h"

#include "RicExportContourMapToTextFeature.h"

#include "ContourMap/RimContourMapPeaksCollection.h"
#include "ContourMap/RimContourMapProjection.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "RimGeoMechContourMapView.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicComputeContourMapPeaksFeature, "RicComputeContourMapPeaksFeature" );

//--------------------------------------------------------------------------------------------------
/// Enabled for all contour map view types (Eclipse, GeoMech and ensemble statistics).
//--------------------------------------------------------------------------------------------------
bool RicComputeContourMapPeaksFeature::isCommandEnabled() const
{
    auto [existingEclipseContourMap, existingGeoMechContourMap] = RicExportContourMapToTextFeature::findContourMapView();

    return existingEclipseContourMap || existingGeoMechContourMap;
}

//--------------------------------------------------------------------------------------------------
/// Computes peaks using the peaks collection's current (default, unless previously edited) peakCount and
/// minDistance field values. Use the "Contour Map Peaks" object's own property panel to customize and
/// recompute.
//--------------------------------------------------------------------------------------------------
void RicComputeContourMapPeaksFeature::onActionTriggered( bool isChecked )
{
    auto [existingEclipseContourMap, existingGeoMechContourMap] = RicExportContourMapToTextFeature::findContourMapView();

    RimContourMapProjection* contourMapProjection = nullptr;
    if ( existingEclipseContourMap ) contourMapProjection = existingEclipseContourMap->contourMapProjection();
    if ( existingGeoMechContourMap ) contourMapProjection = existingGeoMechContourMap->contourMapProjection();
    if ( !contourMapProjection ) return;

    auto* peaksCollection = contourMapProjection->peaksCollection();
    if ( !peaksCollection ) return;

    peaksCollection->computePeaks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicComputeContourMapPeaksFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/WellTargetPoint16x16.png" ) );
    actionToSetup->setText( "Compute Contour Map Peaks" );
}
