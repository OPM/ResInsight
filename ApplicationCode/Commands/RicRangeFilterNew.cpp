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
#include "RicRangeFilterNewExec.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafSelectionManager.h"

#include "cafCmdFeatureManager.h"
#include "cafCmdExecCommandManager.h"

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
        RimCellRangeFilterCollection* rangeFilterCollection = selectedRangeFilterCollection[0];

        RicRangeFilterNewExec* filterExec = new RicRangeFilterNewExec(rangeFilterCollection);

        caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNew::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CellFilter_Range.png"));
    actionToSetup->setText("New Range Filter");
}

