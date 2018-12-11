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
#include "RicNewPolylineTargetFeature.h"

CAF_CMD_SOURCE_INIT(RicNewPolylineTargetFeature, "RicNewPolylineTargetFeature");

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimGridView.h"
#include "RimCase.h"
#include "RimUserDefinedPolylinesAnnotation.h"
#include "RimPolylineTarget.h"
#include "cafSelectionManager.h"
#include <QAction>

#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPolylineTargetFeature::isCommandEnabled()
{
    {
        std::vector<RimUserDefinedPolylinesAnnotation*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if ( !objects.empty() )
        {
            return true;
        }
    }
    {
        std::vector<RimPolylineTarget*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects, caf::SelectionManager::FIRST_LEVEL);

        if ( !objects.empty() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineTargetFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimPolylineTarget*> selectedTargets;
    caf::SelectionManager::instance()->objectsByType(&selectedTargets, caf::SelectionManager::FIRST_LEVEL);
    if (!selectedTargets.empty())
    {
        auto firstTarget = selectedTargets.front();
        RimUserDefinedPolylinesAnnotation* polylineDef = nullptr;
        firstTarget->firstAncestorOrThisOfTypeAsserted(polylineDef);
        
        auto afterBeforePair = polylineDef->findActiveTargetsAroundInsertionPoint(firstTarget);
        
        cvf::Vec3d newPos = cvf::Vec3d::ZERO;

        if (!afterBeforePair.first && afterBeforePair.second)
        {
            newPos = afterBeforePair.second->targetPointXYZ();

            // Small displacement to separate the targets
            newPos.x() -= 50;
            newPos.y() -= 50;
        } 
        else if (afterBeforePair.first && afterBeforePair.second)
        {
            newPos = 0.5*(afterBeforePair.first->targetPointXYZ() + afterBeforePair.second->targetPointXYZ());
        }
        else if (afterBeforePair.first && !afterBeforePair.second)
        {
            std::vector<RimPolylineTarget*> activeTargets = polylineDef->activeTargets();
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

        auto* newTarget = new RimPolylineTarget;
        newTarget->setAsPointTargetXYD({ newPos[0], newPos[1], -newPos[2] });

        polylineDef->insertTarget(firstTarget, newTarget);
        polylineDef->updateConnectedEditors();
        polylineDef->updateVisualization();

        return;
    }

    std::vector<RimUserDefinedPolylinesAnnotation*> polylineDefs;
    caf::SelectionManager::instance()->objectsByType(&polylineDefs);
    if (!polylineDefs.empty())
    {
        auto* polylineDef = polylineDefs[0];
        std::vector<RimPolylineTarget*> activeTargets =  polylineDef->activeTargets();
        
        size_t targetCount = activeTargets.size();

        if ( targetCount == 0 )
        {
            auto defaultPos = cvf::Vec3d::ZERO;

            // Set decent position
            std::vector<RimGridView*> gridViews;
            RiaApplication::instance()->project()->allVisibleGridViews(gridViews);
            if (!gridViews.empty())
            {
                auto minPos = gridViews.front()->ownerCase()->allCellsBoundingBox().min();
                defaultPos = minPos;
            }

            polylineDef->appendTarget(defaultPos);
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

            auto* newTarget = new RimPolylineTarget;
            newTarget->setAsPointTargetXYD({ newPos[0], newPos[1], -newPos[2] });
            polylineDef->insertTarget(nullptr, newTarget);
        }

        polylineDef->updateConnectedEditors();
        polylineDef->updateVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineTargetFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Target");
    actionToSetup->setIcon(QIcon(":/Well.png"));
}


