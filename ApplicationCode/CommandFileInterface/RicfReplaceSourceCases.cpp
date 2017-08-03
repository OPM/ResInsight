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

#include "RicfReplaceSourceCases.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"

CAF_PDM_SOURCE_INIT(RicfReplaceSourceCases, "replaceSourceCases");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfReplaceSourceCases::RicfReplaceSourceCases()
{
    RICF_InitField(&m_caseGroupId,  "caseGroupId",  -1,        "Case Group ID",  "", "", "");
    RICF_InitField(&m_gridListFile, "gridListFile", QString(), "Grid List File",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfReplaceSourceCases::execute()
{
    if (m_gridListFile().isNull())
    {
        RiaLogging::error("replaceSourceCases: Required parameter gridListFile.");
        return;
    }

    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if (lastProjectPath.isNull())
    {
        RiaLogging::error("replaceSourceCases: 'openProject' must be called before 'replaceSourceCases' to specify project file to replace cases in.");
        return;
    }

    cvf::ref<RiaProjectModifier> projectModifier = new RiaProjectModifier;

    std::vector<QString> listFileNames = RiaApplication::readFileListFromTextFile(m_gridListFile());
    if (m_caseGroupId() == -1)
    {
        projectModifier->setReplaceSourceCasesFirstOccurrence(listFileNames);
    }
    else
    {
        projectModifier->setReplaceSourceCasesById(m_caseGroupId(), listFileNames);
    }

    RiaApplication::instance()->loadProject(lastProjectPath, RiaApplication::PLA_CALCULATE_STATISTICS, projectModifier.p());
}
