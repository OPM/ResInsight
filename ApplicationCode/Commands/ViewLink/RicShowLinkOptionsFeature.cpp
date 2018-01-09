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

#include "RicShowLinkOptionsFeature.h"

#include "RiaApplication.h"
#include "RimProject.h"
#include "Rim3dView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicShowLinkOptionsFeature, "RicShowLinkOptionsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowLinkOptionsFeature::isCommandEnabled()
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    RimViewController* viewController = activeView->viewController();
   
    if (viewController)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowLinkOptionsFeature::onActionTriggered(bool isChecked)
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RimViewController* viewController = activeView->viewController();

    RiuMainWindow::instance()->selectAsCurrentItem(viewController);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowLinkOptionsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Link Options");
}

