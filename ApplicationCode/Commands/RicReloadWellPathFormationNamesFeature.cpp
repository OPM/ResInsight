/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicReloadWellPathFormationNamesFeature.h"

#include "RiaApplication.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicReloadWellPathFormationNamesFeature, "RicReloadWellPathFormationNamesFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadWellPathFormationNamesFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType(&wellPaths);

    std::vector<RimWellPathCollection*> wellPathCollection;
    caf::SelectionManager::instance()->objectsByType(&wellPathCollection);

    return (wellPaths.size() > 0 || wellPathCollection.size() > 0);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadWellPathFormationNamesFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType(&wellPaths);

    std::vector<RimWellPathCollection*> wellPathCollections;
    caf::SelectionManager::instance()->objectsByType(&wellPathCollections);

    if (wellPaths.size() > 0)
    {
        RimWellPathCollection* wellPathCollection;
        wellPaths[0]->firstAncestorOrThisOfTypeAsserted(wellPathCollection);
        wellPathCollection->reloadAllWellPathFormations();
    }
    else if (wellPathCollections.size() > 0) 
    {
        wellPathCollections[0]->reloadAllWellPathFormations(); 
    }

    RimProject* project = RiaApplication::instance()->project();
    if (project)
    {
        if (project->mainPlotCollection())
        {
            project->mainPlotCollection->updatePlotsWithFormations();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadWellPathFormationNamesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Reload All Well Picks");
    actionToSetup->setIcon(QIcon(":/Refresh-32.png"));
}
