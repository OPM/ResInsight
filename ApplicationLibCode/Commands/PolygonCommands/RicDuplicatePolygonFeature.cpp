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

#include "RicDuplicatePolygonFeature.h"

#include "RiaApplication.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonInView.h"
#include "Polygons/RimPolygonTools.h"
#include "Rim3dView.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDuplicatePolygonFeature, "RicDuplicatePolygonFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicDuplicatePolygonFeature::RicDuplicatePolygonFeature()
    : RicBasicPolygonFeature( false /*multiselect*/ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicatePolygonFeature::onActionTriggered( bool isChecked )
{
    auto selPolygons = selectedPolygons();
    if ( selPolygons.empty() ) return;

    auto sourcePolygon = selPolygons[0];

    auto polygonCollection = RimTools::polygonCollection();

    auto newPolygon = polygonCollection->createUserDefinedPolygon();
    newPolygon->setPointsInDomainCoords( sourcePolygon->pointsInDomainCoords() );
    auto sourceName = sourcePolygon->name();
    newPolygon->setName( "Copy of " + sourceName );
    polygonCollection->addUserDefinedPolygon( newPolygon );

    polygonCollection->uiCapability()->updateAllRequiredEditors();

    Riu3DMainWindowTools::setExpanded( newPolygon );

    auto activeView = RiaApplication::instance()->activeReservoirView();
    RimPolygonTools::selectPolygonInView( newPolygon, activeView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicatePolygonFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Duplicate Polygon" );
    actionToSetup->setIcon( QIcon( ":/caf/duplicate.svg" ) );
}
