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

#include "RicToggleAxisAutoZoomFeature.h"

#include "RimPlotAxisPropertiesInterface.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicToggleAxisAutoZoomFeature, "RicToggleAxisAutoZoomFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleAxisAutoZoomFeature::isCommandEnabled() const
{
    auto* axisProperties = caf::firstAncestorOfTypeFromSelectedObject<RimPlotAxisPropertiesInterface>();
    return ( axisProperties != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleAxisAutoZoomFeature::onActionTriggered( bool isChecked )
{
    auto* axisProperties = caf::firstAncestorOfTypeFromSelectedObject<RimPlotAxisPropertiesInterface>();
    if ( !axisProperties ) return;

    bool enableAutoZoom = !axisProperties->isAutoZoom();
    axisProperties->setAutoZoom( enableAutoZoom );

    // Re-enabling auto zoom releases the user-defined range, so the axis follows the data again. This also updates
    // the pinned tag in the project tree.
    axisProperties->setRangeUserDefined( !enableAutoZoom );

    axisProperties->settingsChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleAxisAutoZoomFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Auto Zoom" );
    actionToSetup->setIcon( QIcon( ":/ZoomAll.svg" ) );
    actionToSetup->setCheckable( true );

    auto* axisProperties = caf::firstAncestorOfTypeFromSelectedObject<RimPlotAxisPropertiesInterface>();
    if ( axisProperties )
    {
        actionToSetup->setChecked( axisProperties->isAutoZoom() );
    }
}
