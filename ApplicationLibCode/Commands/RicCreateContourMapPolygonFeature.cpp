/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RiaLogging.h"

#include "RicExportContourMapToTextFeature.h"
#include "RicPolygonFromImageDialog.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimGeoMechContourMapView.h"
#include "RimTools.h"

#include "RigContourMapProjection.h"
#include "RigPolygonTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateContourMapPolygonFeature, "RicCreateContourMapPolygonFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonFeature::onActionTriggered( bool isChecked )
{
    RimContourMapProjection* contourMapProjection = nullptr;

    auto [existingEclipseContourMap, existingGeoMechContourMap] = RicExportContourMapToTextFeature::findContourMapView();
    if ( existingEclipseContourMap ) contourMapProjection = existingEclipseContourMap->contourMapProjection();
    if ( existingGeoMechContourMap ) contourMapProjection = existingGeoMechContourMap->contourMapProjection();

    if ( !contourMapProjection ) return;

    auto rigContourMapProjection = contourMapProjection->mapProjection();
    if ( !rigContourMapProjection ) return;

    auto vertexSizeIJ = rigContourMapProjection->numberOfVerticesIJ();

    std::vector<std::vector<int>> image( vertexSizeIJ.x(), std::vector<int>( vertexSizeIJ.y(), 0 ) );

    for ( cvf::uint i = 0; i < vertexSizeIJ.x(); i++ )
    {
        for ( cvf::uint j = 0; j < vertexSizeIJ.y(); j++ )
        {
            double valueAtVertex = rigContourMapProjection->valueAtVertex( i, j );

            if ( !std::isinf( valueAtVertex ) )
            {
                image[i][j] = 1;
            }
            else
            {
                image[i][j] = 0;
            }
        }
    }

    ImageProcessingDialog dlg;
    dlg.show();
    dlg.setImageData( image );
    dlg.updateAndShowImages();

    if ( dlg.exec() == QDialog::Rejected ) return;

    auto finalImage = dlg.finalImageData();
    if ( finalImage.empty() ) return;

    auto boundaryPoints = RigPolygonTools::boundary( finalImage );

    std::vector<cvf::Vec3d> polygonDomainCoords;
    {
        auto xVertexPositions = rigContourMapProjection->xVertexPositions();
        auto yVertexPositions = rigContourMapProjection->yVertexPositions();
        auto origin3d         = rigContourMapProjection->origin3d();

        for ( auto [i, j] : boundaryPoints )
        {
            double xDomain = xVertexPositions.at( i ) + origin3d.x();
            double yDomain = yVertexPositions.at( j ) + origin3d.y();

            polygonDomainCoords.emplace_back( cvf::Vec3d( xDomain, yDomain, origin3d.z() ) );
        }

        // Epsilon used to simplify polygon. Useful range typical value in [5..30]
        const double defaultEpsilon = 20.0;
        RigPolygonTools::simplifyPolygon( polygonDomainCoords, defaultEpsilon );
    }

    if ( polygonDomainCoords.size() >= 3 )
    {
        auto polygonCollection = RimTools::polygonCollection();

        auto newPolygon = polygonCollection->appendUserDefinedPolygon();

        newPolygon->setPointsInDomainCoords( polygonDomainCoords );
        newPolygon->coordinatesChanged.send();

        polygonCollection->uiCapability()->updateAllRequiredEditors();
    }
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
