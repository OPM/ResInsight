/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicApplyPropertyFilterAsCellResultFeature.h"

#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseView.h"

#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicApplyPropertyFilterAsCellResultFeature, "RicApplyPropertyFilterAsCellResultFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicApplyPropertyFilterAsCellResultFeature::isCommandEnabled()
{
    {
        std::vector<RimEclipsePropertyFilter*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if (objects.size() == 1)
        {
            return true;
        }
    }

    {
        std::vector<RimGeoMechPropertyFilter*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if (objects.size() == 1)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicApplyPropertyFilterAsCellResultFeature::onActionTriggered(bool isChecked)
{
    {
        std::vector<RimEclipsePropertyFilter*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if (objects.size() == 1)
        {
            RimEclipsePropertyFilter* propertyFilter = objects[0];
            if (!propertyFilter) return;

            RimEclipseView* rimEclipseView = nullptr;
            propertyFilter->firstAncestorOrThisOfType(rimEclipseView);
            if (!rimEclipseView) return;

            rimEclipseView->cellResult()->simpleCopy(propertyFilter->resultDefinition());
            rimEclipseView->cellResult()->updateConnectedEditors();

            rimEclipseView->scheduleCreateDisplayModelAndRedraw();

            return;
        }
    }

    {
        std::vector<RimGeoMechPropertyFilter*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if (objects.size() == 1)
        {
            RimGeoMechPropertyFilter* propertyFilter = objects[0];
            if (!propertyFilter) return;

            RimGeoMechView* geoMechView = nullptr;
            propertyFilter->firstAncestorOrThisOfType(geoMechView);
            if (!geoMechView) return;

            geoMechView->cellResultResultDefinition()->setResultAddress(propertyFilter->resultDefinition()->resultAddress());
            geoMechView->cellResultResultDefinition()->updateConnectedEditors();

            geoMechView->scheduleCreateDisplayModelAndRedraw();

            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicApplyPropertyFilterAsCellResultFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Apply As Cell Result");
    actionToSetup->setIcon(QIcon(":/CellResult.png"));
}

