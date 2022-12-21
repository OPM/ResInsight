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
#include "RimHistoryWellAllocationPlot.h"
#include "RimProject.h"
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

    CAF_PDM_InitFieldNoDefault( &m_defaultHistoryWellAllocPlot, "DefaultHistoryWellAllocationPlot", "" );
    m_defaultHistoryWellAllocPlot.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_defaultWellAllocPlot, "DefaultWellAllocationPlot", "" );
    m_defaultWellAllocPlot.uiCapability()->setUiTreeHidden( true );

    // CAF_PDM_InitFieldNoDefault( &m_dbgWellDistributionPlot, "DbgWellDistributionPlot", "");
    // m_dbgWellDistributionPlot.uiCapability()->setUiHidden( true );

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
    // TODO: Remove? Testing without manually deleting for RimHistoryWellAllocationPlot

    delete m_defaultWellAllocPlot();

    m_storedWellAllocPlots.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::deleteAllPlots()
{
    // TODO: Remove? Testing without manually deleting for RimHistoryWellAllocationPlot

    if ( m_defaultWellAllocPlot )
    {
        m_defaultWellAllocPlot->removeFromMdiAreaAndDeleteViewWidget();
        delete m_defaultWellAllocPlot();
    }

    delete m_flowCharacteristicsPlot;
    // delete m_dbgWellDistributionPlot;
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

    if ( m_defaultHistoryWellAllocPlot ) m_defaultHistoryWellAllocPlot->loadDataAndUpdate();
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

    // if ( m_dbgWellDistributionPlot )
    //{
    //    m_dbgWellDistributionPlot->loadDataAndUpdate();
    //}

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
    plotCount += m_defaultHistoryWellAllocPlot ? 1 : 0;
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
RimHistoryWellAllocationPlot* RimFlowPlotCollection::defaultHistoryWellAllocPlot()
{
    if ( !m_defaultHistoryWellAllocPlot() )
    {
        m_defaultHistoryWellAllocPlot = new RimHistoryWellAllocationPlot;
        m_defaultHistoryWellAllocPlot->setDescription( "Default History Well Allocation Plot" );
    }

    this->updateConnectedEditors();

    return m_defaultHistoryWellAllocPlot();
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

    if ( !m_defaultHistoryWellAllocPlot() )
    {
        m_defaultHistoryWellAllocPlot = new RimHistoryWellAllocationPlot;
        m_defaultHistoryWellAllocPlot->setDescription( "Default History Well Allocation Plot" );
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
