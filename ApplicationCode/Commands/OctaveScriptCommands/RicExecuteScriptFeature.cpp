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

#include "RicExecuteScriptFeature.h"

#include "RicScriptFeatureImpl.h"

#include "RimCalcScript.h"
#include "RiaApplication.h"
#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicExecuteScriptFeature, "RicExecuteScriptFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExecuteScriptFeature::isCommandEnabled()
{
    std::vector<RimCalcScript*> selection = RicScriptFeatureImpl::selectedScripts();
    return selection.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimCalcScript*> selection = RicScriptFeatureImpl::selectedScripts();
    CVF_ASSERT(selection.size() > 0);

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    mainWindow->showProcessMonitorDockPanel();

    RimCalcScript* calcScript = selection[0];

    RiaApplication* app = RiaApplication::instance();
    QString octavePath = app->octavePath();
    if (!octavePath.isEmpty())
    {
        // TODO: Must rename RimCalcScript::absolutePath to absoluteFileName, as the code below is confusing
        // absolutePath() is a function in QFileInfo
        QFileInfo fi(calcScript->absolutePath());
        QString octaveFunctionSearchPath = fi.absolutePath();

        QStringList arguments = app->octaveArguments();
        arguments.append("--path");
        arguments << octaveFunctionSearchPath;
        arguments << calcScript->absolutePath();

        RiaApplication::instance()->launchProcess(octavePath, arguments);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Execute");
}
