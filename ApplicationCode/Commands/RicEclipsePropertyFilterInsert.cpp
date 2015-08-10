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

#include "RicEclipsePropertyFilterInsert.h"

#include "RicEclipsePropertyFilterInsertExec.h"
#include "RicEclipsePropertyFilter.h"

#include "RimEclipsePropertyFilter.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT(RicEclipsePropertyFilterInsert, "RicEclipsePropertyFilterInsert");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterInsert::isCommandEnabled()
{
    std::vector<RimEclipsePropertyFilter*> propertyFilters = RicEclipsePropertyFilter::selectedPropertyFilters();
    return propertyFilters.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterInsert::onActionTriggered(bool isChecked)
{
    std::vector<RimEclipsePropertyFilter*> propertyFilters = RicEclipsePropertyFilter::selectedPropertyFilters();
    if (propertyFilters.size() == 1)
    {
        RicEclipsePropertyFilterInsertExec* filterExec = new RicEclipsePropertyFilterInsertExec(propertyFilters[0]);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterInsert::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Insert Property Filter");
}
