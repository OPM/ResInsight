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

#include "RicGeoMechPropertyFilterInsert.h"

#include "RicGeoMechPropertyFilterInsertExec.h"
#include "RicGeoMechPropertyFilter.h"

#include "RimGeoMechPropertyFilter.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT(RicGeoMechPropertyFilterInsert, "RicGeoMechPropertyFilterInsert");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicGeoMechPropertyFilterInsert::isCommandEnabled()
{
    std::vector<RimGeoMechPropertyFilter*> propertyFilters = RicGeoMechPropertyFilter::selectedPropertyFilters();
    return propertyFilters.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterInsert::onActionTriggered(bool isChecked)
{
    std::vector<RimGeoMechPropertyFilter*> propertyFilters = RicGeoMechPropertyFilter::selectedPropertyFilters();
    if (propertyFilters.size() == 1)
    {
        RicGeoMechPropertyFilterInsertExec* filterExec = new RicGeoMechPropertyFilterInsertExec(propertyFilters[0]);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterInsert::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Insert Property Filter");
}
