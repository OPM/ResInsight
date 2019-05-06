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

#include "RicCloseProjectFeature.h"

#include "RiaGuiApplication.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicCloseProjectFeature, "RicCloseProjectFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseProjectFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseProjectFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    RiaGuiApplication* app = RiaGuiApplication::instance();
    if (!app || !app->askUserToSaveModifiedProject()) return;

    app->closeProject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseProjectFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("&Close Project");
}
