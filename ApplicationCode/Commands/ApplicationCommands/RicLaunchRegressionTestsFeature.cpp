/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RiaApplication.h"
#include "RiaRegressionTest.h"

#include "cafPdmSettings.h"

#include <QAction>
#include <QDir>

CAF_CMD_SOURCE_INIT(RicLaunchRegressionTestsFeature, "RicLaunchRegressionTestsFeature");

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
void RicLaunchRegressionTestsFeature::onActionTriggered(bool isChecked)
{
    RiaRegressionTest regTestConfig;

    caf::PdmSettings::readFieldsFromApplicationStore(&regTestConfig);

    QStringList testFilter = regTestConfig.testFilter().split(";", QString::SkipEmptyParts);

    // Launch regression test using the current test folder and test filter
    RiaApplication::instance()->executeRegressionTests(regTestConfig.regressionTestFolder, &testFilter);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLaunchRegressionTestsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Launch Regression Tests");
}
