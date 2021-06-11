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

#include "RicDeleteItemExec.h"
#include "RicDeleteItemExecData.h"

#include "cafNotificationCenter.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmDocument.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiFieldHandle.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicDeleteItemExec::name()
{
    if ( !m_commandData.m_description().isEmpty() )
    {
        return m_commandData.m_description();
    }

    return m_commandData.classKeyword();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteItemExec::redo()
{
    caf::PdmFieldHandle* field =
        caf::PdmReferenceHelper::fieldFromReference( m_commandData.m_rootObject, m_commandData.m_pathToField );

    caf::PdmChildArrayFieldHandle* listField = dynamic_cast<caf::PdmChildArrayFieldHandle*>( field );
    if ( listField )
    {
        std::vector<caf::PdmObjectHandle*> children;
        listField->childObjects( &children );

        caf::PdmObjectHandle* obj = children[m_commandData.m_indexToObject];
        caf::SelectionManager::instance()->removeObjectFromAllSelections( obj );

        std::vector<caf::PdmObjectHandle*> referringObjects;
        obj->objectsWithReferringPtrFields( referringObjects );

        if ( m_commandData.m_deletedObjectAsXml().isEmpty() )
        {
            m_commandData.m_deletedObjectAsXml = xmlObj( obj )->writeObjectToXmlString();
        }

        delete obj;

        listField->erase( m_commandData.m_indexToObject );

        caf::PdmObjectHandle* parentObj = listField->ownerObject();
        parentObj->uiCapability()->updateConnectedEditors();
        parentObj->onChildDeleted( listField, referringObjects );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteItemExec::undo()
{
    caf::PdmFieldHandle* field =
        caf::PdmReferenceHelper::fieldFromReference( m_commandData.m_rootObject, m_commandData.m_pathToField );

    caf::PdmChildArrayFieldHandle* listField = dynamic_cast<caf::PdmChildArrayFieldHandle*>( field );
    if ( listField )
    {
        caf::PdmObjectHandle* obj =
            caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( m_commandData.m_deletedObjectAsXml(),
                                                                     caf::PdmDefaultObjectFactory::instance(),
                                                                     false );

        listField->insertAt( m_commandData.m_indexToObject, obj );

        obj->xmlCapability()->initAfterReadRecursively();
        obj->xmlCapability()->resolveReferencesRecursively();

        listField->uiCapability()->updateConnectedEditors();
        listField->ownerObject()->uiCapability()->updateConnectedEditors();

        caf::PdmObjectHandle* parentObj = listField->ownerObject();
        if ( parentObj )
        {
            std::vector<caf::PdmObjectHandle*> referringObjects;

            // TODO: Here we need a different concept like onChildAdded()
            parentObj->onChildDeleted( listField, referringObjects );
        }

        if ( m_notificationCenter ) m_notificationCenter->notifyObservers();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicDeleteItemExec::RicDeleteItemExec( caf::NotificationCenter* notificationCenter )
    : CmdExecuteCommand( notificationCenter )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicDeleteItemExecData& RicDeleteItemExec::commandData()
{
    return m_commandData;
}
