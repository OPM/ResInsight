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
    
    CAF_PDM_InitField(&minimumDepth, "MinimumDepth", 0.0, "Set minimum depth", "", "", "");
    CAF_PDM_InitField(&maximumDepth, "MaximumDepth", 1000.0, "Set maximum depth", "", "", "");

    CAF_PDM_InitFieldNoDefault(&traces, "Traces", "",  "", "", "");
    traces.uiCapability()->setUiHidden(true);

    updateViewerWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::~RimWellLogPlot()
{
    RiuMainWindow::instance()->removeWellLogViewer(m_viewer);
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
            m_viewer = new RiuWellLogPlot(RiuMainWindow::instance());

            RiuMainWindow::instance()->addWellLogViewer(m_viewer);
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

        RiuMainWindow::instance()->setActiveWellLogViewer(m_viewer);
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
    else if (changedField == &minimumDepth || changedField == &maximumDepth)
    {
        m_viewer->setDepthRange(minimumDepth, maximumDepth);
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
void RimWellLogPlot::addTrace()
{
    RimWellLogPlotTrace* trace = new RimWellLogPlotTrace();
    traces.push_back(trace);

    trace->setUiName(QString("Trace %1").arg(traces.size()));

    RiuWellLogTracePlot* viewer = m_viewer->createTracePlot();
    trace->setViewer(viewer);

    updateConnectedEditors();
    RiuMainWindow::instance()->setCurrentObjectInTreeView(trace);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot* RimWellLogPlot::viewer()
{
    return m_viewer;
}
