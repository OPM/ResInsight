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

#include "RicExecuteScriptForCasesFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimCalcScript.h"
#include "RimCase.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicExecuteScriptForCasesFeature, "RicExecuteScriptForCasesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExecuteScriptForCasesFeature::isCommandEnabled() const
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimCase>();
    return !selection.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptForCasesFeature::onActionTriggered( bool isChecked )
{
    QString scriptAbsolutePath = userData().toString();

    if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->showProcessMonitorDockPanel();

    RiaApplication* app = RiaApplication::instance();

    QString             pathToScriptExecutable;
    QProcessEnvironment processEnvironment;

    if ( scriptAbsolutePath.endsWith( ".py" ) )
    {
        processEnvironment     = app->pythonProcessEnvironment();
        pathToScriptExecutable = app->pythonPath();

        if ( pathToScriptExecutable.isEmpty() )
        {
            RiaLogging::warning( "Path to Python executable is empty, not able to execute script" );
        }
    }
    else
    {
        processEnvironment     = app->octaveProcessEnvironment();
        pathToScriptExecutable = app->octavePath();
        if ( pathToScriptExecutable.isEmpty() )
        {
            RiaLogging::warning( "Path to Octave executable is empty, not able to execute script" );
        }
    }

    if ( !pathToScriptExecutable.isEmpty() )
    {
        QStringList      arguments = RimCalcScript::createCommandLineArguments( scriptAbsolutePath );
        std::vector<int> caseIdsInSelection;
        {
            const auto selection = caf::SelectionManager::instance()->objectsByType<RimCase>();
            for ( RimCase* rimCase : selection )
            {
                caseIdsInSelection.push_back( rimCase->caseId() );
            }
        }

        RiaApplication::instance()->launchProcessForMultipleCases( pathToScriptExecutable, arguments, caseIdsInSelection, processEnvironment );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptForCasesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Execute script" );
}
