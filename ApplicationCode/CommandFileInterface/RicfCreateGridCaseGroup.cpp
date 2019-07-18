/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Statoil ASA
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

#include "RicfCreateGridCaseGroup.h"

#include "RiaApplication.h"
#include "RiaImportEclipseCaseTools.h"

#include "RimIdenticalGridCaseGroup.h"

#include <QFileInfo>
#include <QDir>
#include <QStringList>

CAF_PDM_SOURCE_INIT(RicfCreateGridCaseGroupResult, "createGridCaseGroupResult");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateGridCaseGroupResult::RicfCreateGridCaseGroupResult(int caseGroupId /*= -1*/, const QString& caseGroupName /*= ""*/)
{
    CAF_PDM_InitObject("case_group_result", "", "", "");
    CAF_PDM_InitField(&this->caseGroupId, "groupId", caseGroupId, "", "", "", "");
    CAF_PDM_InitField(&this->caseGroupName, "groupName", caseGroupName, "", "", "", "");
}

CAF_PDM_SOURCE_INIT(RicfCreateGridCaseGroup, "createGridCaseGroup");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateGridCaseGroup::RicfCreateGridCaseGroup()
{
    RICF_InitFieldNoDefault(&m_casePaths, "casePaths", "List of Paths to Case Files", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfCreateGridCaseGroup::execute()
{
    QStringList casePaths;
    for (QString casePath : m_casePaths())
    {
        QFileInfo casePathInfo(casePath);
        if (!casePathInfo.exists())
        {
            QDir startDir(RiaApplication::instance()->startDir());
            casePath = startDir.absoluteFilePath(casePath);
        }
        casePaths.push_back(casePath);
    }
    
    RimIdenticalGridCaseGroup* caseGroup = nullptr;

    if (RiaImportEclipseCaseTools::addEclipseCases(casePaths, &caseGroup) && caseGroup)
    {
        RicfCommandResponse response;
        response.setResult(new RicfCreateGridCaseGroupResult(caseGroup->groupId(), caseGroup->name()));
        return response;
    }

    return RicfCommandResponse(RicfCommandResponse::COMMAND_ERROR, "Could not load grid case group");
}
