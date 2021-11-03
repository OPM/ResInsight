/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimStimPlanModelPlotCollection.h"

#include "RimStimPlanModelPlot.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimStimPlanModelPlotCollection, "StimPlanModelPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlotCollection::RimStimPlanModelPlotCollection()
{
    CAF_PDM_InitScriptableObject( "StimPlan Model Plots", ":/WellLogPlots16x16.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanModelPlots, "StimPlanModelPlots", "", "", "", "" );
    m_stimPlanModelPlots.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlotCollection::~RimStimPlanModelPlotCollection()
{
    m_stimPlanModelPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlotCollection::loadDataAndUpdateAllPlots()
{
    for ( const auto& w : m_stimPlanModelPlots() )
    {
        w->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimStimPlanModelPlotCollection::plotCount() const
{
    return m_stimPlanModelPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlotCollection::addStimPlanModelPlot( RimStimPlanModelPlot* newPlot )
{
    m_stimPlanModelPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanModelPlot*> RimStimPlanModelPlotCollection::stimPlanModelPlots() const
{
    return m_stimPlanModelPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlotCollection::deleteAllPlots()
{
    m_stimPlanModelPlots.deleteAllChildObjects();
}
