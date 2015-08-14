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

#include "RicRangeFilterNew.h"

#include "RicRangeFilterHelper.h"
#include "RicRangeFilterNewExec.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafCmdFeatureManager.h"
#include "cafCmdExecCommandManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicRangeFilterNew, "RicRangeFilterNew");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterNew::isCommandEnabled()
{
    return RicRangeFilterHelper::isRangeFilterCommandAvailable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNew::onActionTriggered(bool isChecked)
{
    RicRangeFilterNewExec* filterExec = RicRangeFilterHelper::createRangeFilterExecCommand();

    caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNew::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CellFilter_Range.png"));
    actionToSetup->setText("New Range Filter");
}

