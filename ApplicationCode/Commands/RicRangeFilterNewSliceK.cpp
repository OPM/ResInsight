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

#include "RicRangeFilterNewSliceK.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>
#include "RicRangeFilterNewExec.h"
#include "cafCmdExecCommandManager.h"

CAF_CMD_SOURCE_INIT(RicRangeFilterNewSliceK, "RicRangeFilterNewSliceK");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterNewSliceK::isCommandEnabled()
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
void RicRangeFilterNewSliceK::onActionTriggered(bool isChecked)
{
    std::vector<RimCellRangeFilterCollection*> selectedRangeFilterCollection;
    caf::SelectionManager::instance()->objectsByType(&selectedRangeFilterCollection);

    if (selectedRangeFilterCollection.size() == 1)
    {
        RimCellRangeFilterCollection* rangeFilterCollection = selectedRangeFilterCollection[0];

        RicRangeFilterNewExec* filterExec = new RicRangeFilterNewExec(rangeFilterCollection);
        filterExec->m_kSlice = true;

        caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewSliceK::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New K-slice range filter");
}
