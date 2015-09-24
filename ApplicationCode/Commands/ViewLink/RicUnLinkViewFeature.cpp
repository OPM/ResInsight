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

#include "RicUnLinkViewFeature.h"

#include "RimViewLink.h"
#include "RimView.h"
#include "RimViewLinker.h"

#include "cafSelectionManager.h"

#include <QAction>
#include "RiaApplication.h"
#include "RimProject.h"
#include "cafCmdFeatureManager.h"

CAF_CMD_SOURCE_INIT(RicUnLinkViewFeature, "RicUnLinkViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicUnLinkViewFeature::isCommandEnabled()
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    RimProject* proj = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = proj->findViewLinkerFromView(activeView);
    if (viewLinker)
    {
        if (viewLinker->masterView() == activeView)
        {
            return false;
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicUnLinkViewFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RimProject* proj = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = proj->findViewLinkerFromView(activeView);
    if (viewLinker)
    {
        for (size_t i = 0; i < viewLinker->viewLinks.size(); i++)
        {
            RimViewController* viewLink = viewLinker->viewLinks[i];
            if (viewLink->managedView() == activeView)
            {
                caf::SelectionManager::instance()->setSelectedItem(viewLink);

                caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature("RicDeleteItemFeature");
                if (feature)
                {
                    feature->action()->trigger();

                    return;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicUnLinkViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Unlink View");
}

