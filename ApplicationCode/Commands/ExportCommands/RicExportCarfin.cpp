/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicExportCarfin.h"

#include "RimCase.h"

#include "cafSelectionManager.h"

#include <QAction>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportCarfin::isCommandEnabled()
{
    if (RicExportCarfin::selectedCase() != nullptr)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfin::onActionTriggered(bool isChecked)
{
    RimCase* rimCase = RicExportCarfin::selectedCase();
    CVF_ASSERT(rimCase);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfin::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export CARFIN ...");
    actionToSetup->setIcon(QIcon(":/Save.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* RicExportCarfin::selectedCase()
{
    std::vector<RimCase*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType(&selectedObjects);

    if (selectedObjects.size() == 1)
    {
        return selectedObjects[0];
    }

    return nullptr;
}
