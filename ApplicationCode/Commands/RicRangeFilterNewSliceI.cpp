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

#include "RicRangeFilterNewSliceI.h"

#include "RicRangeFilterNewExec.h"
#include "RicRangeFilterHelper.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicRangeFilterNewSliceI, "RicRangeFilterNewSliceI");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterNewSliceI::isCommandEnabled()
{
    return RicRangeFilterHelper::isRangeFilterCommandAvailable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewSliceI::onActionTriggered(bool isChecked)
{
    RicRangeFilterNewExec* filterExec = RicRangeFilterHelper::createRangeFilterExecCommand();
    filterExec->m_iSlice = true;

    caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewSliceI::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New I-slice range filter");
}

