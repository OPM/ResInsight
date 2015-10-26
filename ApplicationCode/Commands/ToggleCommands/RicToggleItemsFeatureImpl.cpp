/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicToggleItemsFeatureImpl.h"
#include <vector>
#include "cafPdmUiObjectHandle.h"
#include "cafSelectionManager.h"
#include "cafPdmUiItem.h"
#include <QModelIndex>
#include "RiuMainWindow.h"
#include "cafPdmUiTreeView.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiFieldHandle.h"

bool RicToggleItemsFeatureImpl::isToggleCommandsAvailable()
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems(selectedItems);

    if (selectedItems.size() == 1)
    {
        QModelIndex modIndex = RiuMainWindow::instance()->projectTreeView()->findModelIndex(selectedItems[0]);
        caf::PdmUiTreeOrdering* treeItem = static_cast<caf::PdmUiTreeOrdering*>(modIndex.internalPointer()); 
        if (!treeItem) return false;

        for (int cIdx = 0; cIdx < treeItem->childCount(); ++ cIdx)
        {
            caf::PdmUiTreeOrdering*  child = treeItem->child(cIdx);
            if (!child) continue;
            if (!child->isRepresentingObject()) continue;

            caf::PdmObjectHandle* childObj = child->object();
            caf::PdmUiObjectHandle* uiObjectHandleChild = uiObj(childObj);

            if (uiObjectHandleChild && uiObjectHandleChild->objectToggleField())
            {
                return true;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < selectedItems.size(); ++i)
        {
            caf::PdmUiObjectHandle* uiObjectHandle = dynamic_cast<caf::PdmUiObjectHandle*>(selectedItems[i]);

            if (uiObjectHandle && uiObjectHandle->objectToggleField())
            {
                return true;
            }
        }
    }

    return false;
}

bool RicToggleItemsFeatureImpl::isToggleCommandsForSubItems()
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems(selectedItems);
    if (isToggleCommandsAvailable() && selectedItems.size() == 1)
    { 
        return true;
    }
    return false;
}


//--------------------------------------------------------------------------------------------------
/// Set toggle state for list of model indices. 
//--------------------------------------------------------------------------------------------------
void RicToggleItemsFeatureImpl::setObjectToggleStateForSelection(SelectionToggleType state)
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems(selectedItems);
    if (state != TOGGLE && selectedItems.size() == 1)
    {
        // If only one item is selected, loop over its children, and toggle them instead of the 
        // selected item directly

        // We need to get the children through the tree view, because that is where the actually shown children is 
        
        QModelIndex modIndex = RiuMainWindow::instance()->projectTreeView()->findModelIndex(selectedItems[0]);
        caf::PdmUiTreeOrdering* treeItem = reinterpret_cast<caf::PdmUiTreeOrdering*>(modIndex.internalPointer()); 

        for (int cIdx = 0; cIdx < treeItem->childCount(); ++ cIdx)
        {
            caf::PdmUiTreeOrdering*  child = treeItem->child(cIdx);
            if (!child) continue;
            if (!child->isRepresentingObject()) continue;

            caf::PdmObjectHandle* childObj = child->object();
            caf::PdmUiObjectHandle* uiObjectHandleChild = uiObj(childObj);

            if (uiObjectHandleChild && uiObjectHandleChild->objectToggleField())
            {
                caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>(uiObjectHandleChild->objectToggleField());

                caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
                if (uiFieldHandle)
                {
                    if (state == TOGGLE_ON)  uiFieldHandle->setValueFromUi(true);
                    if (state == TOGGLE_OFF) uiFieldHandle->setValueFromUi(false);
                    if (state == TOGGLE_SUBITEMS)     uiFieldHandle->setValueFromUi(!(field->v()));
                }
            }
        }
    }
    else
    {
        for (size_t i = 0; i < selectedItems.size(); ++i)
        {
            caf::PdmUiObjectHandle* uiObjectHandle = dynamic_cast< caf::PdmUiObjectHandle*>(selectedItems[i]);

            if (uiObjectHandle && uiObjectHandle->objectToggleField())
            {
                caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>* >(uiObjectHandle->objectToggleField());

                caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
                if (uiFieldHandle)
                {
                    if (state == TOGGLE_ON)  uiFieldHandle->setValueFromUi(true);
                    if (state == TOGGLE_OFF) uiFieldHandle->setValueFromUi(false);
                    if (state == TOGGLE_SUBITEMS || state == TOGGLE)
                    {
                        uiFieldHandle->setValueFromUi(!(field->v()));
                    }
                }
            } 
        }
    }
}
