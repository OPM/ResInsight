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

#include "RicfCommandForwarding.h"

#include "RimIdenticalGridCaseGroup.h"
#include "RimProject.h"
#include "RimcProject.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCreateGridCaseGroupResult, "createGridCaseGroupResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateGridCaseGroupResult::RicfCreateGridCaseGroupResult( int caseGroupId /*= -1*/, const QString& caseGroupName /*= ""*/ )
{
    CAF_PDM_InitObject( "case_group_result" );
    CAF_PDM_InitField( &this->caseGroupId, "groupId", caseGroupId, "" );
    CAF_PDM_InitField( &this->caseGroupName, "groupName", caseGroupName, "" );
}

CAF_PDM_SOURCE_INIT( RicfCreateGridCaseGroup, "createGridCaseGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateGridCaseGroup::RicfCreateGridCaseGroup()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_casePaths, "casePaths", "List of Paths to Case Files" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateGridCaseGroup::execute()
{
    const QString commandName = classKeyword();

    RimProject_createGridCaseGroup method( RimProject::current() );
    method.setCasePaths( m_casePaths() );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    auto* caseGroup = dynamic_cast<RimIdenticalGridCaseGroup*>( result.value() );
    if ( !caseGroup ) return RicfForwarding::errorResponse( "Created object is not a grid case group", commandName );

    caf::PdmScriptResponse response;
    response.setResult( new RicfCreateGridCaseGroupResult( caseGroup->groupId(), caseGroup->name() ) );
    return response;
}
