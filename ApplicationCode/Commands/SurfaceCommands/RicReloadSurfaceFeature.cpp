/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicReloadSurfaceFeature.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicReloadSurfaceFeature, "RicReloadSurfaceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadSurfaceFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSurfaceFeature::onActionTriggered( bool isChecked )
{
    RimSurfaceCollection* surfColl = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimSurfaceCollection>();
    if ( surfColl )
    {
        // get the Surfaces
        std::vector<RimSurface*> surfaces = caf::selectedObjectsByTypeStrict<RimSurface*>();

        // ask the collection to reload them
        surfColl->reloadSurfaces( surfaces );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSurfaceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Refresh-32.png" ) );
    actionToSetup->setText( "Reload" );
}
