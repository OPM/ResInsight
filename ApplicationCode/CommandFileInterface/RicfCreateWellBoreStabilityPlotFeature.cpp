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

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "WellLogCommands/RicNewWellBoreStabilityPlotFeature.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimWbsParameters.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellPath.h"

#include <QAction>

CAF_PDM_SOURCE_INIT( RicfCreateWbsPlotResult, "createWbsPlotResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateWbsPlotResult::RicfCreateWbsPlotResult( int viewId /*= -1*/ )
{
    CAF_PDM_InitObject( "wbs_result", "", "", "" );
    CAF_PDM_InitField( &this->viewId, "viewId", viewId, "", "", "", "" );
}

CAF_PDM_SOURCE_INIT( RicfCreateWellBoreStabilityPlotFeature, "createWellBoreStabilityPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateWellBoreStabilityPlotFeature::RicfCreateWellBoreStabilityPlotFeature()
{
    RICF_InitField( &m_caseId, "caseId", -1, "GeoMech Case Id", "", "", "" );
    RICF_InitField( &m_wellPath, "wellPath", QString( "" ), "Well Path", "", "", "" );
    RICF_InitField( &m_timeStep, "timeStep", -1, "Time Step", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wbsParameters, "wbsParameters", "WbsParameters", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfCreateWellBoreStabilityPlotFeature::execute()
{
    RimProject* project = RiaApplication::instance()->project();

    std::vector<RimGeoMechCase*> geoMechCases;
    project->descendantsIncludingThisOfType( geoMechCases );

    RimGeoMechCase* chosenCase = nullptr;
    for ( RimGeoMechCase* geoMechCase : geoMechCases )
    {
        if ( geoMechCase->caseId() == m_caseId() )
        {
            chosenCase = geoMechCase;
            break;
        }
    }

    RimWellPath* chosenWellPath = nullptr;
    for ( RimWellPath* wellPath : project->allWellPaths() )
    {
        if ( wellPath->name() == m_wellPath() )
        {
            chosenWellPath = wellPath;
            break;
        }
    }

    if ( chosenCase && chosenWellPath && m_timeStep() >= 0 )
    {
        RimWellBoreStabilityPlot* wbsPlot = RicNewWellBoreStabilityPlotFeature::createPlot( chosenCase,
                                                                                            chosenWellPath,
                                                                                            m_timeStep(),
                                                                                            m_wbsParameters() );
        RicfCommandResponse       response;
        response.setResult( new RicfCreateWbsPlotResult( wbsPlot->id() ) );
        return response;
    }

    QString error = QString( "createWellBoreStabilityPlot: Could not find GeoMech case with id %1" ).arg( m_caseId() );
    RiaLogging::error( error );
    return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
}
