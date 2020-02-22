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
bool RicExecuteScriptForCasesFeature::isCommandEnabled()
{
    std::vector<RimCase*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( selection.size() > 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptForCasesFeature::onActionTriggered( bool isChecked )
{
    QString scriptAbsolutePath = userData().toString();

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    mainWindow->showProcessMonitorDockPanel();

    RiaApplication* app        = RiaApplication::instance();
    QString         octavePath = app->octavePath();
    if ( !octavePath.isEmpty() )
    {
        QStringList arguments = RimCalcScript::createCommandLineArguments( scriptAbsolutePath );

        std::vector<RimCase*> selection;
        caf::SelectionManager::instance()->objectsByType( &selection );

        // Get case ID from selected cases in selection model
        std::vector<int> caseIdsInSelection;
        for ( size_t i = 0; i < selection.size(); i++ )
        {
            RimCase* casePtr = selection[i];
            caseIdsInSelection.push_back( casePtr->caseId );
        }

        if ( caseIdsInSelection.size() > 0 )
        {
            RiaApplication::instance()->launchProcessForMultipleCases( octavePath,
                                                                       arguments,
                                                                       caseIdsInSelection,
                                                                       app->octaveProcessEnvironment() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteScriptForCasesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Execute script" );
}
