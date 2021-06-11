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
#include "RicNewPressureTableItemFeature.h"

#include "RimPressureTable.h"
#include "RimPressureTableItem.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPressureTableItemFeature, "RicNewPressureTableItemFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPressureTableItemFeature::isCommandEnabled()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimPressureTable>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPressureTableItemFeature::onActionTriggered( bool isChecked )
{
    RimPressureTable* pressureTable = caf::SelectionManager::instance()->selectedItemOfType<RimPressureTable>();
    if ( pressureTable )
    {
        RimPressureTableItem* attribute = new RimPressureTableItem;
        pressureTable->insertItem( nullptr, attribute );
        pressureTable->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPressureTableItemFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Append New Item" );
}
