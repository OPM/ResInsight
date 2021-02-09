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

#include "RimVfpPlotCollection.h"

#include "RiaApplication.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEquil.h"

#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimVfpPlot.h"

CAF_PDM_SOURCE_INIT( RimVfpPlotCollection, "RimVfpPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlotCollection::RimVfpPlotCollection()
{
    CAF_PDM_InitObject( "VFP Plots", ":/SummaryXPlotsLight16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_vfpPlots, "VfpPlots", "Vertical Flow Performance Plots", "", "", "" );
    m_vfpPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlotCollection::~RimVfpPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::addPlot( RimVfpPlot* newPlot )
{
    m_vfpPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::insertPlot( RimVfpPlot* vfpPlot, size_t index )
{
    m_vfpPlots.insert( index, vfpPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimVfpPlot*> RimVfpPlotCollection::plots() const
{
    return m_vfpPlots.childObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::deleteAllChildObjects()
{
    m_vfpPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimVfpPlotCollection::plotCount() const
{
    return m_vfpPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::removePlot( RimVfpPlot* vfpPlot )
{
    m_vfpPlots.removeChildObject( vfpPlot );
    updateAllRequiredEditors();
}
