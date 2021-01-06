/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicSaveProjectNoGlobalPathsFeature.h"

#include "RimProject.h"

#include <QAction>
#include <QDir>

CAF_CMD_SOURCE_INIT( RicSaveProjectNoGlobalPathsFeature, "RicSaveProjectNoGlobalPathsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveProjectNoGlobalPathsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveProjectNoGlobalPathsFeature::onActionTriggered( bool isChecked )
{
    RimProject* proj = RimProject::current();
    if ( proj )
    {
        proj->writeFile();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveProjectNoGlobalPathsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Save Project (No Global Paths)" );
}
