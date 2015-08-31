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

    m_viewer = NULL;
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
        m_viewer->setVisible(newValue == true);
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
void RimWellLogPlotTrace::addCurve(std::vector<double>& depthValues, std::vector<double>& values)
{
    CVF_ASSERT(m_viewer);

    RimWellLogPlotCurve* curve = new RimWellLogPlotCurve();
    curves.push_back(curve);

    curve->setPlot(m_viewer);
    curve->setUiName(QString("Curve %1").arg(curves.size()));
    curve->plot(depthValues, values);
    
    RiuMainWindow::instance()->projectTreeView()->setExpanded(this, true);
    updateConnectedEditors();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrace::setViewer(RiuWellLogTracePlot* viewer)
{
    if (m_viewer)
    {
        delete m_viewer;
    }

    m_viewer = viewer;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTracePlot* RimWellLogPlotTrace::viewer()
{
    return m_viewer;
}
