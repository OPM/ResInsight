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

#include "RicEditScriptFeature.h"

#include "RicScriptFeatureImpl.h"

#include "RimCalcScript.h"
#include "RiaApplication.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicEditScriptFeature, "RicEditScriptFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditScriptFeature::isCommandEnabled()
{
    std::vector<RimCalcScript*> selection = RicScriptFeatureImpl::selectedScripts();
    return selection.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditScriptFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimCalcScript*> selection = RicScriptFeatureImpl::selectedScripts();
    CVF_ASSERT(selection.size() > 0);

    RimCalcScript* calcScript = selection[0];

    RiaApplication* app = RiaApplication::instance();
    QString scriptEditor = app->scriptEditorPath();
    if (!scriptEditor.isEmpty())
    {
        QStringList arguments;
        arguments << calcScript->absolutePath;

        QProcess* myProcess = new QProcess(this);
        myProcess->start(scriptEditor, arguments);

        if (!myProcess->waitForStarted(1000))
        {
            QMessageBox::warning(Riu3DMainWindowTools::mainWindowWidget(), "Script editor", "Failed to start script editor executable\n" + scriptEditor);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditScriptFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit");
}
