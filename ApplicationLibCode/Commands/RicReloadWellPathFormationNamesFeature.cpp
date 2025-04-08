/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicReloadWellPathFormationNamesFeature.h"

#include "RimMainPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadWellPathFormationNamesFeature, "RicReloadWellPathFormationNamesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadWellPathFormationNamesFeature::isCommandEnabled() const
{
    const auto wellPaths           = caf::SelectionManager::instance()->objectsByType<RimWellPath>();
    const auto wellPathCollections = caf::SelectionManager::instance()->objectsByType<RimWellPathCollection>();

    return ( !wellPaths.empty() || !wellPathCollections.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadWellPathFormationNamesFeature::onActionTriggered( bool isChecked )
{
    const auto wellPaths           = caf::SelectionManager::instance()->objectsByType<RimWellPath>();
    const auto wellPathCollections = caf::SelectionManager::instance()->objectsByType<RimWellPathCollection>();

    if ( !wellPaths.empty() )
    {
        RimWellPathCollection* wellPathCollection = wellPaths[0]->firstAncestorOrThisOfTypeAsserted<RimWellPathCollection>();
        wellPathCollection->reloadAllWellPathFormations();
    }
    else if ( !wellPathCollections.empty() )
    {
        wellPathCollections[0]->reloadAllWellPathFormations();
    }

    RimMainPlotCollection::current()->updatePlotsWithFormations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadWellPathFormationNamesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload All Well Picks" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}
