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

#include "GridCrossPlotCommands/RicCreateSaturationPressurePlotsFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSaturationPressurePlotCollection.h"

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
    std::vector<int> caseIds = m_caseIds();
    if ( caseIds.empty() )
    {
        RimProject* project = RimProject::current();
        if ( project )
        {
            auto eclipeCases = project->eclipseCases();
            for ( auto c : eclipeCases )
            {
                caseIds.push_back( c->caseId() );
            }
        }
    }

    if ( caseIds.empty() )
    {
        QString error( "createSaturationPressurePlots: No cases found" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RimProject* project = RimProject::current();
    if ( !project )
    {
        QString error( "No project loaded" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    auto eclipeCases = project->eclipseCases();
    for ( auto c : eclipeCases )
    {
        auto eclipseResultCase = dynamic_cast<RimEclipseResultCase*>( c );
        if ( !eclipseResultCase ) continue;

        for ( auto caseId : caseIds )
        {
            if ( c->caseId == caseId )
            {
                int timeStep = 0;
                RicCreateSaturationPressurePlotsFeature::createPlots( eclipseResultCase, timeStep );
            }
        }
    }

    RimSaturationPressurePlotCollection* collection = project->mainPlotCollection()->saturationPressurePlotCollection();
    collection->updateAllRequiredEditors();
    RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();

    return caf::PdmScriptResponse();
}
