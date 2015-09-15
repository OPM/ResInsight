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

#include "RimWellLogFileCurve.h"

#include "qwt_plot_curve.h"


CAF_PDM_SOURCE_INIT(RimWellLogFileCurve, "WellLogFileCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::RimWellLogFileCurve()
{
    CAF_PDM_InitObject("Well Log File Curve", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::~RimWellLogFileCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::updatePlotData()
{
    RimWellLogPlotCurve::updatePlotData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setCurveData(const std::vector<double>& values, const std::vector<double>& depthValues)
{
    m_plotCurve->setSamples(values.data(), depthValues.data(), (int) depthValues.size());
}
