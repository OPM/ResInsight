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

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadPolygonCloudAddressFeature, "RicReloadPolygonCloudAddressFeature" );

//--------------------------------------------------------------------------------------------------
/// Reloads whatever is currently visualized for the selected address(es): re-fetches base data
/// and any cached realizations. Does not rebuild the owning source's directory tree.
//--------------------------------------------------------------------------------------------------
void RicReloadPolygonCloudAddressFeature::onActionTriggered( bool isChecked )
{
    auto cloudAddresses = caf::selectedObjectsByType<RimPolygonCloudAddress*>();

    for ( auto address : cloudAddresses )
    {
        std::vector<int> cachedRealizations = address->cachedRealizations();

        if ( address->hasBaseData() )
        {
            address->evictBaseData();
            address->ensureBaseFetched();
        }

        for ( int realization : cachedRealizations )
        {
            address->evictRealizationIfUnused( realization );
            address->ensureRealizationFetched( realization );
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
