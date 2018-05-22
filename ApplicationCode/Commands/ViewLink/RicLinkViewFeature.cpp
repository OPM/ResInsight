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

#include "RicLinkViewFeature.h"

#include "RiaApplication.h"

#include "RicLinkVisibleViewsFeature.h"

#include "RimProject.h"
#include "Rim3dView.h"
#include "RimViewLinkerCollection.h"
#include "RimViewLinker.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicLinkViewFeature, "RicLinkViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicLinkViewFeature::isCommandEnabled()
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    RimProject* proj = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = proj->viewLinkerCollection->viewLinker();
    
    if(!viewLinker) return false;

    RimViewController* viewController = activeView->viewController();
    
    if(viewController)
    {
        return false;
    }
    else
    {
        if (!activeView->isMasterView())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkViewFeature::onActionTriggered(bool isChecked)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (!activeView) return;

    std::vector<RimGridView*> views;
    views.push_back(activeView);

    RicLinkVisibleViewsFeature::linkViews(views);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Link View");
    actionToSetup->setIcon(QIcon(":/chain.png"));
}

