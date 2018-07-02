/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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
#include "RicNewWellPathListTargetFeature.h"

CAF_CMD_SOURCE_INIT(RicNewWellPathListTargetFeature, "RicNewWellPathListTargetFeature");

#include "cafSelectionManager.h"
#include "RimWellPathTarget.h"
#include "RimModeledWellPath.h"
#include <QAction>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathListTargetFeature::isCommandEnabled()
{
    {
        std::vector<RimWellPathGeometryDef*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if ( objects.size() > 0 )
        {
            return true;
        }
    }
    {
        std::vector<RimWellPathTarget*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects, caf::SelectionManager::CURRENT);

        if ( objects.size() > 0 )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathListTargetFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPathTarget*> targets;
    caf::SelectionManager::instance()->objectsByType(&targets, caf::SelectionManager::CURRENT);
    if (targets.size() > 0)
    {
        RimWellPathGeometryDef* wellGeomDef = nullptr;
        targets[0]->firstAncestorOrThisOfTypeAsserted(wellGeomDef);
        
        wellGeomDef->insertTarget(targets[0], new RimWellPathTarget);
        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization();
        return;
    }

    std::vector<RimWellPathGeometryDef*> geomDefs;
    caf::SelectionManager::instance()->objectsByType(&geomDefs);
    if (geomDefs.size() > 0)
    {
        RimWellPathGeometryDef* wellGeomDef = geomDefs[0];

        wellGeomDef->insertTarget(nullptr, new RimWellPathTarget);
        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathListTargetFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Target");
    actionToSetup->setIcon(QIcon(":/Well.png"));
}


