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

#include "RimWellLogPlot.h"

#include "RimWellLogPlotTrace.h"

#include "RiuWellLogPlot.h"
#include "RiuWellLogTracePlot.h"
#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

#include "cvfAssert.h"

#include <math.h>

CAF_PDM_SOURCE_INIT(RimWellLogPlot, "WellLogPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::RimWellLogPlot()
{
    CAF_PDM_InitObject("Well Log Plot", ":/WellCollection.png", "", "");

    m_viewer = NULL;

    CAF_PDM_InitField(&showWindow, "ShowWindow", true, "Show well log plot", "", "", "");
    showWindow.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&m_minimumVisibleDepth, "MinimumDepth", 0.0, "Minimum depth", "", "", "");
    CAF_PDM_InitField(&m_maximumVisibleDepth, "MaximumDepth", 1000.0, "Maximum depth", "", "", "");

    CAF_PDM_InitFieldNoDefault(&traces, "Traces", "",  "", "", "");
    traces.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&windowGeometry, "WindowGeometry", "", "", "", "");
    windowGeometry.uiCapability()->setUiHidden(true);
   
    m_depthRangeMinimum = 0.00;
    m_depthRangeMaximum = 1000.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::~RimWellLogPlot()
{
    RiuMainWindow::instance()->removeViewer(m_viewer);
    detachAllCurves();
    delete m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateViewerWidget()
{
    if (showWindow())
    {
        if (!m_viewer)
        {
            m_viewer = new RiuWellLogPlot(this, RiuMainWindow::instance());

            recreateTracePlots();
        }

        RiuMainWindow::instance()->addViewer(m_viewer, windowGeometry());
        RiuMainWindow::instance()->setActiveViewer(m_viewer);
        updateAxisRanges();
    }
    else
    {
        if (m_viewer)
        {
            windowGeometry = RiuMainWindow::instance()->windowGeometryForViewer(m_viewer);

            RiuMainWindow::instance()->removeViewer(m_viewer);
            detachAllCurves();
            delete m_viewer;
            m_viewer = NULL;
           
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &showWindow)
    {
        updateViewerWidget();
    }
    else if (changedField == &m_minimumVisibleDepth || changedField == &m_maximumVisibleDepth)
    {
        m_viewer->setDepthRange(m_minimumVisibleDepth, m_maximumVisibleDepth);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlot::objectToggleField()
{
    return &showWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::addTrace(RimWellLogPlotTrace* trace)
{
    traces.push_back(trace);
    if(m_viewer)
    {
        trace->recreateViewer();
        m_viewer->insertTracePlot(trace->viewer());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot* RimWellLogPlot::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::zoomDepth(double zoomFactor)
{
    double center       = (m_maximumVisibleDepth + m_minimumVisibleDepth)/2;
    double newHalfDepth = zoomFactor*(m_maximumVisibleDepth - m_minimumVisibleDepth)/2;

    double newMinimum = center - newHalfDepth;
    double newMaximum = center + newHalfDepth;

    setDepthRange(newMinimum, newMaximum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::panDepth(double panFactor)
{
    double delta = panFactor*(m_maximumVisibleDepth - m_minimumVisibleDepth);
    setDepthRange(m_minimumVisibleDepth + delta, m_maximumVisibleDepth + delta);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthRange(double minimumDepth, double maximumDepth)
{
    m_minimumVisibleDepth = minimumDepth;
    m_maximumVisibleDepth = maximumDepth;

    m_minimumVisibleDepth.uiCapability()->updateConnectedEditors();
    m_maximumVisibleDepth.uiCapability()->updateConnectedEditors();

    if(m_viewer) m_viewer->setDepthRange(m_minimumVisibleDepth, m_maximumVisibleDepth);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateAvailableDepthRange()
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    for (size_t tIdx = 0; tIdx < traces.size(); tIdx++)
    {
        double minTraceDepth = HUGE_VAL;
        double maxTraceDepth = -HUGE_VAL;

        if (traces[tIdx]->availableDepthRange(&minTraceDepth, &maxTraceDepth))
        {
            if (minTraceDepth < minDepth)
            {
                minDepth = minTraceDepth;
            }

            if (maxTraceDepth > maxDepth)
            {
                maxDepth = maxTraceDepth;
            }
        }
    }

    if (minDepth < HUGE_VAL && maxDepth > -HUGE_VAL)
    {
        m_depthRangeMinimum = minDepth;
        m_depthRangeMaximum = maxDepth;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlot::availableDepthRange(double* minimumDepth, double* maximumDepth)
{
    if (m_maximumVisibleDepth > m_minimumVisibleDepth)
    {
        *minimumDepth = m_depthRangeMinimum;
        *maximumDepth = m_depthRangeMaximum;

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::visibleDepthRange(double* minimumDepth, double* maximumDepth) const
{
    *minimumDepth = m_minimumVisibleDepth;
    *maximumDepth = m_maximumVisibleDepth;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setupBeforeSave()
{
    if (m_viewer)
    {
        windowGeometry = RiuMainWindow::instance()->windowGeometryForViewer(m_viewer);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::loadDataAndUpdate()
{
    updateViewerWidget();

    for (size_t tIdx = 0; tIdx < traces.size(); ++tIdx)
    {
        traces[tIdx]->loadDataAndUpdate();
    }

    updateAvailableDepthRange();
    updateAxisRanges();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateAxisRanges()
{
    if (m_viewer) m_viewer->setDepthRange(m_minimumVisibleDepth, m_maximumVisibleDepth);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setVisibleDepthRangeFromContents()
{
    setDepthRange(m_depthRangeMinimum, m_depthRangeMaximum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::recreateTracePlots()
{
    CVF_ASSERT(m_viewer);

    for (size_t tIdx = 0; tIdx < traces.size(); ++tIdx)
    {
        traces[tIdx]->recreateViewer();
        m_viewer->insertTracePlot(traces[tIdx]->viewer());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::detachAllCurves()
{
    for (size_t tIdx = 0; tIdx < traces.size(); ++tIdx)
    {
       traces[tIdx]->detachAllCurves();
    }
}
