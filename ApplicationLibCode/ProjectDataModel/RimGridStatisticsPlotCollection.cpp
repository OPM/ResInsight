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

#include "RimGridStatisticsPlotCollection.h"

#include "RimGridStatisticsPlot.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimGridStatisticsPlotCollection, "GridStatisticsPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsPlotCollection::RimGridStatisticsPlotCollection()
{
    CAF_PDM_InitScriptableObject( "Grid Statistics Plots", ":/WellLogPlots16x16.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_gridStatisticsPlots, "GridStatisticsPlots", "", "", "", "" );
    m_gridStatisticsPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlotCollection::reloadAllPlots()
{
    for ( const auto& w : m_gridStatisticsPlots() )
    {
        w->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlotCollection::addGridStatisticsPlot( RimGridStatisticsPlot* newPlot )
{
    m_gridStatisticsPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridStatisticsPlot*> RimGridStatisticsPlotCollection::gridStatisticsPlots() const
{
    return m_gridStatisticsPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlotCollection::deleteAllPlots()
{
    m_gridStatisticsPlots.deleteAllChildObjects();
}
