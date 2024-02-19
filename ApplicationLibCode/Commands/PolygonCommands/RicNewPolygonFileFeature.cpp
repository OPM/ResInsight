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

#include "RicNewPolygonFileFeature.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonFile.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPolygonFileFeature, "RicNewPolygonFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFileFeature::onActionTriggered( bool isChecked )
{
    auto proj              = RimProject::current();
    auto polygonCollection = proj->activeOilField()->polygonCollection();

    auto newPolygonFile = new RimPolygonFile();
    newPolygonFile->setName( "File Polygon " + QString::number( polygonCollection->polygonFiles().size() + 1 ) );
    polygonCollection->addPolygonFile( newPolygonFile );
    polygonCollection->uiCapability()->updateAllRequiredEditors();

    RiuPlotMainWindowTools::setExpanded( newPolygonFile );
    RiuPlotMainWindowTools::selectAsCurrentItem( newPolygonFile );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New File Polygon" );
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
}
