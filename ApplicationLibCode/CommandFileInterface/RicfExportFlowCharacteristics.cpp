/////////////////////////////////////////////////////////////////////////////////
//
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

#include "RicfExportFlowCharacteristics.h"

#include "RicfCommandForwarding.h"

#include "RimEclipseResultCase.h"
#include "RimcEclipseCase.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfExportFlowCharacteristics, "exportFlowCharacteristics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportFlowCharacteristics::RicfExportFlowCharacteristics()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_selectedTimeSteps, "timeSteps", std::vector<int>(), "Selected Time Steps" );
    CAF_PDM_InitScriptableField( &m_injectors, "injectors", std::vector<QString>(), "Injectors" );
    CAF_PDM_InitScriptableField( &m_producers, "producers", std::vector<QString>(), "Producers" );
    CAF_PDM_InitScriptableField( &m_fileName, "fileName", QString(), "Export File Name" );
    CAF_PDM_InitScriptableField( &m_minCommunication, "minimumCommunication", 0.0, "Minimum Communication" );
    CAF_PDM_InitScriptableField( &m_maxPvFraction, "aquiferCellThreshold", 0.1, "Aquifer Cell Threshold" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportFlowCharacteristics::execute()
{
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* eclipseCase = dynamic_cast<RimEclipseResultCase*>( rimCase.value() );
    if ( !eclipseCase )
    {
        return RicfForwarding::errorResponse( QString( "Case with ID %1 is not an Eclipse result case" ).arg( m_caseId() ), commandName );
    }

    RimEclipseResultCase_exportFlowCharacteristics method( eclipseCase );
    method.setTimeSteps( m_selectedTimeSteps() );
    method.setInjectors( m_injectors() );
    method.setProducers( m_producers() );
    method.setFileName( m_fileName() );
    method.setMinimumCommunication( m_minCommunication() );
    method.setAquiferCellThreshold( m_maxPvFraction() );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
