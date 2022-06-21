/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicShowClassNamesFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RiaPreferencesSystem.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowClassNamesFeature, "RicShowClassNamesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowClassNamesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowClassNamesFeature::onActionTriggered( bool isChecked )
{
    RiaPreferences::current()->systemPreferences()->setAppendClassNameToUiText( isChecked );

    RiaGuiApplication::instance()->applyPreferences();
    RiaGuiApplication::instance()->applyGuiPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowClassNamesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Class Names" );
    actionToSetup->setCheckable( true );
    actionToSetup->setChecked( isCommandChecked() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowClassNamesFeature::isCommandChecked()
{
    return RiaPreferences::current()->systemPreferences()->appendClassNameToUiText();
}
