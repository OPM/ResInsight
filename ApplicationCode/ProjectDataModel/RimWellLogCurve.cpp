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

#include "RimWellLogCurve.h"

#include "RigWellLogCurveData.h" 

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuWellLogTrack.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_symbol.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimWellLogCurve, "WellLogPlotCurve");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::RimWellLogCurve()
{
    CAF_PDM_InitObject("WellLogCurve", ":/WellLogCurve16x16.png", "", "");

    m_qwtPlotCurve->setXAxis(QwtPlot::xTop);
    m_qwtPlotCurve->setYAxis(QwtPlot::yLeft);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::~RimWellLogCurve()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::depthRange(double* minimumDepth, double* maximumDepth) const
{
    CVF_ASSERT(minimumDepth && maximumDepth);
    CVF_ASSERT(m_qwtPlotCurve);
    
    if (m_qwtPlotCurve->data()->size() < 1)
    {
        return false;
    }

    *minimumDepth = m_qwtPlotCurve->minYValue();
    *maximumDepth = m_qwtPlotCurve->maxYValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::valueRange(double* minimumValue, double* maximumValue) const
{
    CVF_ASSERT(minimumValue && maximumValue);
    CVF_ASSERT(m_qwtPlotCurve);

    if (m_qwtPlotCurve->data()->size() < 1)
    {
        return false;
    }

    *minimumValue = m_qwtPlotCurve->minXValue();
    *maximumValue = m_qwtPlotCurve->maxXValue();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateZoomInParentPlot()
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        wellLogPlot->calculateAvailableDepthRange();
        wellLogPlot->updateDepthZoom();
    }

    RimWellLogTrack* plotTrack;
    firstAncestorOrThisOfType(plotTrack);
    if (plotTrack)
    {
        plotTrack->updateXZoomAndParentPlotDepthZoom();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellLogCurveData* RimWellLogCurve::curveData() const
{
    return m_curveData.p();
}
