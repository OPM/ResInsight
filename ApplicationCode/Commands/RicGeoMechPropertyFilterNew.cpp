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

#include "RicGeoMechPropertyFilterNew.h"

#include "RicGeoMechPropertyFilterNewExec.h"
#include "RicGeoMechPropertyFilter.h"
 
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicGeoMechPropertyFilterNew, "RicGeoMechPropertyFilterNew");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicGeoMechPropertyFilterNew::isCommandEnabled()
{
    std::vector<RimGeoMechPropertyFilterCollection*> filterCollections = RicGeoMechPropertyFilter::selectedPropertyFilterCollections();
    return filterCollections.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNew::onActionTriggered(bool isChecked)
{
    std::vector<RimGeoMechPropertyFilterCollection*> filterCollections = RicGeoMechPropertyFilter::selectedPropertyFilterCollections();
    if (filterCollections.size() == 1)
    {
        RicGeoMechPropertyFilterNewExec* filterExec = new RicGeoMechPropertyFilterNewExec(filterCollections[0]);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNew::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CellFilter_Values.png"));
    actionToSetup->setText("New Property Filter");
}
