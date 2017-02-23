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

#include "RicSimWellFracturesDeleteAllFeature.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimSimWellFractureCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

namespace caf
{

CAF_CMD_SOURCE_INIT(RicSimWellFracturesDeleteAllFeature, "RicSimWellFracturesDeleteAllFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSimWellFracturesDeleteAllFeature::isCommandEnabled()
{
    std::vector<RimSimWellFractureCollection*> objects;
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
void RicSimWellFracturesDeleteAllFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellFractureCollection*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    RimSimWellFractureCollection* fractureCollection = nullptr;
    if (objects.size() > 0)
    {
        fractureCollection = objects[0];
        fractureCollection->deleteFractures();

        RimEclipseWell* eclipseWell = nullptr;
        fractureCollection->firstAncestorOrThisOfType(eclipseWell);
        if (eclipseWell) eclipseWell->updateConnectedEditors();
        
        RimEclipseView* mainView = nullptr;
        fractureCollection->firstAncestorOrThisOfType(mainView);
        if (mainView) mainView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSimWellFracturesDeleteAllFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete All Fractures");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

} // end namespace caf
