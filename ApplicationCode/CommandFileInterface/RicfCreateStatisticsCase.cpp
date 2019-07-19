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

#include "RicfCreateStatisticsCase.h"

#include "RimCaseCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseStatisticsCaseCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimProject.h"

#include "RiaApplication.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_PDM_SOURCE_INIT(RicfCreateStatisticsCaseResult, "createStatisticsCaseResult");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateStatisticsCaseResult::RicfCreateStatisticsCaseResult(int caseId /*= -1*/)
{
    CAF_PDM_InitObject("statistics_case_result", "", "", "");
    CAF_PDM_InitField(&this->caseId, "caseId", caseId, "", "", "", "");
}

CAF_PDM_SOURCE_INIT(RicfCreateStatisticsCase, "createStatisticsCase");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateStatisticsCase::RicfCreateStatisticsCase()
{
    RICF_InitField(&m_caseGroupId, "caseGroupId", -1, "Case Group Id", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfCreateStatisticsCase::execute()
{
    RimProject* project = RiaApplication::instance()->project();

    std::vector<RimIdenticalGridCaseGroup*> gridCaseGroups;
    project->descendantsIncludingThisOfType(gridCaseGroups);
    for (auto gridCaseGroup : gridCaseGroups)
    {
        if (gridCaseGroup->groupId() == m_caseGroupId())
        {
            RimEclipseStatisticsCase* createdObject = gridCaseGroup->createAndAppendStatisticsCase();
            project->assignCaseIdToCase(createdObject);
            gridCaseGroup->updateConnectedEditors();
            RicfCommandResponse response;
            response.setResult(new RicfCreateStatisticsCaseResult(createdObject->caseId()));
            return response;
        }
    }
    return RicfCommandResponse(RicfCommandResponse::COMMAND_ERROR, "Could not find grid case group");
}
