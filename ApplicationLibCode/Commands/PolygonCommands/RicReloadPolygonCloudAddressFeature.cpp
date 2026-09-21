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

#include "RicReloadPolygonCloudAddressFeature.h"

#include "Polygons/Cloud/RimPolygonCloudAddress.h"
#include "Polygons/Cloud/RimPolygonCloudRealizationGroup.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadPolygonCloudAddressFeature, "RicReloadPolygonCloudAddressFeature" );

//--------------------------------------------------------------------------------------------------
/// Reloads whatever is currently visualized for the selected address(es): if base data has been
/// fetched, evict and re-fetch it; re-fetch every existing realization-comparison group the same
/// way. Does not rebuild the owning RimPolygonCloudSource's directory tree (that is a separate,
/// one-time operation).
//--------------------------------------------------------------------------------------------------
void RicReloadPolygonCloudAddressFeature::onActionTriggered( bool isChecked )
{
    auto cloudAddresses = caf::selectedObjectsByType<RimPolygonCloudAddress*>();

    for ( auto address : cloudAddresses )
    {
        if ( address->hasBaseData() )
        {
            address->evictBaseData();
            address->ensureBaseFetched();
        }

        for ( int realization : address->fetchedRealizationGroupRealizations() )
        {
            address->evictRealizationGroup( realization );
            address->ensureRealizationGroupFetched( realization );
        }

        address->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPolygonCloudAddressFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}
