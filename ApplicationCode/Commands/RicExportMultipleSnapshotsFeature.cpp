/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicExportMultipleSnapshotsFeature.h"

#include "RiaApplication.h"
#include "RimProject.h"
#include "RiuExportMultipleSnapshotsWidget.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicExportMultipleSnapshotsFeature, "RicExportMultipleSnapshotsFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportMultipleSnapshotsFeature::isCommandEnabled()
{
    RimProject* proj = RiaApplication::instance()->project();
    
    return proj;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSnapshotsFeature::onActionTriggered(bool isChecked)
{
    RimProject* proj = RiaApplication::instance()->project();

    if (proj)
    {
        // Enable the command system to be able to assign a value to multiple fields at the same time
        caf::CmdExecCommandSystemActivator activator;

        RiuExportMultipleSnapshotsWidget dlg(nullptr, proj);
        dlg.exec();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSnapshotsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Multiple Snapshots ...");
    //actionToSetup->setIcon(QIcon(":/Save.png"));
}

