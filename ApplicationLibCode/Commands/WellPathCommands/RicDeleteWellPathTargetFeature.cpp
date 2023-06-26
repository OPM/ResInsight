/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RicDeleteWellPathTargetFeature.h"

CAF_CMD_SOURCE_INIT( RicDeleteWellPathTargetFeature, "RicDeleteWellPathTargetFeature" );

#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "cafSelectionManager.h"

#include <QAction>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteWellPathTargetFeature::isCommandEnabled()
{
    std::vector<RimWellPathTarget*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects, caf::SelectionManager::FIRST_LEVEL );

    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathTargetFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellPathTarget*> targets;
    caf::SelectionManager::instance()->objectsByType( &targets, caf::SelectionManager::FIRST_LEVEL );

    if ( targets.size() > 0 )
    {
        RimWellPathGeometryDef* wellGeomDef = targets[0]->firstAncestorOrThisOfTypeAsserted<RimWellPathGeometryDef>();

        for ( auto target : targets )
        {
            wellGeomDef->deleteTarget( target );
        }

        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathTargetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Target" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
