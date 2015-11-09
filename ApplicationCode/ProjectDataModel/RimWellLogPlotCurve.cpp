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
    CAF_PDM_InitObject("Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show", true, "Show curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curveName,        "CurveName",        "Curve Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_customCurveName,  "CurveDescription", "Custom Name", "", "", "");
    m_customCurveName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_autoName, "AutoName", true, "Auto Name", "", "", "");

    CAF_PDM_InitField(&m_curveColor, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");

    m_qwtPlotCurve = new RiuWellLogPlotCurve;
    m_qwtPlotCurve->setXAxis(QwtPlot::xTop);
    m_qwtPlotCurve->setYAxis(QwtPlot::yLeft);

    m_ownerQwtTrack = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve::~RimWellLogPlotCurve()
{
    m_qwtPlotCurve->detach();    
    delete m_qwtPlotCurve;

    if (m_ownerQwtTrack)
    {
        m_ownerQwtTrack->replot();
    }
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
        m_qwtPlotCurve->setPen(QPen(QColor(m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte())));
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

    if (m_ownerQwtTrack) m_ownerQwtTrack->replot();
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
    if (m_showCurve() && m_ownerQwtTrack)
    {
        m_qwtPlotCurve->attach(m_ownerQwtTrack);
    }
    else
    {
        m_qwtPlotCurve->detach();
    }

    RimWellLogPlot* wellLogPlot;
    this->firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        wellLogPlot->calculateAvailableDepthRange();
    }

    RimWellLogPlotTrack* wellLogPlotTrack;
    this->firstAnchestorOrThisOfType(wellLogPlotTrack);
    if (wellLogPlotTrack)
    {
        wellLogPlotTrack->zoomAllXAndZoomAllDepthOnOwnerPlot();
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

    m_qwtPlotCurve->setPen(QPen(QColor(m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte())));
    // Todo: Rest of the curve setup controlled from this class
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::setQwtTrack(RiuWellLogTrackPlot* plot)
{
    m_ownerQwtTrack = plot;
    if (m_showCurve)
    {
        m_qwtPlotCurve->attach(m_ownerQwtTrack);
        m_ownerQwtTrack->replot();
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
bool RimWellLogPlotCurve::valueRange(double* minimumValue, double* maximumValue) const
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
void RimWellLogPlotCurve::setColor(const cvf::Color3f& color)
{
    m_curveColor = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::detachQwtCurve()
{
    m_qwtPlotCurve->detach();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RimWellLogPlotCurve::plotCurve() const
{
    return m_qwtPlotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updatePlotTitle()
{
    m_qwtPlotCurve->setTitle(m_curveName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotCurve::isCurveVisible() const
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
void RimWellLogPlotCurve::zoomAllOwnerTrackAndPlot()
{
    RimWellLogPlot* wellLogPlot;
    firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        wellLogPlot->calculateAvailableDepthRange();
        wellLogPlot->zoomAllDepth();
    }

    RimWellLogPlotTrack* plotTrack;
    firstAnchestorOrThisOfType(plotTrack);
    if (plotTrack)
    {
        plotTrack->zoomAllXAndZoomAllDepthOnOwnerPlot();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellLogCurveData* RimWellLogPlotCurve::curveData() const
{
    return m_curveData.p();
}
