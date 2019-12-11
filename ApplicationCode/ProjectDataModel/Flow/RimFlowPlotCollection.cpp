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

#include "RiaApplication.h"

#include "RimFlowCharacteristicsPlot.h"
#include "RimProject.h"
#include "RimWellAllocationPlot.h"
#include "RimWellDistributionPlot.h"

#include "cafProgressInfo.h"
#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT( RimFlowPlotCollection, "FlowPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::RimFlowPlotCollection()
{
    CAF_PDM_InitObject( "Flow Diagnostics Plots", ":/WellAllocPlots16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_flowCharacteristicsPlot, "FlowCharacteristicsPlot", "", "", "", "" );
    m_flowCharacteristicsPlot.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_defaultWellAllocPlot, "DefaultWellAllocationPlot", "", "", "", "" );
    m_defaultWellAllocPlot.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellDistributionPlot, "WellDistributionPlot", "", "", "", "" );
    m_wellDistributionPlot.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_storedWellAllocPlots,
                                "StoredWellAllocationPlots",
                                "Stored Well Allocation Plots",
                                "",
                                "",
                                "" );
    CAF_PDM_InitFieldNoDefault( &m_storedFlowCharacteristicsPlots,
                                "StoredFlowCharacteristicsPlots",
                                "Stored Flow Characteristics Plots",
                                "",
                                "",
                                "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::~RimFlowPlotCollection()
{
    delete m_defaultWellAllocPlot();

    m_storedWellAllocPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::closeDefaultPlotWindowAndDeletePlots()
{
    if ( m_defaultWellAllocPlot )
    {
        m_defaultWellAllocPlot->removeFromMdiAreaAndDeleteViewWidget();
        delete m_defaultWellAllocPlot();
    }

    delete m_flowCharacteristicsPlot;

    m_storedWellAllocPlots.deleteAllChildObjects();
    m_storedFlowCharacteristicsPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::loadDataAndUpdate()
{
    caf::ProgressInfo plotProgress( m_storedWellAllocPlots.size() + m_storedFlowCharacteristicsPlots.size() + 2, "" );

    if ( m_defaultWellAllocPlot ) m_defaultWellAllocPlot->loadDataAndUpdate();
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

    if ( m_wellDistributionPlot )
    {
        m_wellDistributionPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimFlowPlotCollection::plotCount() const
{
    size_t plotCount = 0;
    if ( m_defaultWellAllocPlot ) plotCount = 1;
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
void RimFlowPlotCollection::ensureDefaultFlowPlotsAreCreated()
{
    if ( !m_defaultWellAllocPlot() )
    {
        m_defaultWellAllocPlot = new RimWellAllocationPlot;
        m_defaultWellAllocPlot->setDescription( "Default Flow Diagnostics Plot" );
    }

    if ( !m_flowCharacteristicsPlot() )
    {
        m_flowCharacteristicsPlot = new RimFlowCharacteristicsPlot;
    }

    if ( !m_wellDistributionPlot() )
    {
        m_wellDistributionPlot = new RimWellDistributionPlot;
    }
}
