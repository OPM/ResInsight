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

#include "RicfCreateLgrForCompletions.h"

#include "RicfApplicationTools.h"
#include "RicfCommandForwarding.h"

#include "RimEclipseCase.h"
#include "RimWellPath.h"
#include "RimcEclipseCase.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QStringList>

CAF_PDM_SOURCE_INIT( RicfCreateLgrForCompletions, "createLgrForCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateLgrForCompletions::RicfCreateLgrForCompletions()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_timeStep, "timeStep", 0, "Time Step Index" );
    CAF_PDM_InitScriptableField( &m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names" );
    CAF_PDM_InitScriptableField( &m_refinementI, "refinementI", -1, "RefinementI" );
    CAF_PDM_InitScriptableField( &m_refinementJ, "refinementJ", -1, "RefinementJ" );
    CAF_PDM_InitScriptableField( &m_refinementK, "refinementK", -1, "RefinementK" );
    CAF_PDM_InitScriptableField( &m_splitType, "splitType", Lgr::SplitTypeEnum(), "SplitType" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateLgrForCompletions::execute()
{
    const QString commandName = classKeyword();

    QStringList               wellsNotFound;
    std::vector<RimWellPath*> wellPaths =
        RicfApplicationTools::wellPathsFromNames( RicfApplicationTools::toQStringList( m_wellPathNames ), &wellsNotFound );
    if ( !wellsNotFound.empty() )
    {
        return RicfForwarding::errorResponse( "These well paths were not found: " + wellsNotFound.join( ", " ), commandName );
    }
    if ( wellPaths.empty() ) return RicfForwarding::errorResponse( "No well paths found", commandName );

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase.value() );
    if ( !eclipseCase )
    {
        return RicfForwarding::errorResponse( QString( "Case with ID %1 is not an Eclipse case" ).arg( m_caseId() ), commandName );
    }

    RimEclipseCase_createLgrForCompletions method( eclipseCase );
    method.setWellPaths( wellPaths );
    method.setTimeStep( m_timeStep() );
    method.setRefinement( m_refinementI(), m_refinementJ(), m_refinementK() );
    method.setSplitType( m_splitType() );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    caf::PdmScriptResponse response;
    if ( !method.wellsIntersectingOtherLgrs().empty() )
    {
        QString warning = QString( "%1: No LGRs created for some wells due to existing intersecting LGR(s). Affected wells: %2" )
                              .arg( commandName )
                              .arg( method.wellsIntersectingOtherLgrs().join( ", " ) );
        response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
    }
    return response;
}
