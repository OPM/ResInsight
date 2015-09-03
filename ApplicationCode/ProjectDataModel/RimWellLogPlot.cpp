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

#include <float.h>

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

    updateViewerWidget();
    updateAvailableDepthRange();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::~RimWellLogPlot()
{
    RiuMainWindow::instance()->removeViewer(m_viewer);
    delete m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateViewerWidget()
{
    if (showWindow())
    {
        bool isViewerCreated = false;
        if (!m_viewer)
        {
            m_viewer = new RiuWellLogPlot(this, RiuMainWindow::instance());

            RiuMainWindow::instance()->addViewer(m_viewer, windowGeometry());
            isViewerCreated = true;
        }
        
        if (m_viewer->parentWidget())
        {
            m_viewer->parentWidget()->showMaximized();
        }
        else
        {
            m_viewer->showMaximized(); 
        }

        RiuMainWindow::instance()->setActiveViewer(m_viewer);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &showWindow)
    {
        if (newValue == true)
        {
            updateViewerWidget();
        }
        else
        {
            if (m_viewer)
            {
                if (m_viewer->parentWidget())
                {
                    m_viewer->parentWidget()->hide();
                }
                else
                {
                    m_viewer->hide(); 
                }
            }
        }
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
    m_viewer->insertTracePlot(trace->viewer());
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

    m_viewer->setDepthRange(m_minimumVisibleDepth, m_maximumVisibleDepth);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateAvailableDepthRange()
{
    double minDepth = DBL_MAX;
    double maxDepth = DBL_MIN;

    for (size_t tIdx = 0; tIdx < traces.size(); tIdx++)
    {
        double minTraceDepth = DBL_MAX;
        double maxTraceDepth = DBL_MIN;

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

    m_depthRangeMinimum = minDepth;
    m_depthRangeMaximum = maxDepth;
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
void RimWellLogPlot::visibleDepthRange(double* minimumDepth, double* maximumDepth)
{
    *minimumDepth = m_minimumVisibleDepth;
    *maximumDepth = m_maximumVisibleDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::initAfterRead()
{
    for (size_t tIdx = 0; tIdx < traces.size(); ++tIdx)
    {
        m_viewer->insertTracePlot(traces[tIdx]->viewer());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setupBeforeSave()
{
     windowGeometry = RiuMainWindow::instance()->windowGeometryForViewer(m_viewer);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::loadDataAndUpdate()
{
    for (size_t tIdx = 0; tIdx < traces.size(); ++tIdx)
    {
        traces[tIdx]->loadDataAndUpdate();
    }
}
