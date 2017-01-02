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

#include "RicFracturesDeleteAllFeature.h"

#include "RimFractureCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

namespace caf
{

CAF_CMD_SOURCE_INIT(RicFracturesDeleteAllFeature, "RicFracturesDeleteAllFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicFracturesDeleteAllFeature::isCommandEnabled()
{
    std::vector<RimFractureCollection*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() == 1)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFracturesDeleteAllFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimFractureCollection*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    RimFractureCollection* fractureCollection = nullptr;
    if (objects.size() > 0)
    {
        fractureCollection = objects[0];
    }

    fractureCollection->deleteFractures();
    
    fractureCollection->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFracturesDeleteAllFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete All Fractures");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

} // end namespace caf
