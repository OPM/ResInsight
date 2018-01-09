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

#include "RicShowContributingWellsFeature.h"

#include "RicShowContributingWellsFeatureImpl.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimViewManipulator.h"

#include "RiuMainWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowContributingWellsFeature, "RicShowContributingWellsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowContributingWellsFeature::isCommandEnabled()
{
    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    if (collection.size() == 1)
    {
        RimSimWellInView* well = collection[0];
        RimEclipseView* eclipseView = nullptr;
        well->firstAncestorOrThisOfType(eclipseView);

        if (eclipseView)
        {
            RimFlowDiagSolution* flowDiagSolution = eclipseView->cellResult()->flowDiagSolution();
            if (!flowDiagSolution)
            {
                RimEclipseResultCase* eclipseResultCase = nullptr;
                well->firstAncestorOrThisOfTypeAsserted(eclipseResultCase);

                if (eclipseResultCase)
                {
                    flowDiagSolution = eclipseResultCase->defaultFlowDiagSolution();
                }
            }

            if (flowDiagSolution)
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    CAF_ASSERT(collection.size() == 1);

    RimSimWellInView* well = collection[0];
    RimEclipseView* eclipseView = nullptr;
    well->firstAncestorOrThisOfTypeAsserted(eclipseView);

    RimEclipseResultCase* eclipseResultCase = nullptr;
    well->firstAncestorOrThisOfTypeAsserted(eclipseResultCase);

    RimEclipseView* modifiedView = RicShowContributingWellsFeatureImpl::maniuplateSelectedView(eclipseResultCase, well->name(), eclipseView->currentTimeStep());
    if (modifiedView)
    {
        modifiedView->createDisplayModelAndRedraw();

        std::vector<Rim3dView*> viewsToUpdate;
        viewsToUpdate.push_back(modifiedView);

        RimViewManipulator::applySourceViewCameraOnDestinationViews(eclipseView, viewsToUpdate);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Show Contributing Wells");
}
