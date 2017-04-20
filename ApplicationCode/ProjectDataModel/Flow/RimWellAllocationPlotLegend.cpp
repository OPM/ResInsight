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

#include "RimWellAllocationPlotLegend.h"
#include "RimWellAllocationPlot.h"

CAF_PDM_SOURCE_INIT(RimWellAllocationPlotLegend, "WellAllocationPlotLegend");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlotLegend::RimWellAllocationPlotLegend()
{
    CAF_PDM_InitObject("Legend", ":/WellAllocLegend16x16.png", "", "");
    CAF_PDM_InitField(&m_showLegend, "ShowPlotLegend", true, "Show Plot Legend", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlotLegend::~RimWellAllocationPlotLegend()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellAllocationPlotLegend::objectToggleField()
{
    return &m_showLegend;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlotLegend::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showLegend)
    {
        RimWellAllocationPlot* walp;
        firstAncestorOrThisOfType(walp);

        if (walp) walp->showPlotLegend(m_showLegend());
    }
}

