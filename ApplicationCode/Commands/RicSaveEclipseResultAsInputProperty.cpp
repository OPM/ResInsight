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

#include "RicSaveEclipseResultAsInputProperty.h"

#include "RicSaveEclipseResultAsInputPropertyExec.h"

#include "RimEclipseCellColors.h"
#include "RimView.h"

#include "cafSelectionManager.h"
#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicSaveEclipseResultAsInputProperty, "RicEclipseCellResultSave");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSaveEclipseResultAsInputProperty::isCommandEnabled()
{
    std::vector<RimEclipseCellColors*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return selection.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputProperty::onActionTriggered(bool isChecked)
{
    std::vector<RimEclipseCellColors*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    if (selection.size() == 1)
    {
        RicSaveEclipseResultAsInputPropertyExec* cellResultSaveExec = new RicSaveEclipseResultAsInputPropertyExec(selection[0]);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(cellResultSaveExec);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputProperty::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Save Property To File");
}


