/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RicDeleteUncheckedSubItemsFeature.h"

#include "RicDeleteSubItemsFeature.h"

#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteUncheckedSubItemsFeature, "RicDeleteUncheckedSubItemsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteUncheckedSubItemsFeature::isCommandEnabled()
{
    return RicDeleteSubItemsFeature::canCommandBeEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteUncheckedSubItemsFeature::onActionTriggered( bool isChecked )
{
    bool onlyUnchecked = true;
    RicDeleteSubItemsFeature::deleteSubItems( onlyUnchecked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteUncheckedSubItemsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Unchecked Sub Items" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
