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
#include "RiuMainWindow.h"

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

    CAF_PDM_InitFieldNoDefault(&traces, "Traces", "",  "", "", "");
    traces.uiCapability()->setUiHidden(true);

    updateViewerWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::~RimWellLogPlot()
{
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
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlot::objectToggleField()
{
    return &showWindow;
}
