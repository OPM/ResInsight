/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RicDeletePolylineTargetFeature.h"

CAF_CMD_SOURCE_INIT(RicDeletePolylineTargetFeature, "RicDeletePolylineTargetFeature");

#include "RimUserDefinedPolylinesAnnotation.h"
#include "RimPolylineTarget.h"
#include "cafSelectionManager.h"
#include <QAction>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeletePolylineTargetFeature::isCommandEnabled()
{
    std::vector<RimPolylineTarget*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects, caf::SelectionManager::FIRST_LEVEL);

    if ( !objects.empty() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeletePolylineTargetFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimPolylineTarget*> targets;
    caf::SelectionManager::instance()->objectsByType(&targets, caf::SelectionManager::FIRST_LEVEL);
    
    if (!targets.empty())
    {

        RimUserDefinedPolylinesAnnotation* polylineDef = nullptr;
        targets[0]->firstAncestorOrThisOfTypeAsserted(polylineDef);

        for ( auto target: targets )
        {
            polylineDef->deleteTarget(target);
        }
        
        polylineDef->updateConnectedEditors();
        polylineDef->updateVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeletePolylineTargetFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Target");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}


