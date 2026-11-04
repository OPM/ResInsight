/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimGroupNetworkPlotCollection.h"

#include "RimGroupNetworkPlot.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimGroupNetworkPlotCollection, "RimGroupNetworkPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkPlotCollection::RimGroupNetworkPlotCollection()
{
    CAF_PDM_InitObject( "Production Network Plots", ":/summary/components/images/production-network.svg" );

    CAF_PDM_InitFieldNoDefault( &m_items, "Plots", "Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkPlot* RimGroupNetworkPlotCollection::createPlot( RimGroupNetworkData* dataSource )
{
    auto* plot = new RimGroupNetworkPlot();
    plot->setDataSource( dataSource );
    addItem( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGroupNetworkPlot*> RimGroupNetworkPlotCollection::plots() const
{
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGroupNetworkPlotCollection::plotCount() const
{
    return count();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlotCollection::deleteAllPlots()
{
    deleteAllItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlotCollection::loadDataAndUpdateAllPlots()
{
    for ( auto* plot : plots() )
    {
        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlotCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewGroupNetworkPlotFeature";
}
