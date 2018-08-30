/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimWellPathAttributeCurve.h"

#include "RimWellPathAttribute.h"

#include "RiuLineSegmentQwtPlotCurve.h"

#include "qwt_plot.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellPathAttributeCurve, "RimWellPathAttributeCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttributeCurve::RimWellPathAttributeCurve(RimWellPathAttribute* wellPathAttribute,
                                                     CurvePlotPosition     plotPosition,
                                                     CurvePlotItem         curvePlotItem)
    : m_curvePlotPosition(plotPosition)
    , m_curvePlotItem(curvePlotItem)
{
    CAF_PDM_InitObject("Well Attribute Curve", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellPathAttribute, "WellPathAttribute", "Well Attribute", "", "", "");
    m_wellPathAttribute.xmlCapability()->disableIO();
    m_wellPathAttribute = wellPathAttribute;
    m_symbolLabelPosition = RiuCurveQwtSymbol::LabelRightOfSymbol;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttributeCurve::~RimWellPathAttributeCurve()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCurve::updateZoomInParentPlot()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathAttributeCurve::createCurveAutoName()
{
    CVF_ASSERT(m_wellPathAttribute);
    return m_wellPathAttribute->label();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    m_qwtPlotCurve->setXAxis(QwtPlot::xBottom);

    std::vector<double> xValues;
    std::vector<double> yValues;
    if (m_wellPathAttribute)
    {
        setCustomName(m_wellPathAttribute->label());

        double sign = m_curvePlotPosition == PositiveSide ? 1.0 : -1.0;
        double radius = 0.5 * sign * m_wellPathAttribute->diameterInInches();

        if (m_wellPathAttribute->type() == RimWellPathAttribute::AttributeCasing)
        {
            if (m_curvePlotItem == LineCurve)
            {
                setLineStyle(RimPlotCurve::STYLE_SOLID);
                setLineThickness(4);
                setSymbol(RimPlotCurve::SYMBOL_NONE);
                xValues = { radius, radius };
                yValues = { 0.0, m_wellPathAttribute->depthEnd() };
            }
            else if (m_curvePlotItem == MarkerSymbol)
            {
                setLineStyle(RimPlotCurve::STYLE_NONE);
                setLineThickness(0);
                setSymbol(RimPlotCurve::SYMBOL_TRIANGLE);
                setSymbolSize(16);
                
                if (m_curvePlotPosition == PositiveSide)
                {
                    setSymbolLabel(m_wellPathAttribute->diameterLabel());
                }

                xValues = { radius };
                yValues = { m_wellPathAttribute->depthEnd() };
            }
        }
        else if (m_wellPathAttribute->type() == RimWellPathAttribute::AttributeLiner)
        {
            setLineStyle(RimPlotCurve::STYLE_DASH);
            setLineThickness(2);

            xValues = { radius, radius};
            yValues = { m_wellPathAttribute->depthStart(), m_wellPathAttribute->depthEnd() };
        }
    }
    if (!xValues.empty())
    {
        CVF_ASSERT(xValues.size() == yValues.size());
        m_qwtPlotCurve->setSamples(&xValues[0], &yValues[0], static_cast<int>(xValues.size()));
        RimPlotCurve::updateCurvePresentation(updateParentPlot);
    }

    if (updateParentPlot)
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathAttributeCurve::yValueRange(double* minimumValue, double* maximumValue) const
{
    if (m_wellPathAttribute)
    {
        *minimumValue = m_wellPathAttribute->depthStart();
        *maximumValue = m_wellPathAttribute->depthEnd();
        return true;
    }
    return false;
}

