/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicShowMainWindowFeature.h"

#include "RiaApplication.h"

#include "RiuMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowMainWindowFeature, "RicShowMainWindowFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowMainWindowFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowMainWindowFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    RiuMainWindow* mainWnd = RiuMainWindow::instance();

    if (mainWnd->isMinimized())
    {
        mainWnd->showNormal();
        mainWnd->update();
    }
    else
    {
        mainWnd->show();
    }

    mainWnd->raise();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowMainWindowFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Open 3D Window");
    actionToSetup->setIcon(QIcon(":/3DWindow24x24.png"));
}
