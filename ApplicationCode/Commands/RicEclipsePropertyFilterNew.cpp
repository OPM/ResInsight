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

#include "RicEclipsePropertyFilterNew.h"

#include "RicEclipsePropertyFilterNewExec.h"
#include "RicEclipsePropertyFilter.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicEclipsePropertyFilterNew, "RicEclipsePropertyFilterNew");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterNew::isCommandEnabled()
{
    std::vector<RimEclipsePropertyFilterCollection*> filterCollections = RicEclipsePropertyFilter::selectedPropertyFilterCollections();
    return filterCollections.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNew::onActionTriggered(bool isChecked)
{
    std::vector<RimEclipsePropertyFilterCollection*> filterCollections = RicEclipsePropertyFilter::selectedPropertyFilterCollections();
    if (filterCollections.size() == 1)
    {
        RicEclipsePropertyFilterNewExec* filterExec = new RicEclipsePropertyFilterNewExec(filterCollections[0]);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNew::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CellFilter_Values.png"));
    actionToSetup->setText("New Property Filter");
}
