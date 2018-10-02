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

#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"
#include "RimModeledWellPath.h"
#include "cafSelectionManager.h"
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
        caf::SelectionManager::instance()->objectsByType(&objects, caf::SelectionManager::FIRST_LEVEL);

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
    std::vector<RimWellPathTarget*> selectedTargets;
    caf::SelectionManager::instance()->objectsByType(&selectedTargets, caf::SelectionManager::FIRST_LEVEL);
    if (selectedTargets.size() > 0)
    {
        auto firstTarget = selectedTargets.front();
        RimWellPathGeometryDef* wellGeomDef = nullptr;
        firstTarget->firstAncestorOrThisOfTypeAsserted(wellGeomDef);
        
        auto afterBeforePair = wellGeomDef->findActiveTargetsAroundInsertionPoint(firstTarget);
        
        cvf::Vec3d newPos = cvf::Vec3d::ZERO;

        if (!afterBeforePair.first && afterBeforePair.second)
        {
            newPos = afterBeforePair.second->targetPointXYZ();
            newPos.z() = -wellGeomDef->referencePointXyz().z();
        } 
        else if (afterBeforePair.first && afterBeforePair.second)
        {
            newPos = 0.5*(afterBeforePair.first->targetPointXYZ() + afterBeforePair.second->targetPointXYZ());
        }
        else if (afterBeforePair.first && !afterBeforePair.second)
        {
            std::vector<RimWellPathTarget*> activeTargets =  wellGeomDef->activeWellTargets();
            size_t targetCount = activeTargets.size();
            if (targetCount > 1)
            {
                newPos = activeTargets[targetCount-1]->targetPointXYZ();
                cvf::Vec3d nextLastToLast = newPos - activeTargets[targetCount-2]->targetPointXYZ();
                newPos += 0.5*nextLastToLast;
            }
            else
            {
                newPos = afterBeforePair.first->targetPointXYZ() + cvf::Vec3d(0, 0, 200);
            }
        }

        RimWellPathTarget* newTarget = new RimWellPathTarget;
        newTarget->setAsPointTargetXYD({ newPos[0], newPos[1], -newPos[2] });

        wellGeomDef->insertTarget(firstTarget, newTarget);
        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization();

        return;
    }

    std::vector<RimWellPathGeometryDef*> geomDefs;
    caf::SelectionManager::instance()->objectsByType(&geomDefs);
    if (geomDefs.size() > 0)
    {
        RimWellPathGeometryDef* wellGeomDef = geomDefs[0];
        std::vector<RimWellPathTarget*> activeTargets =  wellGeomDef->activeWellTargets();
        
        size_t targetCount = activeTargets.size();

        if ( targetCount == 0 )
        {
            wellGeomDef->appendTarget();
        }
        else
        {
            cvf::Vec3d newPos = cvf::Vec3d::ZERO;

            if ( targetCount > 1 )
            {
                newPos = activeTargets[targetCount-1]->targetPointXYZ();
                cvf::Vec3d nextLastToLast = newPos - activeTargets[targetCount-2]->targetPointXYZ();
                newPos += 0.5*nextLastToLast;
            }
            else if ( targetCount > 0 )
            {
                newPos = activeTargets[targetCount-1]->targetPointXYZ() + cvf::Vec3d(0, 0, 200);
            }

            RimWellPathTarget* newTarget = new RimWellPathTarget;
            newTarget->setAsPointTargetXYD({ newPos[0], newPos[1], -newPos[2] });
            wellGeomDef->insertTarget(nullptr, newTarget);
        }

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


