/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogPlotTrace.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotCurve.h"

#include "RiuWellLogTracePlot.h"
#include "RiuWellLogPlot.h"
#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"
#include "cvfAssert.h"

#include <float.h>

CAF_PDM_SOURCE_INIT(RimWellLogPlotTrace, "WellLogPlotTrace");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrace::RimWellLogPlotTrace()
{
    CAF_PDM_InitObject("Trace", "", "", "");

    CAF_PDM_InitField(&show, "Show", true, "Show trace", "", "", "");
    show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&curves, "Curves", "",  "", "", "");
    curves.uiCapability()->setUiHidden(true);

    m_viewer = new RiuWellLogTracePlot; 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrace::~RimWellLogPlotTrace()
{
    delete m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &show)
    {
        m_viewer->setVisible(show());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotTrace::objectToggleField()
{
    return &show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::addCurve(RimWellLogPlotCurve* curve)
{
    CVF_ASSERT(m_viewer);
    curves.push_back(curve);
    curve->setPlot(m_viewer);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTracePlot* RimWellLogPlotTrace::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotTrace::availableDepthRange(double* minimumDepth, double* maximumDepth)
{
    double minDepth = DBL_MAX;
    double maxDepth = DBL_MIN;

    size_t curveCount = curves.size();
    if (curveCount < 1)
    {
        return false;
    }

    bool rangeUpdated = false;

    for (size_t cIdx = 0; cIdx < curveCount; cIdx++)
    {
        double minCurveDepth = DBL_MAX;
        double maxCurveDepth = DBL_MIN;

        if (curves[cIdx]->depthRange(&minCurveDepth, &maxCurveDepth))
        {
            if (minCurveDepth < minDepth)
            {
                minDepth = minCurveDepth;
                rangeUpdated = true;
            }

            if (maxCurveDepth > maxDepth)
            {
                maxDepth = maxCurveDepth;
                rangeUpdated = true;
            }
        }
    }

    if (rangeUpdated)
    {
        *minimumDepth = minDepth;
        *maximumDepth = maxDepth;
    }

    return rangeUpdated;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::initAfterRead()
{
    for(size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->setPlot(this->m_viewer);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::loadDataAndUpdate()
{
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->updatePlotData();
    }
}
