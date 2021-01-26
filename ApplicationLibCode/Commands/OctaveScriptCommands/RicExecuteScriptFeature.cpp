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

#include "RicExecuteLastUsedScriptFeature.h"
#include "RicScriptFeatureImpl.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RimCalcScript.h"
#include "RiuMainWindow.h"
#include "RiuProcessMonitor.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>
#include <QFileInfo>
#include <QSettings>

#include "cafCmdFeatureManager.h"
#include <iostream>

CAF_CMD_SOURCE_INIT( RicExecuteScriptFeature, "RicExecuteScriptFeature" );

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
void RicExecuteScriptFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimCalcScript*> selection = RicScriptFeatureImpl::selectedScripts();
    CVF_ASSERT( selection.size() > 0 );

    executeScript( selection[0] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Execute" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptFeature::executeScript( RimCalcScript* calcScript )
{
    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    mainWindow->showProcessMonitorDockPanel();

    RiaApplication* app = RiaApplication::instance();
    if ( calcScript->scriptType() == RimCalcScript::OCTAVE )
    {
        QString octavePath = app->octavePath();
        if ( !octavePath.isEmpty() )
        {
            QStringList arguments = RimCalcScript::createCommandLineArguments( calcScript->absoluteFileName() );
            RiaApplication::instance()->launchProcess( octavePath, arguments, app->octaveProcessEnvironment() );
        }
    }
    else if ( calcScript->scriptType() == RimCalcScript::PYTHON )
    {
        QString pythonPath = app->pythonPath();
        if ( !pythonPath.isEmpty() )
        {
            QStringList         arguments = RimCalcScript::createCommandLineArguments( calcScript->absoluteFileName() );
            QProcessEnvironment penv      = app->pythonProcessEnvironment();

            RiuProcessMonitor* processMonitor = RiuMainWindow::instance()->processMonitor();
            if ( RiaApplication::instance()->preferences()->showPythonDebugInfo() && processMonitor )
            {
                QStringList debugInfo;
                debugInfo << "----- Launching Python interpreter -----";
                debugInfo << "Python interpreter path: " + pythonPath;
                debugInfo << "Using arguments: ";
                for ( QString argument : arguments )
                {
                    debugInfo << "*  " + argument;
                }
                QStringList envList = penv.toStringList();
                debugInfo << "Using environment: ";
                for ( QString envVariable : envList )
                {
                    debugInfo << "*  " + envVariable;
                }

                debugInfo << "------------------------------------";

                for ( QString debugString : debugInfo )
                {
                    std::cout << debugString.toStdString() << std::endl;
                    processMonitor->addStringToLog( debugString + "\n" );
                }
            }
            RiaApplication::instance()->launchProcess( pythonPath, arguments, penv );
        }
    }

    if ( !calcScript->absoluteFileName().isEmpty() )
    {
        QSettings settings;
        settings.setValue( RicExecuteLastUsedScriptFeature::lastUsedScriptPathKey(), calcScript->absoluteFileName() );

        auto cmdFeature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicExecuteLastUsedScriptFeature" );
        cmdFeature->action(); // Retrieve the action to update the looks
    }
}
