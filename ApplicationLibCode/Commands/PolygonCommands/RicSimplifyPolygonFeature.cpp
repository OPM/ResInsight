////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicSimplifyPolygonFeature.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"

#include "RigPolygonTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QInputDialog>

CAF_CMD_SOURCE_INIT( RicSimplifyPolygonFeature, "RicSimplifyPolygonFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSimplifyPolygonFeature::RicSimplifyPolygonFeature()
    : RicBasicPolygonFeature( true /*multiselect*/ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSimplifyPolygonFeature::onActionTriggered( bool isChecked )
{
    auto selPolygons = selectedPolygons();
    if ( selPolygons.empty() ) return;

    std::vector<std::vector<cvf::Vec3d>> originalCoords;

    for ( auto sourcePolygon : selPolygons )
    {
        originalCoords.push_back( sourcePolygon->pointsInDomainCoords() );
    }

    const int defaultEpsilon = 10;

    QInputDialog inputDialog;
    inputDialog.setWindowTitle( "Simplify Polygon" );
    inputDialog.setLabelText( "Threshold (larger value removes more points) :" );
    inputDialog.setInputMode( QInputDialog::IntInput );
    inputDialog.setIntRange( 10, 200 );
    inputDialog.setIntValue( defaultEpsilon );

    connect( &inputDialog,
             &QInputDialog::intValueChanged,
             [&originalCoords, &selPolygons]( int value )
             {
                 for ( size_t i = 0; i < originalCoords.size(); i++ )
                 {
                     auto coords = originalCoords[i];
                     RigPolygonTools::simplifyPolygon( coords, value );

                     auto sourcePolygon = selPolygons[i];
                     sourcePolygon->setPointsInDomainCoords( coords );
                     sourcePolygon->coordinatesChanged.send();
                 }
             } );

    if ( inputDialog.exec() == QDialog::Rejected )
    {
        for ( size_t i = 0; i < originalCoords.size(); i++ )
        {
            auto coords = originalCoords[i];

            auto sourcePolygon = selPolygons[i];
            sourcePolygon->setPointsInDomainCoords( coords );
            sourcePolygon->coordinatesChanged.send();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSimplifyPolygonFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Simplify Polygon" );
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
}
