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

#include "RimWellLogTrack.h"

#include "RigStatisticsCalculator.h"
#include "RigWellLogCurveData.h"

#include "RimWellFlowRateCurve.h"
#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"

#include "RiuMainWindow.h"
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"

#include "cvfAssert.h"
#include "cvfMath.h"

#include "qwt_scale_engine.h"

#include <math.h>

#define RI_LOGPLOTTRACK_MINX_DEFAULT    -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT    100.0


CAF_PDM_SOURCE_INIT(RimWellLogTrack, "WellLogPlotTrack");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::RimWellLogTrack()
{
    CAF_PDM_InitObject("Track", ":/WellLogTrack16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_userName, "TrackDescription", "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_show, "Show", true, "Show track", "", "", "");
    m_show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&curves, "Curves", "",  "", "", "");
    curves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_visibleXRangeMin, "VisibleXRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min", "", "", "");
    CAF_PDM_InitField(&m_visibleXRangeMax, "VisibleXRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max", "", "", "");

    CAF_PDM_InitField(&m_isAutoScaleXEnabled, "AutoScaleX", true, "Auto Scale", "", "", "");

    CAF_PDM_InitField(&m_isLogarithmicScaleEnabled, "LogarithmicScaleX", false, "Logarithmic Scale", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::~RimWellLogTrack()
{
    curves.deleteAllChildObjects();

    if (m_wellLogTrackPlotWidget) 
    {
        m_wellLogTrackPlotWidget->deleteLater();
        m_wellLogTrackPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_show)
    {
        if (m_wellLogTrackPlotWidget)
        {
            m_wellLogTrackPlotWidget->setVisible(m_show());
        }

        RimWellLogPlot* wellLogPlot;
        this->firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            wellLogPlot->calculateAvailableDepthRange();
            wellLogPlot->updateDepthZoom();

            RiuWellLogPlot* wellLogPlotViewer = dynamic_cast<RiuWellLogPlot*>(wellLogPlot->viewWidget());
            if (wellLogPlotViewer)
            {
                wellLogPlotViewer->updateChildrenLayout();
            }
        }
    }
    else if (changedField == &m_visibleXRangeMin || changedField == &m_visibleXRangeMax)
    {
        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);
        m_wellLogTrackPlotWidget->replot();
        m_isAutoScaleXEnabled = false;
    }
    else if (changedField == &m_isAutoScaleXEnabled)
    {
        if (m_isAutoScaleXEnabled())
        { 
            this->updateXZoom();
            computeAndSetXRangeMinForLogarithmicScale();

            if (m_wellLogTrackPlotWidget) m_wellLogTrackPlotWidget->replot();
        }
    }
    else if (changedField == &m_isLogarithmicScaleEnabled)
    {
        updateAxisScaleEngine();

        this->updateXZoom();
        computeAndSetXRangeMinForLogarithmicScale();

        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);

        m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogTrack::objectToggleField()
{
    return &m_show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogTrack::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::addCurve(RimWellLogCurve* curve)
{
    curves.push_back(curve);

    if (m_wellLogTrackPlotWidget)
    {
        curve->setParentQwtPlot(m_wellLogTrackPlotWidget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::insertCurve(RimWellLogCurve* curve, size_t index)
{
    curves.insert(index, curve);
    // Todo: Mark curve data to use either TVD or MD

    if (m_wellLogTrackPlotWidget)
    {
        curve->setParentQwtPlot(m_wellLogTrackPlotWidget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::removeCurve(RimWellLogCurve* curve)
{
    size_t index = curves.index(curve);
    if ( index < curves.size())
    {
        curves[index]->detachQwtCurve();
        curves.removeChildObject(curve);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack* RimWellLogTrack::viewer()
{
    return m_wellLogTrackPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::availableDepthRange(double* minimumDepth, double* maximumDepth)
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    size_t curveCount = curves.size();

    for (size_t cIdx = 0; cIdx < curveCount; cIdx++)
    {
        double minCurveDepth = HUGE_VAL;
        double maxCurveDepth = -HUGE_VAL;

        if (curves[cIdx]->isCurveVisible() && curves[cIdx]->depthRange(&minCurveDepth, &maxCurveDepth))
        {
            if (minCurveDepth < minDepth)
            {
                minDepth = minCurveDepth;
            }

            if (maxCurveDepth > maxDepth)
            {
                maxDepth = maxCurveDepth;
            }
        }
    }

    *minimumDepth = minDepth;
    *maximumDepth = maxDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::loadDataAndUpdate()
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType(wellLogPlot);
    if (wellLogPlot && m_wellLogTrackPlotWidget)
    {
        m_wellLogTrackPlotWidget->setDepthTitle(wellLogPlot->depthPlotTitle());
        m_wellLogTrackPlotWidget->setXTitle(m_xAxisTitle);
    }

    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setXAxisTitle(const QString& text)
{
    m_xAxisTitle = text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::recreateViewer()
{
    if (m_wellLogTrackPlotWidget == NULL)
    {
        m_wellLogTrackPlotWidget = new RiuWellLogTrack(this);
        updateAxisScaleEngine();

        for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
        {
            curves[cIdx]->setParentQwtPlot(this->m_wellLogTrackPlotWidget);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::detachAllCurves()
{
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateXZoomAndParentPlotDepthZoom()
{
    if (m_wellLogTrackPlotWidget)
    {
        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
           wellLogPlot->updateDepthZoom();
        }

        updateXZoom();

        m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateXZoom()
{
    std::vector<RimWellFlowRateCurve*> stackCurves = visibleStackedCurves();
    for (RimWellFlowRateCurve* stCurve: stackCurves) stCurve->updateStackedPlotData();

    if (!m_isAutoScaleXEnabled())
    {
        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);
        m_wellLogTrackPlotWidget->replot();
        return;
    }

    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
    {
        double minCurveValue = HUGE_VAL;
        double maxCurveValue = -HUGE_VAL;

        if (curves[cIdx]->isCurveVisible() && curves[cIdx]->valueRange(&minCurveValue, &maxCurveValue))
        {
            if (minCurveValue < minValue)
            {
                minValue = minCurveValue;
            }

            if (maxCurveValue > maxValue)
            {
                maxValue = maxCurveValue;
            }
        }
    }

    if (minValue == HUGE_VAL)
    {
        minValue = RI_LOGPLOTTRACK_MINX_DEFAULT;
        maxValue = RI_LOGPLOTTRACK_MAXX_DEFAULT;
    }

    m_visibleXRangeMin = minValue;
    m_visibleXRangeMax = maxValue;

    computeAndSetXRangeMinForLogarithmicScale();

    if (m_wellLogTrackPlotWidget) m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogCurve* RimWellLogTrack::curveDefinitionFromCurve(const QwtPlotCurve* curve) const
{
    for (size_t idx = 0; idx < curves.size(); idx++)
    {
        if (curves[idx]->qwtPlotCurve() == curve)
        {
            return curves[idx];
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_userName);

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Visible X Axis Range");
    gridGroup->add(&m_isAutoScaleXEnabled);
    gridGroup->add(&m_isLogarithmicScaleEnabled);
    gridGroup->add(&m_visibleXRangeMin);
    gridGroup->add(&m_visibleXRangeMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimWellLogTrack::curveIndex(RimWellLogCurve* curve)
{
    return curves.index(curve);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::isVisible()
{
    return m_show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAxisScaleEngine()
{
    if (m_isLogarithmicScaleEnabled)
    {
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xTop, new QwtLogScaleEngine);
        
        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
    }
    else
    {
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xTop, new QwtLinearScaleEngine);

        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_wellLogTrackPlotWidget->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::computeAndSetXRangeMinForLogarithmicScale()
{
    if (m_isAutoScaleXEnabled && m_isLogarithmicScaleEnabled)
    {
        double pos = HUGE_VAL;
        double neg = -HUGE_VAL;

        for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
        {
            if (curves[cIdx]->isCurveVisible() && curves[cIdx]->curveData())
            {
                RigStatisticsCalculator::posNegClosestToZero(curves[cIdx]->curveData()->xPlotValues(), pos, neg);
            }
        }

        if (pos != HUGE_VAL)
        {
            m_visibleXRangeMin = pos;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setLogarithmicScale(bool enable)
{
    m_isLogarithmicScaleEnabled = enable;

    updateAxisScaleEngine();
    computeAndSetXRangeMinForLogarithmicScale();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellFlowRateCurve*> RimWellLogTrack::visibleStackedCurves()
{
    std::vector<RimWellFlowRateCurve*> stackedCurves;
    for (RimWellLogCurve* curve: curves)
    {
        if (curve && curve->isCurveVisible() )
        {
            RimWellFlowRateCurve* wfrCurve = dynamic_cast<RimWellFlowRateCurve*>(curve);
            if (wfrCurve) stackedCurves.push_back(wfrCurve);
        }
    }

    return stackedCurves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::description()
{
    return m_userName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve* > RimWellLogTrack::curvesVector()
{
    std::vector<RimWellLogCurve* > curvesVector;

    for (RimWellLogCurve* curve : curves)
    {
        curvesVector.push_back(curve);
    }

    return curvesVector;
}

