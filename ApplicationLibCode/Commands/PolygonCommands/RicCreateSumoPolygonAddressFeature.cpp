////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicCreateSumoPolygonAddressFeature.h"

#include "Cloud/RimCloudDataSourceCollection.h"
#include "Cloud/RimSumoDataSource.h"

#include "Polygons/Cloud/RimPolygonCloudAddress.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSumoPolygonAddressFeature, "RicCreateSumoPolygonAddressFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoPolygonAddressFeature::onActionTriggered( bool isChecked )
{
    RimPolygonCollection* polygonCollection = nullptr;

    auto selected = caf::selectedObjectsByTypeStrict<RimPolygonCollection*>();
    if ( !selected.empty() ) polygonCollection = selected.front();

    if ( !polygonCollection ) polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return;

    auto newCloudAddress = new RimPolygonCloudAddress();

    // Data source and a default realization are pre-selected as a convenience, but the actual
    // fetch is left to the user via the "Apply" button in the property panel -- creating this via
    // the context menu should let the user finish choosing polygon result/name/contact type before
    // any request is made to ri-cloud-api.
    auto dataSources = RimCloudDataSourceCollection::instance()->sumoDataSources();
    if ( !dataSources.empty() )
    {
        auto* dataSource = dataSources.front();
        newCloudAddress->setDataSource( dataSource );

        auto realizationIds = dataSource->selectedRealizationIds();
        if ( !realizationIds.empty() )
        {
            bool ok          = false;
            int  realization = realizationIds.front().toInt( &ok );
            if ( ok ) newCloudAddress->setRealization( realization );
        }
    }

    polygonCollection->addPolygonCloudAddress( newCloudAddress );

    polygonCollection->uiCapability()->updateAllRequiredEditors();

    Riu3DMainWindowTools::setExpanded( newCloudAddress );
    Riu3DMainWindowTools::selectAsCurrentItem( newCloudAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoPolygonAddressFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Sumo Polygon" );
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
}
