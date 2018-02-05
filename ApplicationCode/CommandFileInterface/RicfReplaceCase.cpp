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

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"

#include "RicfCommandFileExecutor.h"

#include "RimProject.h"

CAF_PDM_SOURCE_INIT(RicfSingleCaseReplace, "replaceCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfSingleCaseReplace::RicfSingleCaseReplace()
{
    RICF_InitField(&m_caseId,        "caseId",      -1, "Case ID",  "", "", "");
    RICF_InitField(&m_newGridFile,   "newGridFile", QString(), "New Grid File",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RicfSingleCaseReplace::caseId() const
{
    return m_caseId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicfSingleCaseReplace::filePath() const
{
    return m_newGridFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfSingleCaseReplace::execute()
{
    // Never call execute on this object, information is aggregated into RicfMultiCaseReplace
    CAF_ASSERT(false);
}




CAF_PDM_SOURCE_INIT(RicfMultiCaseReplace, "replaceCaseImpl_no_support_for_command_file_text_parsing");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfMultiCaseReplace::RicfMultiCaseReplace()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfMultiCaseReplace::setCaseReplacePairs(const std::map<int, QString>& caseIdToGridFileNameMap)
{
    m_caseIdToGridFileNameMap = caseIdToGridFileNameMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfMultiCaseReplace::execute()
{
    if (m_caseIdToGridFileNameMap.empty())
    {
        RiaLogging::error("replaceCaseImpl: No replacements available.");
        return;
    }

    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if (lastProjectPath.isNull())
    {
        RiaLogging::error("replaceCase: 'openProject' must be called before 'replaceCase' to specify project file to replace case in.");
        return;
    }

    cvf::ref<RiaProjectModifier> projectModifier = new RiaProjectModifier;
    for (const auto& a : m_caseIdToGridFileNameMap)
    {
        const auto caseId = a.first;
        const auto filePath = a.second;
        if (caseId < 0)
        {
            projectModifier->setReplaceCaseFirstOccurrence(filePath);
        }
        else
        {
            projectModifier->setReplaceCase(caseId, filePath);
        }
    }

    RiaApplication::instance()->loadProject(lastProjectPath, RiaApplication::PLA_NONE, projectModifier.p());
}
