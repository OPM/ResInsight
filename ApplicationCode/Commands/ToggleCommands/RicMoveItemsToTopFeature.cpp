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

#include "RicMoveItemsToTopFeature.h"

#include "RiaGuiApplication.h"
#include "RicToggleItemsFeatureImpl.h"
#include "RiuMainWindow.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmPtrArrayFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicMoveItemsToTopFeature, "RicMoveItemsToTopFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMoveItemsToTopFeature::isCommandEnabled()
{
    using namespace caf;

    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

    if ( !selectedItems.empty() )
    {
        auto pdmObject      = RicMoveItemsToTopFeature::objectHandleFromUiItem( selectedItems[0] );
        auto reorderability = PdmFieldReorderCapability::reorderCapabilityOfParentContainer( pdmObject );
        if ( reorderability )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMoveItemsToTopFeature::onActionTriggered( bool isChecked )
{
    using namespace caf;

    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

    if ( !selectedItems.empty() )
    {
        auto pdmObject      = RicMoveItemsToTopFeature::objectHandleFromUiItem( selectedItems[0] );
        auto reorderability = PdmFieldReorderCapability::reorderCapabilityOfParentContainer( pdmObject );

        if ( reorderability )
        {
            PdmPtrArrayFieldHandle* arrayField = dynamic_cast<PdmPtrArrayFieldHandle*>( pdmObject->parentField() );
            CAF_ASSERT( arrayField );

            bool anyObjectMoved = false;

            std::vector<caf::PdmUiItem*> reverseOrder( selectedItems );
            std::reverse( reverseOrder.begin(), reverseOrder.end() );
            for ( auto uiItem : reverseOrder )
            {
                auto currentObject = dynamic_cast<PdmObject*>( uiItem );
                if ( currentObject )
                {
                    size_t indexToMove = arrayField->size() + 1;
                    for ( auto i = 0; i < arrayField->size(); i++ )
                    {
                        if ( arrayField->at( i ) == currentObject )
                        {
                            indexToMove = i;
                            continue;
                        }
                    }

                    if ( reorderability->canItemBeMovedUp( indexToMove ) )
                    {
                        reorderability->moveItemToTop( indexToMove );

                        anyObjectMoved = true;
                    }
                }
            }

            if ( anyObjectMoved )
            {
                std::vector<const caf::PdmUiItem*> constSelectedItems;
                for ( auto s : selectedItems )
                {
                    constSelectedItems.push_back( s );
                }

                caf::PdmUiTreeView* uiTreeView = RiaGuiApplication::activeMainWindow()->projectTreeView();

                if ( !constSelectedItems.empty() )
                {
                    QModelIndex itemIndex   = uiTreeView->findModelIndex( constSelectedItems[0] );
                    QModelIndex parentIndex = itemIndex.parent();
                    uiTreeView->updateSubTree( parentIndex );
                }

                // Restore selection highlight after reordering
                uiTreeView->selectItems( constSelectedItems );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMoveItemsToTopFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Move to Top" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RicMoveItemsToTopFeature::objectHandleFromUiItem( caf::PdmUiItem* uiItem )
{
    using namespace caf;

    PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>( uiItem );

    if ( uiObjectHandle )
    {
        PdmObjectHandle* pdmObject = uiObjectHandle->objectHandle();

        return pdmObject;
    }

    return nullptr;
}
