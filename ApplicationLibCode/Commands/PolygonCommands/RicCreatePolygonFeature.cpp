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

#include "RicCreatePolygonFeature.h"

#include "RiaApplication.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonTools.h"
#include "Rim3dView.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreatePolygonFeature, "RicCreatePolygonFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePolygonFeature::onActionTriggered( bool isChecked )
{
    auto polygonCollection = RimTools::polygonCollection();

    auto newPolygon = polygonCollection->appendUserDefinedPolygon();
    polygonCollection->uiCapability()->updateAllRequiredEditors();

    Riu3DMainWindowTools::setExpanded( newPolygon );

    auto activeView = RiaApplication::instance()->activeReservoirView();
    RimPolygonTools::activate3dEditOfPolygonInView( newPolygon, activeView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePolygonFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Polygon" );
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
}
