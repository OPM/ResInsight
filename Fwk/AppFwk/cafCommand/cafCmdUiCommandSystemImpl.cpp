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


#include "cafCmdUiCommandSystemImpl.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdExecuteCommand.h"
#include "cafCmdFieldChangeExec.h"
#include "cafCmdFeatureManager.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmUiObjectHandle.h"

#include "cafSelectionManager.h"

#include <QMenu>

#include <vector>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdUiCommandSystemImpl::CmdUiCommandSystemImpl()
{
    m_undoFeatureEnabled = false;
    m_disableUndoForFieldChange = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::fieldChangedCommand(PdmFieldHandle* editorField, const QVariant& newUiValue)
{
    std::vector<PdmFieldHandle*> fieldsToUpdate;
    fieldsToUpdate.push_back(editorField);

    // For current selection, find all fields with same keyword
    {
        std::vector<PdmUiItem*> items;
        SelectionManager::instance()->selectedItems(items, SelectionManager::CURRENT);

        for (size_t i = 0; i < items.size(); i++)
        {
            PdmObjectHandle* objectHandle = dynamic_cast<PdmObjectHandle*>(items[i]);
            if (objectHandle)
            {
                // An object is selected, find field with same keyword as the current field being edited
                PdmFieldHandle* fieldHandle = objectHandle->findField(editorField->keyword());
                if (fieldHandle && fieldHandle != editorField)
                {
                    fieldsToUpdate.push_back(fieldHandle);
                }
            }
            else
            {
                // A field is selected, check if keywords are identical
                PdmUiFieldHandle* uiFieldHandle = dynamic_cast<PdmUiFieldHandle*>(items[i]);
                if (uiFieldHandle)
                {
                    PdmFieldHandle* field = uiFieldHandle->fieldHandle();
                    if (field && field != editorField && field->keyword() == editorField->keyword())
                    {
                        fieldsToUpdate.push_back(field);
                    }
                }
            }
        }
    }

    std::vector<CmdExecuteCommand*> commands;

    for (size_t i = 0; i < fieldsToUpdate.size(); i++)
    {
        PdmFieldHandle* field = fieldsToUpdate[i];
        PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if (uiFieldHandle)
        {
            QVariant fieldCurrentUiValue = uiFieldHandle->uiValue();

            if (fieldCurrentUiValue != newUiValue)
            {
                PdmObjectHandle* rootObjHandle = PdmReferenceHelper::findRoot(field);

                QString reference = PdmReferenceHelper::referenceFromRootToField(rootObjHandle, field);
                if (reference.isEmpty())
                {
                    CAF_ASSERT(false);
                    return;
                }

                CmdFieldChangeExec* fieldChangeExec = new CmdFieldChangeExec(SelectionManager::instance()->notificationCenter());

                fieldChangeExec->commandData()->m_newUiValue = newUiValue;
                fieldChangeExec->commandData()->m_pathToField = reference;
                fieldChangeExec->commandData()->m_rootObject = rootObjHandle;

                commands.push_back(fieldChangeExec);
            }
        }
    }

    caf::PdmUiObjectHandle* uiOwnerObjectHandle = uiObj(editorField->ownerObject());
    if (uiOwnerObjectHandle && !uiOwnerObjectHandle->useUndoRedoForFieldChanged())
    {
        // Temporarily disable undo framework as requested by the PdmUiObjectHandle
        m_disableUndoForFieldChange = true;
    }

    if (commands.size() == 1)
    {
        CmdExecCommandManager::instance()->processExecuteCommand(commands[0]);
    }
    else
    {
        CmdExecCommandManager::instance()->processExecuteCommandsAsMacro("Multiple Field Change", commands);
    }

    if (uiOwnerObjectHandle && !uiOwnerObjectHandle->useUndoRedoForFieldChanged())
    {
        // Restore undo feature to normal operation
        m_disableUndoForFieldChange = false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::populateMenuWithDefaultCommands(const QString& uiConfigName, QMenu* menu)
{
    if (uiConfigName == "PdmUiTreeViewEditor" ||
        uiConfigName == "PdmUiTableViewEditor")
    {
        caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();

        menu->addAction(commandManager->action("PdmListField_AddItem"));
        menu->addAction(commandManager->action("PdmListField_DeleteItem"));

        QStringList commandIdList;
        commandIdList << "PdmListField_AddItem" << "PdmListField_DeleteItem";
        commandManager->refreshStates(commandIdList);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CmdUiCommandSystemImpl::isUndoEnabled()
{
    return m_undoFeatureEnabled;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::enableUndoFeature(bool enable)
{
    m_undoFeatureEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CmdUiCommandSystemImpl::disableUndoForFieldChange()
{
    return m_disableUndoForFieldChange;
}

} // end namespace caf
