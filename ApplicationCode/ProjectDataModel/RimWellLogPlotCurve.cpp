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

#include "RimWellLogPlot.h"

#include "RimWellLogPlotTrack.h"

#include "RiuWellLogPlotCurve.h"
#include "RiuWellLogTrackPlot.h"

#include "cvfAssert.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimWellLogPlotCurve, "WellLogPlotCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve::RimWellLogPlotCurve()
{
    CAF_PDM_InitObject("Curve", "", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show", true, "Show curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curveName,        "CurveName",        "Curve Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_customCurveName,  "CurveDescription", "Custom Name", "", "", "");
    m_customCurveName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_autoName, "AutoName", true, "Auto Name", "", "", "");

    CAF_PDM_InitField(&m_curveColor, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");

    m_plotCurve = new RiuWellLogPlotCurve;
    m_plotCurve->setXAxis(QwtPlot::xTop);
    m_plotCurve->setYAxis(QwtPlot::yLeft);

    m_plot = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve::~RimWellLogPlotCurve()
{
    m_plotCurve->detach();    
    if (m_plot) m_plot->replot();

    delete m_plotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showCurve)
    {
       this->updateCurveVisibility();
    }
    else if (changedField == &m_curveName)
    {
        m_customCurveName = m_curveName;
        updatePlotTitle();
    }
    else if (&m_curveColor == changedField)
    {
        m_plotCurve->setPen(QPen(QColor(m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte())));
    }
    else if (changedField == &m_autoName)
    {
        if (!m_autoName)
        {
            m_customCurveName = createCurveName();
        }

        updateOptionSensitivity();
        updateCurveName();
        updatePlotTitle();
    }

    if (m_plot) m_plot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotCurve::objectToggleField()
{
    return &m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updateCurveVisibility()
{
    if (m_showCurve() && m_plot)
    {
        m_plotCurve->attach(m_plot);
    }
    else
    {
        m_plotCurve->detach();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updatePlotConfiguration()
{
    this->updateCurveVisibility();
    this->updateCurveName();
    this->updatePlotTitle();

    m_plotCurve->setPen(QPen(QColor(m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte())));
    // Todo: Rest of the curve setup controlled from this class
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::setPlot(RiuWellLogTrackPlot* plot)
{
    m_plot = plot;
    if (m_showCurve)
    {
        m_plotCurve->attach(m_plot);
        m_plot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotCurve::userDescriptionField()
{
    return &m_curveName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotCurve::depthRange(double* minimumDepth, double* maximumDepth) const
{
    CVF_ASSERT(minimumDepth && maximumDepth);
    CVF_ASSERT(m_plotCurve);

    if (m_plotCurve->data()->size() < 1)
    {
        return false;
    }

    *minimumDepth = m_plotCurve->minYValue();
    *maximumDepth = m_plotCurve->maxYValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotCurve::valueRange(double* minimumValue, double* maximumValue) const
{
    CVF_ASSERT(minimumValue && maximumValue);
    CVF_ASSERT(m_plotCurve);

    if (m_plotCurve->data()->size() < 1)
    {
        return false;
    }

    *minimumValue = m_plotCurve->minXValue();
    *maximumValue = m_plotCurve->maxXValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::setColor(const cvf::Color3f& color)
{
    m_curveColor = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::detachCurve()
{
    m_plotCurve->detach();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RimWellLogPlotCurve::plotCurve() const
{
    return m_plotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updatePlotTitle()
{
    m_plotCurve->setTitle(m_curveName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotCurve::isCurveVisibile()
{
    return m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::initAfterRead()
{
    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updateTrackAndPlotFromCurveData()
{
    RimWellLogPlot* wellLogPlot;
    firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        bool setDepthRange = !wellLogPlot->hasAvailableDepthRange();
        wellLogPlot->updateAvailableDepthRange();

        if (setDepthRange)
        {
            wellLogPlot->setVisibleDepthRangeFromContents();
        }
    }

    RimWellLogPlotTrack* plotTrack;
    firstAnchestorOrThisOfType(plotTrack);
    if (plotTrack)
    {
        plotTrack->updateAxisRangesAndReplot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updateCurveName()
{
    if (m_autoName)
    {
        m_curveName = this->createCurveName();
    }
    else
    {
        m_curveName = m_customCurveName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updateOptionSensitivity()
{
    m_curveName.uiCapability()->setUiReadOnly(m_autoName);
}
