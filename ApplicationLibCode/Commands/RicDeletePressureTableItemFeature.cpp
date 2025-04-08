/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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
#include "RicDeletePressureTableItemFeature.h"

#include "RimPressureTable.h"
#include "RimPressureTableItem.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeletePressureTableItemFeature, "RicDeletePressureTableItemFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeletePressureTableItemFeature::isCommandEnabled() const
{
    auto objects = caf::SelectionManager::instance()->objectsByType<RimPressureTableItem>( caf::SelectionManager::FIRST_LEVEL );
    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeletePressureTableItemFeature::onActionTriggered( bool isChecked )
{
    auto objects = caf::SelectionManager::instance()->objectsByType<RimPressureTableItem>( caf::SelectionManager::FIRST_LEVEL );
    if ( !objects.empty() )
    {
        RimPressureTable* pressureTable = objects[0]->firstAncestorOrThisOfTypeAsserted<RimPressureTable>();
        for ( RimPressureTableItem* attributeToDelete : objects )
        {
            pressureTable->deleteItem( attributeToDelete );
        }
        pressureTable->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeletePressureTableItemFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}
