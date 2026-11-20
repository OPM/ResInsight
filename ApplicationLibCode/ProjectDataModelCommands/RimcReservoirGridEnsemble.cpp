/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimcReservoirGridEnsemble.h"

#include "Formations/RimFormationTools.h"
#include "RimEclipseCase.h"
#include "RimReservoirGridEnsemble.h"

#include "RimcDataContainerString.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimReservoirGridEnsemble, RimcReservoirGridEnsemble_formationNames, "FormationNamesInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcReservoirGridEnsemble_formationNames::RimcReservoirGridEnsemble_formationNames( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Formation Names", "", "", "Formation names of the main case, or of all realizations" );

    CAF_PDM_InitScriptableField( &m_union, "Union", false, "", "", "", "Return the union of the formation names of all realizations" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcReservoirGridEnsemble_formationNames::execute()
{
    auto* ensemble = self<RimReservoirGridEnsemble>();
    if ( !ensemble ) return std::unexpected( QString( "Self is not a RimReservoirGridEnsemble" ) );

    // Main case first, so the union keeps the formation order of the main case
    std::vector<std::vector<QString>> formationNameLists = { RimFormationTools::formationNames( ensemble->mainCase() ) };
    if ( m_union() )
    {
        for ( RimEclipseCase* eclipseCase : ensemble->cases() )
        {
            formationNameLists.push_back( RimFormationTools::formationNames( eclipseCase ) );
        }
    }

    auto* result           = new RimcDataContainerString();
    result->m_stringValues = m_union() ? RimFormationTools::unionOfFormationNames( formationNameLists ) : formationNameLists.front();
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcReservoirGridEnsemble_formationNames::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}
