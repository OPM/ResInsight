/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicLaunchRegressionTestsFeature.h"

#include "RiaRegressionTestRunner.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QDir>

CAF_CMD_SOURCE_INIT( RicLaunchRegressionTestsFeature, "RicLaunchRegressionTestsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLaunchRegressionTestsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLaunchRegressionTestsFeature::onActionTriggered( bool isChecked )
{
    RiaRegressionTestRunner::instance()->executeRegressionTests();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLaunchRegressionTestsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Regression Tests" );
}

CAF_CMD_SOURCE_INIT( RicLaunchRegressionTestDialogFeature, "RicLaunchRegressionTestDialogFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLaunchRegressionTestDialogFeature::showRegressionTestDialog()
{
    RiaRegressionTest regTestConfig;
    regTestConfig.readSettingsFromApplicationStore();

    caf::PdmUiPropertyViewDialog regressionTestDialog( nullptr, &regTestConfig, "Regression Test", "" );
    regressionTestDialog.resize( QSize( 600, 350 ) );

    if ( regressionTestDialog.exec() == QDialog::Accepted )
    {
        // Write preferences using QSettings and apply them to the application
        regTestConfig.writeSettingsToApplicationStore();

        RiaRegressionTestRunner::instance()->executeRegressionTests();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLaunchRegressionTestDialogFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLaunchRegressionTestDialogFeature::onActionTriggered( bool isChecked )
{
    RicLaunchRegressionTestDialogFeature::showRegressionTestDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLaunchRegressionTestDialogFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reg Test Dialog" );
}
