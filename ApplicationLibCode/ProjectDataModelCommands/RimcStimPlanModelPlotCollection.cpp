/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
#include "RimcStimPlanModelPlotCollection.h"

#include "CompletionCommands/RicNewStimPlanModelPlotFeature.h"

#include "RimEclipseCase.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelPlot.h"
#include "RimStimPlanModelPlotCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimStimPlanModelPlotCollection,
                                   RimcStimPlanModelPlotCollection_appendStimPlanModelPlot,
                                   "AppendStimPlanModelPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcStimPlanModelPlotCollection_appendStimPlanModelPlot::RimcStimPlanModelPlotCollection_appendStimPlanModelPlot(
    caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create StimPlan Model", "", "", "Create a new StimPlan Model" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanModel, "StimPlanModel", "", "", "", "StimPlan Model" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcStimPlanModelPlotCollection_appendStimPlanModelPlot::execute()
{
    RimStimPlanModelPlot*           stimPlanModelPlot           = nullptr;
    RimStimPlanModelPlotCollection* stimPlanModelPlotCollection = self<RimStimPlanModelPlotCollection>();

    if ( m_stimPlanModel && stimPlanModelPlotCollection )
    {
        stimPlanModelPlot = RicNewStimPlanModelPlotFeature::createPlot( m_stimPlanModel );
    }

    return stimPlanModelPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcStimPlanModelPlotCollection_appendStimPlanModelPlot::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcStimPlanModelPlotCollection_appendStimPlanModelPlot::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimStimPlanModelPlot );
}
