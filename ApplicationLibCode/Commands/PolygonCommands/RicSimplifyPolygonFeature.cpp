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

#include "RigCellGeometryTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QInputDialog>

CAF_CMD_SOURCE_INIT( RicSimplifyPolygonFeature, "RicSimplifyPolygonFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSimplifyPolygonFeature::onActionTriggered( bool isChecked )
{
    auto sourcePolygon = caf::SelectionManager::instance()->selectedItemOfType<RimPolygon>();
    if ( !sourcePolygon )
    {
        auto sourcePolygonInView = caf::SelectionManager::instance()->selectedItemOfType<RimPolygonInView>();
        if ( sourcePolygonInView )
        {
            sourcePolygon = sourcePolygonInView->polygon();
        }
    }

    if ( !sourcePolygon ) return;

    const double defaultEpsilon = 10.0;

    bool ok;
    auto epsilon =
        QInputDialog::getDouble( nullptr, "Simplify Polygon Threshold", "Threshold:", defaultEpsilon, 1.0, 1000.0, 1, &ok, Qt::WindowFlags(), 1 );

    if ( ok )
    {
        auto coords = sourcePolygon->pointsInDomainCoords();
        RigCellGeometryTools::simplifyPolygon( &coords, epsilon );

        sourcePolygon->setPointsInDomainCoords( coords );
        sourcePolygon->coordinatesChanged.send();
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
