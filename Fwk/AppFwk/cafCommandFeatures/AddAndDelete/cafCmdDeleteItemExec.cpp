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


#include "cafCmdDeleteItemExec.h"
#include "cafCmdDeleteItemExecData.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiFieldHandle.h"

#include "cafNotificationCenter.h"
#include "cafSelectionManager.h"


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString CmdDeleteItemExec::name()
{
    return m_commandData->classKeyword();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdDeleteItemExec::redo()
{
    PdmFieldHandle* field = PdmReferenceHelper::fieldFromReference(m_commandData->m_rootObject, m_commandData->m_pathToField);

    PdmChildArrayFieldHandle* listField = dynamic_cast<PdmChildArrayFieldHandle*>(field);
    if (listField)
    {
        std::vector<PdmObjectHandle*> children;
        listField->childObjects(&children);

        PdmObjectHandle* obj = children[m_commandData->m_indexToObject];
        caf::SelectionManager::instance()->removeObjectFromAllSelections(obj);

        if (m_commandData->m_deletedObjectAsXml().isEmpty())
        {
            QString encodedXml;
            {
                m_commandData->m_deletedObjectAsXml = xmlObj(obj)->writeObjectToXmlString();
            }
        }

        listField->erase(m_commandData->m_indexToObject);

        
        // TODO: The notification here could possibly be changed to 
        // PdmUiFieldHandle::notifyDataChange() similar to void CmdFieldChangeExec::redo()

        caf::PdmUiObjectHandle* ownerUiObject = uiObj(listField->ownerObject());
        if (ownerUiObject)
        {
            ownerUiObject->fieldChangedByUi(field, QVariant(), QVariant());
        }

        listField->uiCapability()->updateConnectedEditors();

        if (m_notificationCenter) m_notificationCenter->notifyObservers();

        delete obj;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdDeleteItemExec::undo()
{
    PdmFieldHandle* field = PdmReferenceHelper::fieldFromReference(m_commandData->m_rootObject, m_commandData->m_pathToField);

    PdmChildArrayFieldHandle* listField = dynamic_cast<PdmChildArrayFieldHandle*>(field);
    if (listField)
    {
        PdmObjectHandle* obj = PdmXmlObjectHandle::readUnknownObjectFromXmlString(m_commandData->m_deletedObjectAsXml(), PdmDefaultObjectFactory::instance(), false);

        listField->insertAt(m_commandData->m_indexToObject, obj);

        // TODO: The notification here could possibly be changed to 
        // PdmUiFieldHandle::notifyDataChange() similar to void CmdFieldChangeExec::redo()

        caf::PdmUiObjectHandle* ownerUiObject = uiObj(listField->ownerObject());
        if (ownerUiObject)
        {
            ownerUiObject->fieldChangedByUi(field, QVariant(), QVariant());
        }

        listField->uiCapability()->updateConnectedEditors();

        if (m_notificationCenter) m_notificationCenter->notifyObservers();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdDeleteItemExec::CmdDeleteItemExec(NotificationCenter* notificationCenter)
    : CmdExecuteCommand(notificationCenter)
{
    m_commandData = new CmdDeleteItemExecData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdDeleteItemExecData* CmdDeleteItemExec::commandData()
{
    return m_commandData;
}

} // end namespace caf
