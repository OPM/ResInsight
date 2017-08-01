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

#include "RicfReplaceCase.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"

CAF_PDM_SOURCE_INIT(RicfReplaceCase, "replaceCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfReplaceCase::RicfReplaceCase()
{
    RICF_InitField(&m_caseId,        "case",        -1, "Case ID",  "", "", "");
    RICF_InitField(&m_newGridFile,   "newGridFile", QString(), "New Grid File",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfReplaceCase::execute()
{
    if (m_newGridFile().isNull())
    {
        RiaLogging::error("replaceCase: Required parameter newGridFile.");
        return;
    }

    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if (lastProjectPath.isNull())
    {
        RiaLogging::error("replaceCase: 'openProject' must be called before 'replaceCase' to specify project file to replace case in.");
        return;
    }


    cvf::ref<RiaProjectModifier> projectModifier = new RiaProjectModifier;
    if (m_caseId() == -1)
    {
        projectModifier->setReplaceCaseFirstOccurrence(m_newGridFile());
    }
    else
    {
        projectModifier->setReplaceCase(m_caseId(), m_newGridFile());
    }

    RiaApplication::instance()->loadProject(lastProjectPath, RiaApplication::PLA_NONE, projectModifier.p());
}
