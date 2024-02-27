/////////////////////////////////////////////////////////////////////////////////
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

#include "RicNewPolygonIntersectionFeature.h"

#include "RiaApplication.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPolygonIntersectionFeature, "RicNewPolygonIntersectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonIntersectionFeature::onActionTriggered( bool isChecked )
{
    RimGridView* activeView = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !activeView ) return;

    auto collection = activeView->intersectionCollection();
    if ( !collection ) return;

    auto polygon = caf::SelectionManager::instance()->selectedItemOfType<RimPolygon>();
    if ( !polygon )
    {
        if ( auto polygonInView = caf::SelectionManager::instance()->selectedItemOfType<RimPolygonInView>() )
        {
            polygon = polygonInView->polygon();
        }
    }

    auto intersection = new RimExtrudedCurveIntersection();
    intersection->configureForProjectPolyLine( polygon );
    collection->appendIntersectionAndUpdate( intersection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonIntersectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CrossSection16x16.png" ) );
    actionToSetup->setText( "Create Polygon Intersection" );
}
