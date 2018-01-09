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

#include "RicSaveEclipseResultAsInputPropertyFeature.h"

#include "RicSaveEclipseResultAsInputPropertyExec.h"

#include "RimEclipseCellColors.h"
#include "Rim3dView.h"

#include "cafSelectionManager.h"
#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicSaveEclipseResultAsInputPropertyFeature, "RicSaveEclipseResultAsInputPropertyFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSaveEclipseResultAsInputPropertyFeature::isCommandEnabled()
{
    std::vector<RimEclipseCellColors*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return selection.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputPropertyFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

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
void RicSaveEclipseResultAsInputPropertyFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Property To File");
}


