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

#define RI_LOGPLOT_MINDEPTH_DEFAULT 0.0
#define RI_LOGPLOT_MAXDEPTH_DEFAULT 1000.0

namespace caf {

    template<>
    void caf::AppEnum< RimWellLogPlot::DepthTypeEnum >::setUp()
    {
        addItem(RimWellLogPlot::MEASURED_DEPTH,       "MEASURED_DEPTH",       "Measured Depth");
        addItem(RimWellLogPlot::TRUE_VERTICAL_DEPTH,  "TRUE_VERTICAL_DEPTH",  "True Vertical Depth");
        setDefault(RimWellLogPlot::MEASURED_DEPTH);
    }

} // End namespace caf


CAF_PDM_SOURCE_INIT(RimWellLogPlot, "WellLogPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::RimWellLogPlot()
{
    CAF_PDM_InitObject("Well Log Plot", ":/WellCollection.png", "", "");

    m_viewer = NULL;

    CAF_PDM_InitField(&m_showWindow, "ShowWindow", true, "Show well log plot", "", "", "");
    m_showWindow.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Well Log Plot"),"Name", "", "", "");
    
    caf::AppEnum< RimWellLogPlot::DepthTypeEnum > depthType = MEASURED_DEPTH;
    CAF_PDM_InitField(&m_depthType, "DepthType", depthType, "Depth type",   "", "", "");

    CAF_PDM_InitField(&m_minimumVisibleDepth, "MinimumDepth", 0.0, "Minimum depth", "", "", "");
    CAF_PDM_InitField(&m_maximumVisibleDepth, "MaximumDepth", 1000.0, "Maximum depth", "", "", "");    

    CAF_PDM_InitFieldNoDefault(&traces, "Traces", "",  "", "", "");
    traces.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&windowGeometry, "WindowGeometry", "", "", "", "");
    windowGeometry.uiCapability()->setUiHidden(true);
   
    m_depthRangeMinimum = HUGE_VAL;
    m_depthRangeMaximum = -HUGE_VAL;
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
    if (m_showWindow())
    {
        if (!m_viewer)
        {
            m_viewer = new RiuWellLogPlot(this, RiuMainWindow::instance());

            recreateTracePlots();

            RiuMainWindow::instance()->addViewer(m_viewer, windowGeometry());
            RiuMainWindow::instance()->setActiveViewer(m_viewer);
        }

        updateViewerWidgetWindowTitle();
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
    if (changedField == &m_showWindow)
    {
        if (m_showWindow)
        {
            loadDataAndUpdate();
        }
        else
        {
            updateViewerWidget();
        }
    }
    else if (changedField == &m_minimumVisibleDepth || changedField == &m_maximumVisibleDepth)
    {
        updateAxisRanges();
    }
    else if (changedField == &m_userName)
    {
        updateViewerWidgetWindowTitle();
    }
    if (changedField == &m_depthType)
    {
        updateTraces();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlot::objectToggleField()
{
    return &m_showWindow;
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
void RimWellLogPlot::zoomDepth(double zoomFactor, double zoomCenter)
{
    double newMinimum = zoomCenter - (zoomCenter - m_minimumVisibleDepth)*zoomFactor;
    double newMaximum = zoomCenter + (m_maximumVisibleDepth - zoomCenter)*zoomFactor;

    setVisibleDepthRange(newMinimum, newMaximum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::panDepth(double panFactor)
{
    double delta = panFactor*(m_maximumVisibleDepth - m_minimumVisibleDepth);
    setVisibleDepthRange(m_minimumVisibleDepth + delta, m_maximumVisibleDepth + delta);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setVisibleDepthRange(double minimumDepth, double maximumDepth)
{
    m_minimumVisibleDepth = minimumDepth;
    m_maximumVisibleDepth = maximumDepth;

    m_minimumVisibleDepth.uiCapability()->updateConnectedEditors();
    m_maximumVisibleDepth.uiCapability()->updateConnectedEditors();

    updateAxisRanges();
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

    m_depthRangeMinimum = minDepth;
    m_depthRangeMaximum = maxDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::availableDepthRange(double* minimumDepth, double* maximumDepth) const
{
    if (hasAvailableDepthRange())
    {
        *minimumDepth = m_depthRangeMinimum;
        *maximumDepth = m_depthRangeMaximum;
    }
    else
    {
        *minimumDepth = RI_LOGPLOT_MINDEPTH_DEFAULT;
        *maximumDepth = RI_LOGPLOT_MAXDEPTH_DEFAULT;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlot::hasAvailableDepthRange() const
{
    return m_depthRangeMinimum < HUGE_VAL && m_depthRangeMaximum > -HUGE_VAL;
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
    updateTraces();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateTraces()
{
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
    if (m_viewer)
    {
        double minDepth = m_minimumVisibleDepth < HUGE_VAL ? m_minimumVisibleDepth : RI_LOGPLOT_MINDEPTH_DEFAULT;
        double maxDepth = m_maximumVisibleDepth > -HUGE_VAL ? m_maximumVisibleDepth : RI_LOGPLOT_MAXDEPTH_DEFAULT;

        m_viewer->setDepthRangeAndReplot(minDepth, maxDepth);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setVisibleDepthRangeFromContents()
{
    if (hasAvailableDepthRange())
    {
        setVisibleDepthRange(m_depthRangeMinimum, m_depthRangeMaximum);
    }
    else
    {
        setVisibleDepthRange(RI_LOGPLOT_MINDEPTH_DEFAULT, RI_LOGPLOT_MAXDEPTH_DEFAULT);
    }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateViewerWidgetWindowTitle()
{
    if (m_viewer)
    {
        m_viewer->setWindowTitle(m_userName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::handleViewerDeletion()
{
    m_showWindow = false;

    if (m_viewer)
    {
        detachAllCurves();
        m_viewer = NULL;
    }
 
    uiCapability()->updateUiIconFromToggleField();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::DepthTypeEnum RimWellLogPlot::depthType() const
{
    return m_depthType.value();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogPlot::depthPlotTitle() const
{
    QString depthTitle = "Depth";
    
    switch (m_depthType.value())
    {
        case MEASURED_DEPTH:
            depthTitle = "MD";
            break;

        case TRUE_VERTICAL_DEPTH:
            depthTitle = "TVD";
            break;
    }

    depthTitle += " [m]";
    return depthTitle;
}
