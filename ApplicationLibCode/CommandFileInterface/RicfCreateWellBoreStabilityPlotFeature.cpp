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
#include "RicfCreateWellBoreStabilityPlotFeature.h"

#include "RicfCommandForwarding.h"

#include "RimGeoMechCase.h"
#include "RimProject.h"
#include "RimWbsParameters.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellPath.h"
#include "RimcGeoMechCase.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCreateWbsPlotResult, "createWbsPlotResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateWbsPlotResult::RicfCreateWbsPlotResult( int viewId /*= -1*/ )
{
    CAF_PDM_InitObject( "wbs_result" );
    CAF_PDM_InitField( &this->viewId, "viewId", viewId, "" );
}

CAF_PDM_SOURCE_INIT( RicfCreateWellBoreStabilityPlotFeature, "createWellBoreStabilityPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateWellBoreStabilityPlotFeature::RicfCreateWellBoreStabilityPlotFeature()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "GeoMech Case Id" );
    CAF_PDM_InitScriptableField( &m_wellPath, "wellPath", QString( "" ), "Well Path" );
    CAF_PDM_InitScriptableField( &m_timeStep, "timeStep", -1, "Time Step" );

    CAF_PDM_InitFieldNoDefault( &m_wbsParameters, "wbsParameters", "WbsParameters" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateWellBoreStabilityPlotFeature::execute()
{
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* geoMechCase = dynamic_cast<RimGeoMechCase*>( rimCase.value() );
    if ( !geoMechCase )
    {
        return RicfForwarding::errorResponse( QString( "Could not find GeoMech case with id %1" ).arg( m_caseId() ), commandName );
    }

    RimWellPath* wellPath = RimProject::current()->wellPathByName( m_wellPath() );
    if ( !wellPath ) return RicfForwarding::errorResponse( QString( "Could not find well path '%1'" ).arg( m_wellPath() ), commandName );

    RimGeoMechCase_createWellBoreStabilityPlot method( geoMechCase );
    method.setWellPath( wellPath );
    method.setTimeStep( m_timeStep() );
    method.setParameters( m_wbsParameters() );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    auto* plot = dynamic_cast<RimWellBoreStabilityPlot*>( result.value() );
    if ( !plot ) return RicfForwarding::errorResponse( "Created object is not a Well Bore Stability plot", commandName );

    caf::PdmScriptResponse response;
    response.setResult( new RicfCreateWbsPlotResult( plot->id() ) );
    return response;
}
