////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicAddCloudPolygonSourceFeature.h"

#include "RiaLogging.h"

#include "Cloud/RimCloudDataSourceCollection.h"
#include "Cloud/RimSumoDataSource.h"

#include "Polygons/Cloud/RimPolygonCloudSource.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAddCloudPolygonSourceFeature, "RicAddCloudPolygonSourceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddCloudPolygonSourceFeature::onActionTriggered( bool isChecked )
{
    RimPolygonCollection* polygonCollection = nullptr;

    auto selected = caf::selectedObjectsByTypeStrict<RimPolygonCollection*>();
    if ( !selected.empty() ) polygonCollection = selected.front();

    if ( !polygonCollection ) polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return;

    auto dataSources = RimCloudDataSourceCollection::instance()->sumoDataSources();
    if ( dataSources.empty() )
    {
        RiaLogging::warning( "No Sumo data sources configured. Add one before adding a cloud polygon source." );
        return;
    }

    // Create an empty source -- the user selects Data Source and Base Realization in the property
    // panel and clicks "Apply" to fetch the polygon result directory and build the tree.
    auto* source = new RimPolygonCloudSource();

    polygonCollection->addPolygonCloudSource( source );

    polygonCollection->uiCapability()->updateAllRequiredEditors();

    Riu3DMainWindowTools::setExpanded( source );
    Riu3DMainWindowTools::selectAsCurrentItem( source );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddCloudPolygonSourceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Cloud Polygon Source" );
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
}
