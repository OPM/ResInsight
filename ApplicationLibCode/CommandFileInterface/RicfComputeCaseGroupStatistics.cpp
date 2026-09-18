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

#include "RicfComputeCaseGroupStatistics.h"

#include "RiaLogging.h"

#include "RicfCommandForwarding.h"

#include "RimEclipseStatisticsCase.h"
#include "RimProject.h"
#include "RimcEclipseStatisticsCase.h"

#include "cafPdmFieldScriptingCapability.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RicfComputeCaseGroupStatistics, "computeCaseGroupStatistics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfComputeCaseGroupStatistics::RicfComputeCaseGroupStatistics()
{
    CAF_PDM_InitScriptableField( &m_groupId, "caseGroupId", -1, "Case Group ID" );
    CAF_PDM_InitScriptableField( &m_caseIds, "caseIds", std::vector<int>(), "Case IDs" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfComputeCaseGroupStatistics::execute()
{
    const QString commandName = classKeyword();

    caf::PdmScriptResponse response;

    // Collect the statistics cases: explicit case ids, plus all statistics cases when a group id is given.
    // Note: legacy behavior is to include statistics cases from all groups when caseGroupId >= 0.
    std::vector<RimEclipseStatisticsCase*> statsCases;

    for ( int caseId : m_caseIds() )
    {
        auto statsCase = RicfForwarding::findStatisticsCase( caseId );
        if ( !statsCase )
        {
            QString warning = QString( "%1: %2" ).arg( commandName ).arg( statsCase.error() );
            RiaLogging::warning( warning.toStdString() );
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
            continue;
        }
        statsCases.push_back( statsCase.value() );
    }

    if ( m_groupId() >= 0 )
    {
        for ( RimEclipseStatisticsCase* statsCase : RimProject::current()->descendantsIncludingThisOfType<RimEclipseStatisticsCase>() )
        {
            if ( statsCase && std::find( statsCases.begin(), statsCases.end(), statsCase ) == statsCases.end() )
            {
                statsCases.push_back( statsCase );
            }
        }
    }

    for ( RimEclipseStatisticsCase* statsCase : statsCases )
    {
        RimcEclipseStatisticsCase_computeStatistics method( statsCase );
        method.setUpdateViews( true );

        auto result = method.execute();
        if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );
    }

    return response;
}
