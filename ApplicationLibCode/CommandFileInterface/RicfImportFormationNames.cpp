/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RicfImportFormationNames.h"

#include "RicfCommandForwarding.h"

#include "Formations/RimFormationNames.h"
#include "RimCase.h"
#include "RimProject.h"
#include "RimcCase.h"
#include "RimcProject.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfImportFormationNames, "importFormationNames" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportFormationNames::RicfImportFormationNames()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_formationFiles, "formationFiles", "" );
    CAF_PDM_InitScriptableField( &m_applyToCaseId, "applyToCaseId", -1, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfImportFormationNames::execute()
{
    const QString commandName = classKeyword();

    RimProject_importFormationNames importMethod( RimProject::current() );
    importMethod.setFormationFiles( m_formationFiles() );

    auto importResult = importMethod.execute();
    if ( !importResult ) return RicfForwarding::errorResponse( importResult.error(), commandName );

    auto* formationNames = dynamic_cast<RimFormationNames*>( importResult.value() );
    if ( !formationNames ) return RicfForwarding::errorResponse( "Imported object is not a formation names object", commandName );

    // Apply to the given case, or to all grid cases when applyToCaseId is -1
    std::vector<RimCase*> cases;
    if ( m_applyToCaseId() == -1 )
    {
        cases = RimProject::current()->allGridCases();
    }
    else
    {
        auto rimCase = RicfForwarding::findCase( m_applyToCaseId() );
        if ( !rimCase )
        {
            caf::PdmScriptResponse response;
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, "Could not find the case to apply the formations to" );
            return response;
        }
        cases.push_back( rimCase.value() );
    }

    for ( RimCase* rimCase : cases )
    {
        RimCase_setFormationNames setMethod( rimCase );
        setMethod.setFormationNames( formationNames );

        auto result = setMethod.execute();
        if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );
    }

    return caf::PdmScriptResponse();
}
