/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfOpenProject.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaRegressionTestRunner.h"

CAF_PDM_SOURCE_INIT(RicfOpenProject, "openProject");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfOpenProject::RicfOpenProject()
{
    RICF_InitField(&m_path, "path", QString(), "Path", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfOpenProject::execute()
{
    bool ok = RiaApplication::instance()->loadProject(m_path);
    if (!ok)
    {
        RiaLogging::error(QString("openProject: Unable to open project at %1").arg(m_path()));
        return;
    }

    if (RiaRegressionTestRunner::instance()->isRunningRegressionTests())
    {
        RiaRegressionTestRunner::regressionTestConfigureProject();
    }

    RicfCommandFileExecutor::instance()->setLastProjectPath(m_path);
}
