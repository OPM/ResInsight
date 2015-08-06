/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2014 Ceetron Solutions AS, USFOS AS, AMOS - NTNU
// 
//  RPM is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  RPM is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicRangeFilterNew.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseView.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicRangeFilterNew, "RicRangeFilterNew");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterNew::isCommandEnabled()
{
    std::vector<RimCellRangeFilter*> selectedRangeFilter;
    caf::SelectionManager::instance()->objectsByType(&selectedRangeFilter);

    std::vector<RimCellRangeFilterCollection*> selectedRangeFilterCollection;
    caf::SelectionManager::instance()->objectsByType(&selectedRangeFilterCollection);

    if (selectedRangeFilter.size() > 0 || selectedRangeFilterCollection.size() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNew::onActionTriggered(bool isChecked)
{
    std::vector<RimCellRangeFilterCollection*> selectedRangeFilterCollection;
    caf::SelectionManager::instance()->objectsByType(&selectedRangeFilterCollection);

    if (selectedRangeFilterCollection.size() == 1)
    {
//         selectedRangeFilterCollection[0]->rangeFilters.push_back(new RimCellRangeFilter);
//         selectedRangeFilterCollection[0]->reservoirView()->createDisplayModelAndRedraw();
    }



/*
    EvmProject* proj = EvaApplication::instance()->project();

    std::vector<EvmNode*> selectedNodes;
    caf::SelectionManager::instance()->objectsByType(&selectedNodes, caf::SelectionManager::CURRENT);

    caf::CmdAddItemFeature* addItemFeature = dynamic_cast<caf::CmdAddItemFeature*>(caf::CmdFeatureManager::instance()->getCommandFeature(caf::CmdAddItemFeature::idNameStatic()));
    assert(addItemFeature);

    if (selectedNodes.size() > 0)
    {
        caf::SelectionManager::instance()->setActiveChildArrayFieldHandle(&proj->boundaryConditions);

        std::vector<caf::CmdExecuteCommand*> commands;
        for (size_t i = 0; i < selectedNodes.size(); i++)
        {
            // Not allowed to add more than one BC
            if (proj->findBoundaryConditionForNode(selectedNodes[i]->id)) continue;

            std::vector<caf::PdmObjectHandle*> newSelection;
            newSelection.push_back(selectedNodes[i]);
            caf::CmdSelectionChangeExec* selectionChangeExec = caf::CmdSelectionHelper::createSelectionCommand(newSelection, caf::SelectionManager::CURRENT);
            commands.push_back(selectionChangeExec);

            int indexAfter = -1;
            caf::CmdAddItemExec* addItemExec = new caf::CmdAddItemExec(caf::SelectionManager::instance()->notificationCenter());

            caf::CmdAddItemExecData* data = addItemExec->commandData();
            data->m_rootObject = caf::PdmReferenceHelper::findRoot(&proj->nodalLoads);
            data->m_pathToField = caf::PdmReferenceHelper::referenceFromRootToField(data->m_rootObject, &proj->boundaryConditions);
            data->m_indexAfter = indexAfter;

            commands.push_back(addItemExec);
        }

        if (commands.size() > 0)
        {
            // Add an empty selection when boundary conditions has been assigned
            std::vector<caf::PdmObjectHandle*> newSelection;
            caf::CmdSelectionChangeExec* selectionChangeExec = caf::CmdSelectionHelper::createSelectionCommand(newSelection, caf::SelectionManager::CURRENT);
            commands.push_back(selectionChangeExec);
        }

        caf::CmdExecCommandManager::instance()->processExecuteCommandsAsMacro("New Boundary Conditions", commands);
    }
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNew::setupActionLook(QAction* actionToSetup)
{
//    actionToSetup->setIcon(QIcon(":/Constraint16x16.png"));
    actionToSetup->setText("New Range Filter");
}

