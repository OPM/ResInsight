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

#include "RicCreateContourMapPolygonFeature.h"

#include "RicCreateContourMapPolygonTools.h"

#include "RigPolygonTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateContourMapPolygonFeature, "RicCreateContourMapPolygonFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonFeature::onActionTriggered( bool isChecked )
{
    auto rigContourMapProjection = RicCreateContourMapPolygonTools::findCurrentContourMapProjection();
    if ( !rigContourMapProjection ) return;

    auto sourceImage = RicCreateContourMapPolygonTools::convertToBinaryImage( rigContourMapProjection );

    const int kernelSize  = 3;
    auto      floodFilled = RigPolygonTools::fillInterior( sourceImage );
    auto      eroded      = RigPolygonTools::erode( floodFilled, kernelSize );
    auto      dilated     = RigPolygonTools::dilate( eroded, kernelSize );

    RicCreateContourMapPolygonTools::createPolygonObjects( dilated, rigContourMapProjection );
}

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
    actionToSetup->setText( "Create Polygon From Contour Map" );
}
