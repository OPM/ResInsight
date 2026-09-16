#include "RicfCreateSaturationPressurePlots.h"
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

#include "RicfCreateSaturationPressurePlots.h"

#include "RiaLogging.h"

#include "RicfCommandForwarding.h"

#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimcEclipseCase.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCreateSaturationPressurePlots, "createSaturationPressurePlots" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateSaturationPressurePlots::RicfCreateSaturationPressurePlots()
{
    CAF_PDM_InitScriptableField( &m_caseIds, "caseIds", std::vector<int>(), "Case IDs" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateSaturationPressurePlots::execute()
{
    const QString commandName = classKeyword();

    RimProject* project = RimProject::current();
    if ( !project ) return RicfForwarding::errorResponse( "No project loaded", commandName );

    // Collect the result cases. No ids means all Eclipse result cases in the project.
    std::vector<RimEclipseResultCase*> resultCases;
    if ( m_caseIds().empty() )
    {
        for ( RimEclipseCase* c : project->eclipseCases() )
        {
            if ( auto* resultCase = dynamic_cast<RimEclipseResultCase*>( c ) ) resultCases.push_back( resultCase );
        }
    }
    else
    {
        for ( int caseId : m_caseIds() )
        {
            auto rimCase = RicfForwarding::findCase( caseId );
            if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

            if ( auto* resultCase = dynamic_cast<RimEclipseResultCase*>( rimCase.value() ) ) resultCases.push_back( resultCase );
        }
    }

    if ( resultCases.empty() ) return RicfForwarding::errorResponse( "No cases found", commandName );

    caf::PdmScriptResponse response;
    for ( RimEclipseResultCase* resultCase : resultCases )
    {
        RimEclipseResultCase_createSaturationPressurePlots method( resultCase );
        method.setTimeStep( 0 );

        auto result = method.execute();
        if ( !result )
        {
            // Legacy behavior: a case without the required data is skipped, not an error
            QString warning = QString( "%1: %2" ).arg( commandName ).arg( result.error() );
            RiaLogging::warning( warning.toStdString() );
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
        }
    }

    return response;
}
