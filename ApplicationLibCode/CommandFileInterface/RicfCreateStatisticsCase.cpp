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

#include "Riu3DMainWindowTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_PDM_SOURCE_INIT( RicfCreateStatisticsCaseResult, "createStatisticsCaseResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateStatisticsCaseResult::RicfCreateStatisticsCaseResult( int caseId /*= -1*/ )
{
    CAF_PDM_InitObject( "statistics_case_result", "", "", "" );
    CAF_PDM_InitField( &this->caseId, "caseId", caseId, "", "", "", "" );
}

CAF_PDM_SOURCE_INIT( RicfCreateStatisticsCase, "createStatisticsCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateStatisticsCase::RicfCreateStatisticsCase()
{
    CAF_PDM_InitScriptableField( &m_caseGroupId, "caseGroupId", -1, "Case Group Id", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateStatisticsCase::execute()
{
    RimProject* project = RimProject::current();

    std::vector<RimIdenticalGridCaseGroup*> gridCaseGroups;
    project->descendantsIncludingThisOfType( gridCaseGroups );
    for ( auto gridCaseGroup : gridCaseGroups )
    {
        if ( gridCaseGroup->groupId() == m_caseGroupId() )
        {
            RimEclipseStatisticsCase* createdObject = gridCaseGroup->createAndAppendStatisticsCase();
            project->assignCaseIdToCase( createdObject );
            gridCaseGroup->updateConnectedEditors();
            caf::PdmScriptResponse response;
            response.setResult( new RicfCreateStatisticsCaseResult( createdObject->caseId() ) );
            return response;
        }
    }
    return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "Could not find grid case group" );
}
