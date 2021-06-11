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

#include "RicEclipseCaseNewGroupFeature.h"

#include "RicEclipseCaseNewGroupExec.h"

#include "RimCase.h"
#include "RimEclipseCaseCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEclipseCaseNewGroupFeature, "RicEclipseCaseNewGroupFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseCaseNewGroupFeature::isCommandEnabled()
{
    std::vector<RimCase*> caseSelection;
    caf::SelectionManager::instance()->objectsByType( &caseSelection );

    std::vector<RimEclipseCaseCollection*> caseCollSelection;
    caf::SelectionManager::instance()->objectsByType( &caseCollSelection );

    return caseSelection.size() > 0 || caseCollSelection.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseCaseNewGroupFeature::onActionTriggered( bool isChecked )
{
    RicEclipseCaseNewGroupExec* cmdExec = new RicEclipseCaseNewGroupExec();
    caf::CmdExecCommandManager::instance()->processExecuteCommand( cmdExec );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseCaseNewGroupFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Grid Case Group" );
}
