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

#include "RimWellLogPlot.h"
#include "RimWellPathAttribute.h"
#include "RimWellPath.h"

#include "RigWellPath.h"
#include "RiuQwtPlotCurve.h"

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
    m_symbolLabelPosition = RiuQwtSymbol::LabelRightOfSymbol;
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

    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);
    RimWellLogPlot::DepthTypeEnum depthType = plot->depthType();

    if (m_wellPathAttribute)
    {
        RimWellPath* wellPath = nullptr;
        m_wellPathAttribute->firstAncestorOrThisOfTypeAsserted(wellPath);
        cvf::Vec3d startPoint = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_wellPathAttribute->depthStart());
        cvf::Vec3d endPoint = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_wellPathAttribute->depthEnd());

        double startTVD = -startPoint.z();
        double endTVD = -endPoint.z();

        double startDepth = depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH ? startTVD : m_wellPathAttribute->depthStart();
        double endDepth   = depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH ? endTVD   : m_wellPathAttribute->depthEnd();

        setCustomName(m_wellPathAttribute->label());

        double sign = m_curvePlotPosition == PositiveSide ? 1.0 : -1.0;
        double radius = 0.5 * sign * m_wellPathAttribute->diameterInInches();

        if (m_wellPathAttribute->type() == RimWellPathAttribute::AttributeCasing)
        {
            if (m_curvePlotItem == LineCurve)
            {
                setLineStyle(RiuQwtPlotCurve::STYLE_SOLID);
                setLineThickness(4);
                setSymbol(RiuQwtSymbol::SYMBOL_NONE);
                xValues = { radius, radius };
                yValues = { startDepth, endDepth };
            }
            else if (m_curvePlotItem == MarkerSymbol)
            {
                setLineStyle(RiuQwtPlotCurve::STYLE_NONE);
                setLineThickness(4);
                setSymbolSize(10);
                
                if (m_curvePlotPosition == PositiveSide)
                {
                    setSymbol(RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE);
                    setSymbolLabel(m_wellPathAttribute->diameterLabel());
                }
                else
                {
                    setSymbol(RiuQwtSymbol::SYMBOL_LEFT_TRIANGLE);
                }

                xValues = { radius };
                yValues = { endDepth };
            }
        }
        else if (m_wellPathAttribute->type() == RimWellPathAttribute::AttributeLiner)
        {
            setLineStyle(RiuQwtPlotCurve::STYLE_DASH);
            setLineThickness(2);

            xValues = { radius, radius};
            yValues = { startDepth, endDepth };
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
        RimWellLogPlot* plot = nullptr;
        firstAncestorOrThisOfTypeAsserted(plot);
        RimWellLogPlot::DepthTypeEnum depthType = plot->depthType();

        RimWellPath* wellPath = nullptr;
        m_wellPathAttribute->firstAncestorOrThisOfTypeAsserted(wellPath);
        cvf::Vec3d startPoint = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_wellPathAttribute->depthStart());
        cvf::Vec3d endPoint = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_wellPathAttribute->depthEnd());

        double startTVD = -startPoint.z();
        double endTVD = -endPoint.z();

        double startDepth = depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH ? startTVD : m_wellPathAttribute->depthStart();
        double endDepth = depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH ? endTVD : m_wellPathAttribute->depthEnd();

        *minimumValue = startDepth;
        *maximumValue = endDepth;
        return true;
    }
    return false;
}

