/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimFlowPlotCollection.h"

#include "RimFlowCharacteristicsPlot.h"
#include "RimProject.h"
#include "RimWellAllocationOverTimePlot.h"
#include "RimWellAllocationPlot.h"
#include "RimWellDistributionPlotCollection.h"

#include "cafProgressInfo.h"
#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT( RimFlowPlotCollection, "FlowPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::RimFlowPlotCollection()
{
    CAF_PDM_InitObject( "Flow Diagnostics Plots", ":/WellAllocPlots16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_flowCharacteristicsPlot, "FlowCharacteristicsPlot", "" );
    m_flowCharacteristicsPlot.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_defaultWellAllocOverTimePlot, "DefaultWellAllocationOverTimePlot", "" );
    m_defaultWellAllocOverTimePlot.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_defaultWellAllocPlot, "DefaultWellAllocationPlot", "" );
    m_defaultWellAllocPlot.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellDistributionPlotCollection, "WellDistributionPlotCollection", "" );
    m_wellDistributionPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_storedWellAllocPlots, "StoredWellAllocationPlots", "Stored Well Allocation Plots" );
    CAF_PDM_InitFieldNoDefault( &m_storedFlowCharacteristicsPlots,
                                "StoredFlowCharacteristicsPlots",
                                "Stored Flow Characteristics Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::~RimFlowPlotCollection()
{
    delete m_defaultWellAllocPlot();

    m_storedWellAllocPlots.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::deleteAllPlots()
{
    if ( m_defaultWellAllocPlot )
    {
        m_defaultWellAllocPlot->removeFromMdiAreaAndDeleteViewWidget();
        delete m_defaultWellAllocPlot();
    }
    delete m_defaultWellAllocOverTimePlot;
    delete m_flowCharacteristicsPlot;
    delete m_wellDistributionPlotCollection;

    m_storedWellAllocPlots.deleteChildren();
    m_storedFlowCharacteristicsPlots.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::loadDataAndUpdateAllPlots()
{
    caf::ProgressInfo plotProgress( m_storedWellAllocPlots.size() + m_storedFlowCharacteristicsPlots.size() + 4, "" );

    if ( m_defaultWellAllocPlot ) m_defaultWellAllocPlot->loadDataAndUpdate();
    plotProgress.incrementProgress();

    if ( m_defaultWellAllocOverTimePlot ) m_defaultWellAllocOverTimePlot->loadDataAndUpdate();
    plotProgress.incrementProgress();

    for ( RimWellAllocationPlot* p : m_storedWellAllocPlots )
    {
        p->loadDataAndUpdate();
        plotProgress.incrementProgress();
    }

    for ( RimFlowCharacteristicsPlot* p : m_storedFlowCharacteristicsPlots )
    {
        p->loadDataAndUpdate();
        plotProgress.incrementProgress();
    }

    if ( m_flowCharacteristicsPlot )
    {
        m_flowCharacteristicsPlot->loadDataAndUpdate();
    }

    if ( m_wellDistributionPlotCollection )
    {
        m_wellDistributionPlotCollection->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimFlowPlotCollection::plotCount() const
{
    size_t plotCount = 0;
    plotCount += m_defaultWellAllocPlot ? 1 : 0;
    plotCount += m_defaultWellAllocOverTimePlot ? 1 : 0;
    plotCount += m_storedWellAllocPlots.size();
    plotCount += m_storedFlowCharacteristicsPlots.size();
    return plotCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::addWellAllocPlotToStoredPlots( RimWellAllocationPlot* plot )
{
    m_storedWellAllocPlots.push_back( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::addFlowCharacteristicsPlotToStoredPlots( RimFlowCharacteristicsPlot* plot )
{
    m_storedFlowCharacteristicsPlots.push_back( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationOverTimePlot* RimFlowPlotCollection::defaultWellAllocOverTimePlot()
{
    if ( !m_defaultWellAllocOverTimePlot() )
    {
        m_defaultWellAllocOverTimePlot = new RimWellAllocationOverTimePlot;
        m_defaultWellAllocOverTimePlot->setDescription( "Default Well Allocation Over Time Plot" );
    }

    this->updateConnectedEditors();

    return m_defaultWellAllocOverTimePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot* RimFlowPlotCollection::defaultWellAllocPlot()
{
    if ( !m_defaultWellAllocPlot() )
    {
        m_defaultWellAllocPlot = new RimWellAllocationPlot;
        m_defaultWellAllocPlot->setDescription( "Default Flow Diagnostics Plot" );
    }

    this->updateConnectedEditors();

    return m_defaultWellAllocPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot* RimFlowPlotCollection::defaultFlowCharacteristicsPlot()
{
    if ( !m_flowCharacteristicsPlot() )
    {
        m_flowCharacteristicsPlot = new RimFlowCharacteristicsPlot;
    }

    this->updateConnectedEditors();

    return m_flowCharacteristicsPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlotCollection* RimFlowPlotCollection::wellDistributionPlotCollection() const
{
    return m_wellDistributionPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::ensureDefaultFlowPlotsAreCreated()
{
    if ( !m_defaultWellAllocPlot() )
    {
        m_defaultWellAllocPlot = new RimWellAllocationPlot;
        m_defaultWellAllocPlot->setDescription( "Default Flow Diagnostics Plot" );
    }

    if ( !m_defaultWellAllocOverTimePlot() )
    {
        m_defaultWellAllocOverTimePlot = new RimWellAllocationOverTimePlot;
        m_defaultWellAllocOverTimePlot->setDescription( "Default Well Allocation Over Time Plot" );
    }

    if ( !m_flowCharacteristicsPlot() )
    {
        m_flowCharacteristicsPlot = new RimFlowCharacteristicsPlot;
    }

    if ( !m_wellDistributionPlotCollection() )
    {
        m_wellDistributionPlotCollection = new RimWellDistributionPlotCollection;
    }
}
