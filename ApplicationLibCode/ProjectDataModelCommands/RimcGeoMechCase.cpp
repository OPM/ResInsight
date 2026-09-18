/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimcGeoMechCase.h"

#include "WellLogCommands/RicNewWellBoreStabilityPlotFeature.h"

#include "RimGeoMechCase.h"
#include "RimWbsParameters.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellPath.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimGeoMechCase, RimGeoMechCase_createWellBoreStabilityPlot, "createWellBoreStabilityPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase_createWellBoreStabilityPlot::RimGeoMechCase_createWellBoreStabilityPlot( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Well Bore Stability Plot", "", "", "Create a Well Bore Stability plot for a well path in this case" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "Well Path", "", "", "Well path to create the plot for" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "Zero-based time step index" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase_createWellBoreStabilityPlot::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase_createWellBoreStabilityPlot::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase_createWellBoreStabilityPlot::setParameters( const RimWbsParameters* parameters )
{
    m_parameters = parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimGeoMechCase_createWellBoreStabilityPlot::execute()
{
    auto* geoMechCase = self<RimGeoMechCase>();
    if ( !geoMechCase ) return std::unexpected( "No GeoMech case is available." );

    RimWellPath* wellPath = m_wellPath();
    if ( !wellPath ) return std::unexpected( "No well path specified." );

    if ( !wellPath->wellPathGeometry() )
    {
        return std::unexpected(
            QString( "The well path %1 has no geometry. Cannot create a Well Bore Stability Plot" ).arg( wellPath->name() ) );
    }

    if ( m_timeStep() < 0 ) return std::unexpected( "Time step must be non-negative." );

    RimWellBoreStabilityPlot* plot = RicNewWellBoreStabilityPlotFeature::createPlot( geoMechCase, wellPath, m_timeStep(), m_parameters );
    if ( !plot ) return std::unexpected( "Could not create Well Bore Stability plot." );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase_createWellBoreStabilityPlot::classKeywordReturnedType() const
{
    return RimWellBoreStabilityPlot::classKeywordStatic();
}
