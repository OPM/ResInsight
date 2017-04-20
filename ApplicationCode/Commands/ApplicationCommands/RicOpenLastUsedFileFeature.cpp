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

#include "RicOpenLastUsedFileFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RiuMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicOpenLastUsedFileFeature, "RicOpenLastUsedFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicOpenLastUsedFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicOpenLastUsedFileFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    if (!app->askUserToSaveModifiedProject()) return;

    QString fileName = app->preferences()->lastUsedProjectFileName;

    if (app->loadProject(fileName))
    {
        app->addToRecentFiles(fileName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicOpenLastUsedFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Open &Last Used Project");
}
