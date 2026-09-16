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

    // The default polygon result (field outline) does not require a name/contact type, so this is
    // already a complete selection once a data source is set -- fetch immediately, since there is
    // no "Apply" step in this property panel and setDataSource()/setRealization() do not trigger a
    // fetch on their own (they are also used from fieldChangedByUi, where loadData() is called
    // separately once per field-change instead).
    newCloudAddress->loadData();

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
