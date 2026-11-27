/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicNewGeoGuidedWellPlacementFeature.h"

#include "RimGeoGuidedWellPlacement.h"
#include "RimReservoirGridEnsemble.h"

#include "RiuMainWindow.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewGeoGuidedWellPlacementFeature, "RicNewGeoGuidedWellPlacementFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewGeoGuidedWellPlacementFeature::isCommandEnabled() const
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimReservoirGridEnsemble>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGeoGuidedWellPlacementFeature::onActionTriggered( bool isChecked )
{
    auto ensemble = caf::firstAncestorOfTypeFromSelectedObject<RimReservoirGridEnsemble>();
    if ( !ensemble ) return;

    auto placement = new RimGeoGuidedWellPlacement();
    ensemble->addGeoGuidedWellPlacement( placement );
    placement->setDefaults();

    ensemble->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem( placement );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGeoGuidedWellPlacementFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Geo-Guided Well Placement" );
    actionToSetup->setIcon( QIcon( ":/EditableWell.png" ) );
}
