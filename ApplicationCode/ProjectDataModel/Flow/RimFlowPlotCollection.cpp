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

CAF_PDM_SOURCE_INIT(RimFlowPlotCollection, "FlowPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::RimFlowPlotCollection()
{
    CAF_PDM_InitObject("Flow Diagnostics Plots", ":/newIcon16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&defaultPlot, "DefaultFlowPlot", "", "", "", "");
    defaultPlot = new RimWellAllocationPlot;
    defaultPlot->setDescription("Default Flow Diagnostics Plot");
    defaultPlot.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&flowPlots, "FlowPlots", "Stored Plots",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::~RimFlowPlotCollection()
{
    delete defaultPlot();

    flowPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::closeDefaultPlotWindowAndDeletePlots()
{
    defaultPlot->removeFromMdiAreaAndDeleteViewWidget();

    flowPlots.deleteAllChildObjects();
}
