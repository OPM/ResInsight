//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafCmdAddItemExec.h"

#include "cafCmdAddItemExecData.h"

#include "cafNotificationCenter.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

#include "cafPdmChildArrayField.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString CmdAddItemExec::name()
{
    PdmFieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );

    QString containedObjectType = "object";

    PdmChildArrayFieldHandle* listField = dynamic_cast<PdmChildArrayFieldHandle*>( field );
    if ( listField )
    {
        PdmXmlFieldHandle* xfh = listField->xmlCapability();
        containedObjectType    = xfh->dataTypeName();
    }

    return QString( "Create new '%1'" ).arg( containedObjectType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdAddItemExec::redo()
{
    PdmFieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );

    PdmChildArrayFieldHandle* listField = dynamic_cast<PdmChildArrayFieldHandle*>( field );
    if ( listField && field->xmlCapability() )
    {
        QString classKeyword = field->xmlCapability()->dataTypeName();

        if ( classKeyword.isEmpty() ) return;

        caf::PdmObjectHandle* obj = PdmDefaultObjectFactory::instance()->create( classKeyword );

        if ( !obj ) return;

        listField->insertAt( m_commandData->m_indexAfter, obj );

        if ( m_commandData->m_indexAfter == -1 )
        {
            m_commandData->m_createdItemIndex = static_cast<int>( listField->size() - 1 );
        }
        else
        {
            m_commandData->m_createdItemIndex = m_commandData->m_indexAfter;
        }

        if ( m_notificationCenter ) m_notificationCenter->notifyObserversOfDataChange( obj );

        listField->uiCapability()->updateConnectedEditors();

        if ( listField->ownerObject() )
        {
            caf::PdmUiObjectHandle* ownerUiObject = uiObj( listField->ownerObject() );
            if ( ownerUiObject )
            {
                ownerUiObject->fieldChangedByUi( listField, QVariant(), QVariant() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdAddItemExec::undo()
{
    PdmFieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );

    PdmChildArrayFieldHandle* listField = dynamic_cast<PdmChildArrayFieldHandle*>( field );
    if ( listField && m_commandData->m_createdItemIndex >= 0 )
    {
        std::vector<caf::PdmObjectHandle*> children;
        listField->childObjects( &children );

        caf::PdmObjectHandle* obj = children[m_commandData->m_createdItemIndex];

        caf::SelectionManager::instance()->removeObjectFromAllSelections( obj );

        listField->erase( m_commandData->m_createdItemIndex );
        listField->uiCapability()->updateConnectedEditors();

        if ( m_notificationCenter ) m_notificationCenter->notifyObservers();

        if ( listField->ownerObject() )
        {
            caf::PdmUiObjectHandle* ownerUiObject = uiObj( listField->ownerObject() );
            if ( ownerUiObject )
            {
                ownerUiObject->fieldChangedByUi( listField, QVariant(), QVariant() );
            }
        }

        delete obj;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdAddItemExec::CmdAddItemExec( NotificationCenter* notificationCenter )
    : CmdExecuteCommand( notificationCenter )
{
    m_commandData = new CmdAddItemExecData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdAddItemExec::~CmdAddItemExec()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdAddItemExecData* CmdAddItemExec::commandData()
{
    return m_commandData;
}

} // end namespace caf
