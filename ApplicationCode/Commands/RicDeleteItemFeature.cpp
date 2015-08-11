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

#include "RicDeleteItemFeature.h"
#include "RicDeleteItemExec.h"
#include "RicDeleteItemExecData.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdSelectionHelper.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

#include <QAction>

namespace caf
{
    CAF_CMD_SOURCE_INIT(RicDeleteItemFeature, "RicDeleteItemFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeleteItemFeature::isCommandEnabled() 
{
    caf::PdmObject* currentPdmObject = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (!currentPdmObject) return false;

    caf::PdmChildArrayFieldHandle* childArrayFieldHandle = dynamic_cast<caf::PdmChildArrayFieldHandle*>(currentPdmObject->parentField());
    if (!childArrayFieldHandle) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::onActionTriggered(bool isChecked)
{
    std::vector<PdmUiItem*> items;
    SelectionManager::instance()->selectedItems(items);
    assert(items.size() > 0);

    caf::PdmObject* currentPdmObject = dynamic_cast<caf::PdmObject*>(items[0]);
    assert(currentPdmObject);

    caf::PdmChildArrayFieldHandle* childArrayFieldHandle = dynamic_cast<caf::PdmChildArrayFieldHandle*>(currentPdmObject->parentField());

    int indexAfter = -1;

    std::vector<PdmObjectHandle*> childObjects;
    childArrayFieldHandle->childObjects(&childObjects);

    for (size_t i = 0; i < childObjects.size(); i++)
    {
        if (childObjects[i] == currentPdmObject)
        {
            indexAfter = static_cast<int>(i);
        }
    }

    // Did not find currently selected pdm object in the current list field
    assert(indexAfter != -1);

    RicDeleteItemExec* executeCmd = new RicDeleteItemExec(SelectionManager::instance()->notificationCenter());

    RicDeleteItemExecData* data = executeCmd->commandData();
    data->m_rootObject = PdmReferenceHelper::findRoot(childArrayFieldHandle);
    data->m_pathToField = PdmReferenceHelper::referenceFromRootToField(data->m_rootObject, childArrayFieldHandle);
    data->m_indexToObject = indexAfter;


    CmdExecCommandManager::instance()->processExecuteCommand(executeCmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete object");
}

} // end namespace caf
