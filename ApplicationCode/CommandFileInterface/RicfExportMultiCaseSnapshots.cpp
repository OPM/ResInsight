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

#include "RicfExportMultiCaseSnapshots.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"

CAF_PDM_SOURCE_INIT(RicfExportMultiCaseSnapshots, "exportMultiCaseSnapshots");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportMultiCaseSnapshots::RicfExportMultiCaseSnapshots()
{
    RICF_InitField(&m_gridListFile, "gridListFile", QString(), "Grid List File",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportMultiCaseSnapshots::execute()
{
    if (m_gridListFile().isNull())
    {
        RiaLogging::error("exportMultiCaseSnapshots: Required parameter gridListFile.");
        return;
    }

    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if (lastProjectPath.isNull())
    {
        RiaLogging::error("exportMultiCaseSnapshots: 'openProject' must be called before 'exportMultiCaseSnapshots' to specify project file to replace cases in.");
        return;
    }

    std::vector<QString> listFileNames = RiaApplication::readFileListFromTextFile(m_gridListFile());
    RiaApplication::instance()->runMultiCaseSnapshots(lastProjectPath, listFileNames, RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::SNAPSHOTS));
}
