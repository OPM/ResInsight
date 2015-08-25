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

#include "RicWellPathsDeleteAllFeature.h"

#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

namespace caf
{

CAF_CMD_SOURCE_INIT(RicWellPathsDeleteAllFeature, "RicWellPathsDeleteAllFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathsDeleteAllFeature::isCommandEnabled()
{
    std::vector<RimWellPathCollection*> objects;
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
void RicWellPathsDeleteAllFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPathCollection*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    RimWellPathCollection* wellPathCollection = objects[0];

    wellPathCollection->wellPaths.deleteAllChildObjects();

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleGeometryRegenAndRedrawViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathsDeleteAllFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete All Well Paths");
    //actionToSetup->setIcon(QIcon(":/WellCollection.png"));
}

} // end namespace caf
