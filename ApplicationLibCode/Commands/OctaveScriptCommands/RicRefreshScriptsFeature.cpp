/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016     Statoil ASA
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

#include "RicRefreshScriptsFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RimProject.h"
#include "RimScriptCollection.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRefreshScriptsFeature, "RicRefreshScriptsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRefreshScriptsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRefreshScriptsFeature::onActionTriggered( bool isChecked )
{
    refreshScriptFolders();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRefreshScriptsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Refresh" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRefreshScriptsFeature::refreshScriptFolders()
{
    RimProject*     proj  = RimProject::current();
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    proj->setScriptDirectories( prefs->scriptDirectories() );
    proj->scriptCollection()->updateConnectedEditors();
}
