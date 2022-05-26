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
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

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
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );
    if ( isToggleCommandsAvailable() && selectedItems.size() == 1 )
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Set toggle state for list of model indices.
//--------------------------------------------------------------------------------------------------
void RicToggleItemsFeatureImpl::setObjectToggleStateForSelection( SelectionToggleType state )
{
    auto fields = findToggleFieldsFromSelection( state );
    if ( fields.empty() ) return;

    auto lastField = fields.back();
    for ( auto field : fields )
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

    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );
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

            if ( uiObjectHandleChild && uiObjectHandleChild->objectToggleField() )
            {
                auto* field = dynamic_cast<caf::PdmField<bool>*>( uiObjectHandleChild->objectToggleField() );
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
