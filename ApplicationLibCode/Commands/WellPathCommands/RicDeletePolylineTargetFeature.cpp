/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicDeletePolylineTargetFeature.h"

#include "RimPolylinePickerInterface.h"
#include "RimPolylineTarget.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeletePolylineTargetFeature, "RicDeletePolylineTargetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeletePolylineTargetFeature::isCommandEnabled() const
{
    const auto objects = caf::SelectionManager::instance()->objectsByType<RimPolylineTarget>( caf::SelectionManager::FIRST_LEVEL );
    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeletePolylineTargetFeature::onActionTriggered( bool isChecked )
{
    const auto targets = caf::SelectionManager::instance()->objectsByType<RimPolylineTarget>( caf::SelectionManager::FIRST_LEVEL );
    if ( !targets.empty() )
    {
        RimPolylinePickerInterface* polylineDef = targets[0]->firstAncestorOrThisOfTypeAsserted<RimPolylinePickerInterface>();

        for ( auto target : targets )
        {
            polylineDef->deleteTarget( target );
        }

        polylineDef->updateEditorsAndVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeletePolylineTargetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Target" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}
