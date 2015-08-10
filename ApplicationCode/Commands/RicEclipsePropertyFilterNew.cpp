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
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "cafSelectionManager.h"
#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicEclipsePropertyFilterNew, "RicEclipsePropertyFilterNew");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterNew::isCommandEnabled()
{
    std::vector<RimEclipsePropertyFilter*> selectedPropertyFilter;
    caf::SelectionManager::instance()->objectsByType(&selectedPropertyFilter);

    std::vector<RimEclipsePropertyFilterCollection*> selectedPropertyFilterCollection;
    caf::SelectionManager::instance()->objectsByType(&selectedPropertyFilterCollection);

    if (selectedPropertyFilter.size() > 0 || selectedPropertyFilterCollection.size() > 0)
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
void RicEclipsePropertyFilterNew::onActionTriggered(bool isChecked)
{
    RimEclipsePropertyFilterCollection* propertyFilterCollection = NULL;

    std::vector<RimEclipsePropertyFilter*> selectedPropertyFilter;
    caf::SelectionManager::instance()->objectsByType(&selectedPropertyFilter);

    std::vector<RimEclipsePropertyFilterCollection*> selectedPropertyFilterCollection;
    caf::SelectionManager::instance()->objectsByType(&selectedPropertyFilterCollection);
    if (selectedPropertyFilterCollection.size() == 1)
    {
        propertyFilterCollection = selectedPropertyFilterCollection[0];
    }
    else if (selectedPropertyFilter.size() > 0)
    {
        propertyFilterCollection = dynamic_cast<RimEclipsePropertyFilterCollection*>(selectedPropertyFilter[0]->owner());
    }

    if (propertyFilterCollection)
    {
        RicEclipsePropertyFilterNewExec* filterExec = new RicEclipsePropertyFilterNewExec(propertyFilterCollection);
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
