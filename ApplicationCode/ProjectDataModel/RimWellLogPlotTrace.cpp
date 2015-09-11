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

#include <math.h>

#define RI_LOGPLOTTRACE_MINX_DEFAULT    -10.0
#define RI_LOGPLOTTRACE_MAXX_DEFAULT    100.0


CAF_PDM_SOURCE_INIT(RimWellLogPlotTrace, "WellLogPlotTrace");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrace::RimWellLogPlotTrace()
{
    CAF_PDM_InitObject("Trace", "", "", "");

    CAF_PDM_InitField(&m_show, "Show", true, "Show trace", "", "", "");
    m_show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&curves, "Curves", "",  "", "", "");
    curves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_minimumValue, "MinimumValue", RI_LOGPLOTTRACE_MINX_DEFAULT, "Minimum value", "", "", "");
    CAF_PDM_InitField(&m_maximumValue, "MaximumValue", RI_LOGPLOTTRACE_MAXX_DEFAULT, "Maximum value", "", "", "");   
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
    if (changedField == &m_show)
    {
        if (m_viewer) m_viewer->setVisible(m_show());
    }
    else if (changedField == &m_minimumValue || changedField == &m_maximumValue)
    {
        m_viewer->setAxisScale(QwtPlot::xTop, m_minimumValue, m_maximumValue);
        m_viewer->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotTrace::objectToggleField()
{
    return &m_show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::addCurve(RimWellLogPlotCurve* curve)
{
    curves.push_back(curve);

    if (m_viewer)
    {
        curve->setPlot(m_viewer);
    }
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
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    size_t curveCount = curves.size();
    if (curveCount < 1)
    {
        return false;
    }

    bool rangeUpdated = false;

    for (size_t cIdx = 0; cIdx < curveCount; cIdx++)
    {
        double minCurveDepth = HUGE_VAL;
        double maxCurveDepth = -HUGE_VAL;

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
void RimWellLogPlotTrace::loadDataAndUpdate()
{
    CVF_ASSERT(m_viewer);

    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->updatePlotData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::recreateViewer()
{
    CVF_ASSERT(m_viewer == NULL);

    m_viewer = new RiuWellLogTracePlot(this);
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->setPlot(this->m_viewer);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::detachAllCurves()
{
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->detachCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::updateAxisRangesAndReplot()
{
    bool rangesChanged = false;

    if (m_viewer)
    {
        RimWellLogPlot* wellLogPlot;
        firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            double minimumDepth, maximumDepth;
            wellLogPlot->visibleDepthRange(&minimumDepth, &maximumDepth);

            m_viewer->setDepthRange(minimumDepth, maximumDepth);
            rangesChanged = true;
        }

        // Assume auto-scaling on X-axis as long as curves exist, reset to default if not
        if (curves.size() < 1)
        {
            m_viewer->setAxisScale(QwtPlot::xTop, RI_LOGPLOTTRACE_MINX_DEFAULT, RI_LOGPLOTTRACE_MAXX_DEFAULT);
            rangesChanged = true;
        }

        if (rangesChanged)
        {
            m_viewer->replot();
        }
    }
}
