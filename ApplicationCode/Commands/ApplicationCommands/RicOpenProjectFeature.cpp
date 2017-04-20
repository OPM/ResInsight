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

#include "RicOpenProjectFeature.h"

#include "RiaApplication.h"

#include "RiuMainWindow.h"

#include <QAction>
#include <QFileDialog>
#include <QStyle>

CAF_CMD_SOURCE_INIT(RicOpenProjectFeature, "RicOpenProjectFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicOpenProjectFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicOpenProjectFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    
    if (!app->askUserToSaveModifiedProject()) return;

    QString defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");
    QString fileName = QFileDialog::getOpenFileName(NULL, "Open ResInsight Project", defaultDir, "ResInsight project (*.rsp *.rip);;All files(*.*)");

    if (fileName.isEmpty()) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("BINARY_GRID", QFileInfo(fileName).absolutePath());

    if (app->loadProject(fileName))
    {
        app->addToRecentFiles(fileName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicOpenProjectFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Open Project");
    actionToSetup->setIcon(QIcon(":/openFolder24x24.png"));
}
