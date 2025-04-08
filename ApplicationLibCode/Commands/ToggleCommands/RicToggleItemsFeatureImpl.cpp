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

#include "RiaFeatureCommandContext.h"
#include "RiaGuiApplication.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include <QModelIndex>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleItemsFeatureImpl::isToggleCommandsAvailable()
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() == 1 )
    {
        caf::PdmUiTreeOrdering* treeItem = findTreeItemFromSelectedUiItem( selectedItems[0] );

        if ( !treeItem ) return false;

        for ( int cIdx = 0; cIdx < treeItem->childCount(); ++cIdx )
        {
            caf::PdmUiTreeOrdering* child = treeItem->child( cIdx );
            if ( !child ) continue;
            if ( !child->isRepresentingObject() ) continue;

            caf::PdmObjectHandle*   childObj            = child->object();
            caf::PdmUiObjectHandle* uiObjectHandleChild = uiObj( childObj );

            if ( uiObjectHandleChild && uiObjectHandleChild->objectToggleField() &&
                 !uiObjectHandleChild->objectToggleField()->uiCapability()->isUiReadOnly() )
            {
                return true;
            }
        }
    }
    else
    {
        for ( auto& selectedItem : selectedItems )
        {
            auto* uiObjectHandle = dynamic_cast<caf::PdmUiObjectHandle*>( selectedItem );
            if ( uiObjectHandle && uiObjectHandle->objectToggleField() )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleItemsFeatureImpl::isToggleCommandsForSubItems()
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    return isToggleCommandsAvailable() && selectedItems.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// Set toggle state for list of model indices.
//--------------------------------------------------------------------------------------------------
void RicToggleItemsFeatureImpl::setObjectToggleStateForSelection( SelectionToggleType state )
{
    auto selectedFields = findToggleFieldsFromSelection( state );

    std::vector<caf::PdmField<bool>*> fieldsToUpdate;
    if ( state == TOGGLE_OFF || state == TOGGLE_ON )
    {
        // Exclude field having the target state. If these fields are included, the one and only call to setValueWithFieldChanged() can
        // contain a field with the target state value. When setting a value to a field with the same value, nothing happens and the UI will
        // get an inconsistent state (some curves toggled off are still visible in a plot).

        const bool targetState = state == TOGGLE_ON;

        for ( const auto& field : selectedFields )
        {
            bool currentValue = field->v();
            if ( currentValue != targetState )
            {
                fieldsToUpdate.push_back( field );
            }
        }
    }
    else
    {
        // All fields will be updated when toggling
        fieldsToUpdate = selectedFields;
    }

    if ( fieldsToUpdate.empty() ) return;

    auto lastField = fieldsToUpdate.back();
    for ( auto field : fieldsToUpdate )
    {
        bool value = !( field->v() );

        if ( state == TOGGLE_ON )
            value = true;
        else if ( state == TOGGLE_OFF )
            value = false;

        if ( field == lastField )
        {
            field->setValueWithFieldChanged( value );
        }
        else
        {
            field->setValue( value );
        }
    }

    // If multiple fields are updated, we call onChildrenUpdated() on the owner of the first field
    // Example: Trigger replot of curves when multiple curves are toggled
    if ( fieldsToUpdate.size() > 1 )
    {
        auto [ownerOfChildArrayField, childArrayFieldHandle] = RicToggleItemsFeatureImpl::findOwnerAndChildArrayField( fieldsToUpdate.front() );
        if ( ownerOfChildArrayField && childArrayFieldHandle )
        {
            std::vector<caf::PdmObjectHandle*> objs;
            ownerOfChildArrayField->onChildrenUpdated( childArrayFieldHandle, objs );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicToggleItemsFeatureImpl::findCollectionName( SelectionToggleType state )
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( state != TOGGLE && selectedItems.size() == 1 )
    {
        caf::PdmUiTreeOrdering* treeItem = findTreeItemFromSelectedUiItem( selectedItems[0] );
        if ( !treeItem ) return {};

        for ( int cIdx = 0; cIdx < treeItem->childCount(); ++cIdx )
        {
            caf::PdmUiTreeOrdering* child = treeItem->child( cIdx );
            if ( !child ) continue;
            if ( !child->isRepresentingObject() ) continue;

            caf::PdmObjectHandle*   childObj            = child->object();
            caf::PdmUiObjectHandle* uiObjectHandleChild = uiObj( childObj );
            if ( !uiObjectHandleChild ) continue;

            // https://github.com/OPM/ResInsight/issues/8382
            // Toggling state is only supported for objects in an array.
            // For example, this will ensure that faults are toggled without altering the fault result object.
            auto arrayField = dynamic_cast<caf::PdmChildArrayFieldHandle*>( childObj->parentField() );
            if ( arrayField && arrayField->ownerObject() )
            {
                return arrayField->uiCapability()->uiName();
            }
        }
    }
    else
    {
        for ( auto& selectedItem : selectedItems )
        {
            auto* uiObjectHandle = dynamic_cast<caf::PdmUiObjectHandle*>( selectedItem );
            if ( uiObjectHandle && uiObjectHandle->objectToggleField() )
            {
                QString objectName = uiObjectHandle->uiName();
                return objectName;
            }
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeView* RicToggleItemsFeatureImpl::findTreeView( const caf::PdmUiItem* uiItem )
{
    RiaFeatureCommandContext* context = RiaFeatureCommandContext::instance();

    auto* customActiveTreeView = dynamic_cast<caf::PdmUiTreeView*>( context->object() );
    if ( customActiveTreeView )
    {
        return customActiveTreeView;
    }

    auto* main3dWindow = RiaGuiApplication::instance()->mainWindow();
    if ( main3dWindow )
    {
        auto activeTree = main3dWindow->getTreeViewWithItem( uiItem );
        if ( activeTree ) return activeTree;
    }

    auto* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    if ( mainPlotWindow )
    {
        auto activeTree = mainPlotWindow->getTreeViewWithItem( uiItem );
        if ( activeTree )
        {
            return activeTree;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Finds the tree item in either the 3D main window or plot main window project tree view
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeOrdering* RicToggleItemsFeatureImpl::findTreeItemFromSelectedUiItem( const caf::PdmUiItem* uiItem )
{
    caf::PdmUiTreeView* pdmUiTreeView = findTreeView( uiItem );

    if ( pdmUiTreeView )
    {
        QModelIndex modIndex = pdmUiTreeView->findModelIndex( uiItem );
        return pdmUiTreeView->uiTreeOrderingFromModelIndex( modIndex );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmField<bool>*> RicToggleItemsFeatureImpl::findToggleFieldsFromSelection( SelectionToggleType state )
{
    std::vector<caf::PdmField<bool>*> fields;

    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( state != TOGGLE && selectedItems.size() == 1 )
    {
        // If only one item is selected, loop over its children, and toggle them instead of the
        // selected item directly

        // We need to get the children through the tree view, because that is where the actually shown children is

        caf::PdmUiTreeOrdering* treeItem = findTreeItemFromSelectedUiItem( selectedItems[0] );
        if ( !treeItem ) return {};

        for ( int cIdx = 0; cIdx < treeItem->childCount(); ++cIdx )
        {
            caf::PdmUiTreeOrdering* child = treeItem->child( cIdx );
            if ( !child ) continue;
            if ( !child->isRepresentingObject() ) continue;

            caf::PdmObjectHandle*   childObj            = child->object();
            caf::PdmUiObjectHandle* uiObjectHandleChild = uiObj( childObj );
            if ( !uiObjectHandleChild ) continue;

            // https://github.com/OPM/ResInsight/issues/8382
            // Toggling state is only supported for objects in an array.
            // For example, this will ensure that faults are toggled without altering the fault result object.
            auto arrayField = dynamic_cast<caf::PdmChildArrayFieldHandle*>( childObj->parentField() );
            if ( !arrayField ) continue;

            if ( uiObjectHandleChild->objectToggleField() )
            {
                auto field = dynamic_cast<caf::PdmField<bool>*>( uiObjectHandleChild->objectToggleField() );
                if ( !field ) continue;

                if ( state == SelectionToggleType::TOGGLE_ON && field->value() ) continue;
                if ( state == SelectionToggleType::TOGGLE_OFF && !field->value() ) continue;

                fields.emplace_back( field );
            }
        }
    }
    else
    {
        for ( auto& selectedItem : selectedItems )
        {
            auto* uiObjectHandle = dynamic_cast<caf::PdmUiObjectHandle*>( selectedItem );
            if ( uiObjectHandle && uiObjectHandle->objectToggleField() )
            {
                auto* field = dynamic_cast<caf::PdmField<bool>*>( uiObjectHandle->objectToggleField() );

                fields.emplace_back( field );
            }
        }
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
/// Based on code in CmdUiCommandSystemImpl::fieldChangedCommand()
/// Could be merged and put into a tool class
//--------------------------------------------------------------------------------------------------
std::pair<caf::PdmObjectHandle*, caf::PdmChildArrayFieldHandle*>
    RicToggleItemsFeatureImpl::findOwnerAndChildArrayField( caf::PdmFieldHandle* fieldHandle )
{
    if ( !fieldHandle ) return {};

    caf::PdmChildArrayFieldHandle* childArrayFieldHandle  = nullptr;
    caf::PdmObjectHandle*          ownerOfChildArrayField = nullptr;

    // Find the first childArrayField by traversing parent field and objects. Usually, the childArrayField is
    // the parent, but in some cases when we change fields in a sub-object of the object we need to traverse
    // more levels

    ownerOfChildArrayField = fieldHandle->ownerObject();
    while ( ownerOfChildArrayField )
    {
        if ( ownerOfChildArrayField->parentField() )
        {
            childArrayFieldHandle  = dynamic_cast<caf::PdmChildArrayFieldHandle*>( ownerOfChildArrayField->parentField() );
            ownerOfChildArrayField = ownerOfChildArrayField->parentField()->ownerObject();

            if ( childArrayFieldHandle && ownerOfChildArrayField ) break;
        }
        else
        {
            ownerOfChildArrayField = nullptr;
        }
    }

    if ( ownerOfChildArrayField && childArrayFieldHandle )
    {
        return { ownerOfChildArrayField, childArrayFieldHandle };
    }

    return {};
}
