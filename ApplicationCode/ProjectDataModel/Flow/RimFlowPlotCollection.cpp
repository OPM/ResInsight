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

#include "RimWellAllocationPlot.h"


#include "cvfAssert.h"
#include "cafProgressInfo.h"

CAF_PDM_SOURCE_INIT(RimFlowPlotCollection, "FlowPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::RimFlowPlotCollection()
{
    CAF_PDM_InitObject("Flow Diagnostics Plots", ":/newIcon16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_defaultPlot, "DefaultFlowPlot", "", "", "", "");
    m_defaultPlot.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_flowPlots, "FlowPlots", "Stored Plots",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::~RimFlowPlotCollection()
{
    delete m_defaultPlot();

    m_flowPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::closeDefaultPlotWindowAndDeletePlots()
{
    if ( m_defaultPlot )
    {
        m_defaultPlot->removeFromMdiAreaAndDeleteViewWidget();
        delete m_defaultPlot();
    }
    m_flowPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::loadDataAndUpdate()
{
    caf::ProgressInfo plotProgress(m_flowPlots.size() + 1, "");

    if (m_defaultPlot) m_defaultPlot->loadDataAndUpdate();
    plotProgress.incrementProgress();

    for (RimWellAllocationPlot* p : m_flowPlots)
    {
        p->loadDataAndUpdate();
        plotProgress.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimFlowPlotCollection::plotCount() const
{
    return m_flowPlots.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::addPlot(RimWellAllocationPlot* plot)
{
    m_flowPlots.push_back(plot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot* RimFlowPlotCollection::defaultPlot()
{
    if ( !m_defaultPlot() ) 
    {
        m_defaultPlot = new RimWellAllocationPlot; 
        m_defaultPlot->setDescription("Default Flow Diagnostics Plot");
    }

    this->updateConnectedEditors();

    return m_defaultPlot();
}
