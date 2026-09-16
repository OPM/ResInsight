#include "RicfRunOctaveScript.h"
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

#include "RicfRunOctaveScript.h"

#include "RicfCommandForwarding.h"

#include "RimCase.h"
#include "RimProject.h"
#include "RimcProject.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfRunOctaveScript, "runOctaveScript" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfRunOctaveScript::RicfRunOctaveScript()
{
    CAF_PDM_InitScriptableField( &m_path, "path", QString(), "Path" );
    CAF_PDM_InitScriptableField( &m_caseIds, "caseIds", std::vector<int>(), "Case IDs" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfRunOctaveScript::execute()
{
    const QString commandName = classKeyword();

    std::vector<RimCase*> cases;
    for ( int caseId : m_caseIds() )
    {
        auto rimCase = RicfForwarding::findCase( caseId );
        if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );
        cases.push_back( rimCase.value() );
    }

    RimProject_runOctaveScript method( RimProject::current() );
    method.setPath( m_path() );
    method.setCases( cases );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
