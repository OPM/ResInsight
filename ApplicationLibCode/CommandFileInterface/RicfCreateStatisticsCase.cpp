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

#include "RicfCommandForwarding.h"

#include "RimEclipseStatisticsCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimcIdenticalGridCaseGroup.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCreateStatisticsCaseResult, "createStatisticsCaseResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateStatisticsCaseResult::RicfCreateStatisticsCaseResult( int caseId /*= -1*/ )
{
    CAF_PDM_InitObject( "statistics_case_result" );
    CAF_PDM_InitField( &this->caseId, "caseId", caseId, "" );
}

CAF_PDM_SOURCE_INIT( RicfCreateStatisticsCase, "createStatisticsCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateStatisticsCase::RicfCreateStatisticsCase()
{
    CAF_PDM_InitScriptableField( &m_caseGroupId, "caseGroupId", -1, "Case Group Id" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateStatisticsCase::execute()
{
    const QString commandName = classKeyword();

    auto caseGroup = RicfForwarding::findCaseGroup( m_caseGroupId() );
    if ( !caseGroup ) return RicfForwarding::errorResponse( caseGroup.error(), commandName );

    RimcIdenticalGridCaseGroup_createStatisticsCase method( caseGroup.value() );
    method.setPopulateResultSelection( true );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    auto* statsCase = dynamic_cast<RimEclipseStatisticsCase*>( result.value() );
    if ( !statsCase ) return RicfForwarding::errorResponse( "Created object is not a statistics case", commandName );

    caf::PdmScriptResponse response;
    response.setResult( new RicfCreateStatisticsCaseResult( statsCase->caseId() ) );
    return response;
}
