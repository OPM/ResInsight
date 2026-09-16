/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-2019 Statoil ASA
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicfLoadCase.h"

#include "RicfCommandForwarding.h"

#include "RimCase.h"
#include "RimProject.h"
#include "RimcProject.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfLoadCaseResult, "loadCaseResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfLoadCaseResult::RicfLoadCaseResult( int caseId )
{
    CAF_PDM_InitObject( "case_result" );
    CAF_PDM_InitField( &this->caseId, "id", caseId, "" );
}

CAF_PDM_SOURCE_INIT( RicfLoadCase, "loadCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfLoadCase::RicfLoadCase()
{
    CAF_PDM_InitScriptableField( &m_path, "path", QString(), "Path to Case File" );
    CAF_PDM_InitScriptableField( &m_gridOnly, "gridOnly", false, "Load Grid Data Only" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfLoadCase::execute()
{
    const QString commandName = classKeyword();

    RimProject_loadCase method( RimProject::current() );
    method.setPath( m_path() );
    method.setGridOnly( m_gridOnly() );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    auto* rimCase = dynamic_cast<RimCase*>( result.value() );
    if ( !rimCase ) return RicfForwarding::errorResponse( "Loaded object is not a case", commandName );

    caf::PdmScriptResponse response;
    response.setResult( new RicfLoadCaseResult( rimCase->caseId() ) );
    return response;
}
