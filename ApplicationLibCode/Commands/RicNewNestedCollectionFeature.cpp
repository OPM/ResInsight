/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicNewNestedCollectionFeature.h"

#include "RiaNameUniquenessTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmNestedCollectionBase.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewNestedCollectionFeature, "RicNewNestedCollectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static caf::PdmNestedCollectionBase* selectedNestedCollection()
{
    auto items = caf::SelectionManager::instance()->selectedItems();
    for ( auto* item : items )
    {
        if ( auto* nested = dynamic_cast<caf::PdmNestedCollectionBase*>( item ) )
        {
            return nested;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewNestedCollectionFeature::isCommandEnabled() const
{
    auto* nested = selectedNestedCollection();
    return nested != nullptr && nested->canAddSubCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewNestedCollectionFeature::onActionTriggered( bool isChecked )
{
    auto* parent = selectedNestedCollection();
    if ( !parent || !parent->canAddSubCollection() ) return;

    caf::PdmObject* added = parent->addNewSubCollection();
    if ( !added ) return;

    RiaNameUniquenessTools::ensureUniqueAmongSiblings( added );

    Riu3DMainWindowTools::selectAsCurrentItem( added );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewNestedCollectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Folder" );
    actionToSetup->setIcon( QIcon( ":/Folder.png" ) );
}
