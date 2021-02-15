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
        PdmFieldHandle*       editorField      = uiFieldHandle->fieldHandle();
        const std::type_info& fieldOwnerTypeId = typeid( *editorField->ownerObject() );

        std::vector<PdmFieldHandle*> fieldsToUpdate;
        fieldsToUpdate.push_back( editorField );

        std::vector<PdmFieldHandle*> otherSelectedFields = fieldsFromSelection( fieldOwnerTypeId, editorField->keyword() );

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
            m_commandInterface->fieldChangedCommand( fieldsToUpdate, newUiValue );
        }
        else
        {
            for ( auto fieldHandle : fieldsToUpdate )
            {
                fieldHandle->uiCapability()->setValueFromUiEditor( newUiValue );
            }
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
std::vector<PdmFieldHandle*> PdmUiCommandSystemProxy::fieldsFromSelection( const std::type_info& fieldOwnerTypeId,
                                                                           const QString&        fieldKeyword )
{
    std::vector<PdmUiItem*>      items;
    std::vector<PdmFieldHandle*> additionalFieldsToUpdate;

    int selectionLevel = 0;
    SelectionManager::instance()->selectedItems( items, selectionLevel );

    for ( auto& item : items )
    {
        PdmObjectHandle* objectHandle = dynamic_cast<PdmObjectHandle*>( item );
        if ( objectHandle && typeid( *objectHandle ) == fieldOwnerTypeId )
        {
            // An object is selected, find field with same keyword as the current field being edited
            PdmFieldHandle* fieldHandle = objectHandle->findField( fieldKeyword );
            if ( fieldHandle )
            {
                additionalFieldsToUpdate.push_back( fieldHandle );
            }
        }
        else
        {
            // Todo Remove when dust has settled. Selection manager is not supposed to select single fields
            // A field is selected, check if keywords are identical
            PdmUiFieldHandle* itemFieldHandle = dynamic_cast<PdmUiFieldHandle*>( item );
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
