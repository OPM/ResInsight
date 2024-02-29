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

#include "RicReloadPolygonFileFeature.h"

#include "Polygons/RimPolygonFile.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadPolygonFileFeature, "RicReloadPolygonFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPolygonFileFeature::onActionTriggered( bool isChecked )
{
    auto polygonFile = caf::SelectionManager::instance()->selectedItemOfType<RimPolygonFile>();
    if ( polygonFile )
    {
        polygonFile->loadData();
        polygonFile->objectChanged.send();

        polygonFile->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPolygonFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}
