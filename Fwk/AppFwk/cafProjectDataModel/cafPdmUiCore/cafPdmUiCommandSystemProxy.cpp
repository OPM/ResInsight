//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafPdmUiCommandSystemProxy.h"

#include "cafInternalPdmUiCommandSystemInterface.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafSelectionManager.h"

#include <cstddef>
#include <typeinfo>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiCommandSystemProxy* PdmUiCommandSystemProxy::instance()
{
    static PdmUiCommandSystemProxy staticInstance;

    return &staticInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiCommandSystemProxy::PdmUiCommandSystemProxy()
{
    m_commandInterface = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCommandSystemProxy::setCommandInterface( PdmUiCommandSystemInterface* commandInterface )
{
    m_commandInterface = commandInterface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCommandSystemProxy::setUiValueToField( PdmUiFieldHandle* uiFieldHandle, const QVariant& newUiValue )
{
    if ( uiFieldHandle && uiFieldHandle->fieldHandle() )
    {
        // Handle editing multiple objects when several objects are selected
        PdmFieldHandle* editorField = uiFieldHandle->fieldHandle();

        std::vector<PdmFieldHandle*> fieldsToUpdate;
        fieldsToUpdate.push_back( editorField );

        std::vector<PdmFieldHandle*> otherSelectedFields = fieldsFromSelection( editorField );

        // If current edited field is part of the selection, update all fields in selection
        if ( std::find( otherSelectedFields.begin(), otherSelectedFields.end(), editorField ) != otherSelectedFields.end() )
        {
            for ( auto otherField : otherSelectedFields )
            {
                if ( otherField != editorField )
                {
                    fieldsToUpdate.push_back( otherField );
                }
            }
        }

        if ( m_commandInterface )
        {
            caf::PdmUiObjectHandle* uiOwnerObjectHandle = uiObj( editorField->ownerObject() );
            if ( uiOwnerObjectHandle && uiOwnerObjectHandle->useUndoRedoForFieldChanged() )
            {
                m_commandInterface->fieldChangedCommand( fieldsToUpdate, newUiValue );
                return;
            }
        }

        for ( auto fieldHandle : fieldsToUpdate )
        {
            fieldHandle->uiCapability()->setValueFromUiEditor( newUiValue );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCommandSystemProxy::setCurrentContextMenuTargetWidget( QWidget* targetWidget )
{
    if ( m_commandInterface )
    {
        m_commandInterface->setCurrentContextMenuTargetWidget( targetWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCommandSystemProxy::populateMenuWithDefaultCommands( const QString& uiConfigName, QMenu* menu )
{
    if ( m_commandInterface )
    {
        m_commandInterface->populateMenuWithDefaultCommands( uiConfigName, menu );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<PdmFieldHandle*> PdmUiCommandSystemProxy::fieldsFromSelection( PdmFieldHandle* editorField )
{
    if ( !editorField ) return {};
    if ( !editorField->ownerObject() ) return {};

    std::vector<PdmFieldHandle*> additionalFieldsToUpdate;

    const auto  fieldKeyword     = editorField->keyword();
    const auto& fieldOwnerTypeId = typeid( *editorField->ownerObject() );

    int                     selectionLevel = 0;
    std::vector<PdmUiItem*> items;
    SelectionManager::instance()->selectedItems( items, selectionLevel );
    for ( auto& item : items )
    {
        auto* objectHandle = dynamic_cast<PdmObjectHandle*>( item );
        if ( objectHandle )
        {
            if ( typeid( *objectHandle ) == fieldOwnerTypeId )
            {
                // An object of same type is selected, find field with same keyword as the current field being edited

                PdmFieldHandle* fieldHandle = objectHandle->findField( fieldKeyword );
                if ( fieldHandle )
                {
                    additionalFieldsToUpdate.push_back( fieldHandle );
                }
            }
            else
            {
                // Search one level in the project tree for fields in child objects
                // Searching in deeper levels is currently not supported, and is considered difficult to match correctly
                // and robust
                //
                // Check for identical owner class to guard for matching field names in multiple child objects of a
                // different type
                const auto editorFieldOwnerClass = editorField->ownerClass();

                std::vector<PdmFieldHandle*> childFields;
                objectHandle->fields( childFields );
                for ( auto field : childFields )
                {
                    std::vector<PdmObjectHandle*> childObjects;
                    field->childObjects( &childObjects );
                    for ( auto childObj : childObjects )
                    {
                        auto childFieldHandle = childObj->findField( fieldKeyword );
                        if ( childFieldHandle && childFieldHandle->ownerClass() == editorFieldOwnerClass )
                        {
                            additionalFieldsToUpdate.push_back( childFieldHandle );
                        }
                    }
                }
            }
        }
        else
        {
            // Todo Remove when dust has settled. Selection manager is not supposed to select single fields
            // A field is selected, check if keywords are identical
            auto* itemFieldHandle = dynamic_cast<PdmUiFieldHandle*>( item );
            if ( itemFieldHandle )
            {
                PdmFieldHandle* field = itemFieldHandle->fieldHandle();
                if ( field && field->keyword() == fieldKeyword )
                {
                    additionalFieldsToUpdate.push_back( field );
                }
            }
        }
    }

    return additionalFieldsToUpdate;
}

} // end namespace caf
