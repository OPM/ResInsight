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

#include "RimWellLogPlotCurve.h"

#include "RiuWellLogTracePlot.h"

#include "qwt_plot_curve.h"

#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT(RimWellLogPlotCurve, "WellLogPlotCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve::RimWellLogPlotCurve()
{
    CAF_PDM_InitObject("Curve", "", "", "");

    CAF_PDM_InitField(&show, "Show", true, "Show curve", "", "", "");
    show.uiCapability()->setUiHidden(true);

    m_plotCurve = new QwtPlotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve::~RimWellLogPlotCurve()
{
    m_plotCurve->detach();    
    m_plot->replot();

    delete m_plotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotCurve::objectToggleField()
{
    return &show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::plot(std::vector<double> depthValues, std::vector<double> values)
{
    CVF_ASSERT(m_plot);

    m_depthValues = depthValues;
    m_values = values;

    m_plotCurve->setTitle(this->uiName());
    m_plotCurve->setSamples(values.data(), depthValues.data(), (int) depthValues.size());
    m_plotCurve->attach(m_plot);
    m_plot->replot();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimWellLogPlotCurve::pointCount() const
{
    return m_depthValues.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const double* RimWellLogPlotCurve::depthValues() const
{
    return m_depthValues.data();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const double* RimWellLogPlotCurve::values() const
{
    return m_values.data();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::setPlot(RiuWellLogTracePlot* plot)
{
    m_plot = plot;
}
