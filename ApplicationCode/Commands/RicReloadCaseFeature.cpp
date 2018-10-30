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

#include "RicReloadCaseFeature.h"

#include "RiaApplication.h"

#include "RimEclipseCase.h"

#include "RiuSelectionManager.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicReloadCaseFeature, "RicReloadCaseFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadCaseFeature::isCommandEnabled()
{
    std::vector<caf::PdmObject*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesCollObjs);
    for (caf::PdmObject* pdmObject : selectedFormationNamesCollObjs)
    {
        if (dynamic_cast<RimEclipseCase*>(pdmObject))
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadCaseFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimEclipseCase*> selectedEclipseCases;
    caf::SelectionManager::instance()->objectsByType(&selectedEclipseCases);

    RiaApplication::clearAllSelections();

    for (RimEclipseCase* selectedCase : selectedEclipseCases)
    {
        selectedCase->reloadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Reload");
    actionToSetup->setIcon(QIcon(":/Refresh-32.png"));
}
