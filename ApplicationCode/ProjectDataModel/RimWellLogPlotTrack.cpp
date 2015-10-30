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

#include "RimWellLogPlotTrack.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotCurve.h"

#include "RiuWellLogTrackPlot.h"
#include "RiuWellLogPlot.h"
#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"
#include "cvfAssert.h"

#include <math.h>

#define RI_LOGPLOTTRACK_MINX_DEFAULT    -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT    100.0


CAF_PDM_SOURCE_INIT(RimWellLogPlotTrack, "WellLogPlotTrack");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrack::RimWellLogPlotTrack()
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrack::~RimWellLogPlotTrack()
{
    delete m_wellLogTrackPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_show)
    {
        if (m_wellLogTrackPlotWidget)
        {
            m_wellLogTrackPlotWidget->setVisible(m_show());
        }

        RimWellLogPlot* wellLogPlot;
        this->firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            wellLogPlot->calculateAvailableDepthRange();
            wellLogPlot->zoomAllDepth();
            if (wellLogPlot->viewer()) wellLogPlot->viewer()->updateChildrenLayout();
        }
    }
    else if (changedField == &m_visibleXRangeMin || changedField == &m_visibleXRangeMax)
    {
        m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);
        m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotTrack::objectToggleField()
{
    return &m_show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotTrack::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::addCurve(RimWellLogPlotCurve* curve)
{
    curves.push_back(curve);

    if (m_wellLogTrackPlotWidget)
    {
        curve->setQwtTrack(m_wellLogTrackPlotWidget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::insertCurve(RimWellLogPlotCurve* curve, size_t index)
{
    curves.insert(index, curve);
    // Todo: Mark curve data to use either TVD or MD

    if (m_wellLogTrackPlotWidget)
    {
        curve->setQwtTrack(m_wellLogTrackPlotWidget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::removeCurve(RimWellLogPlotCurve* curve)
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
RiuWellLogTrackPlot* RimWellLogPlotTrack::viewer()
{
    return m_wellLogTrackPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::availableDepthRange(double* minimumDepth, double* maximumDepth)
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
void RimWellLogPlotTrack::loadDataAndUpdate()
{
    CVF_ASSERT(m_wellLogTrackPlotWidget);

    RimWellLogPlot* wellLogPlot;
    firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        m_wellLogTrackPlotWidget->setDepthTitle(wellLogPlot->depthPlotTitle());
    }

    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->updatePlotData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::recreateViewer()
{
    if (m_wellLogTrackPlotWidget == NULL)
    {
        m_wellLogTrackPlotWidget = new RiuWellLogTrackPlot(this);

        for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
        {
            curves[cIdx]->setQwtTrack(this->m_wellLogTrackPlotWidget);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::detachAllCurves()
{
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::zoomAllXAndZoomAllDepthOnOwnerPlot()
{
    if (m_wellLogTrackPlotWidget)
    {
        RimWellLogPlot* wellLogPlot;
        firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
           wellLogPlot->zoomAllDepth();
        }

        zoomAllXAxis();

        m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::alignDepthZoomToPlotAndZoomAllX()
{
    if (m_wellLogTrackPlotWidget)
    {
        RimWellLogPlot* wellLogPlot;
        firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            double minimumDepth, maximumDepth;
            wellLogPlot->depthZoomMinMax(&minimumDepth, &maximumDepth);

            m_wellLogTrackPlotWidget->setDepthZoom(minimumDepth, maximumDepth);
        }

        zoomAllXAxis();

        m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::zoomAllXAxis()
{
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

    if (m_wellLogTrackPlotWidget) m_wellLogTrackPlotWidget->setXRange(m_visibleXRangeMin, m_visibleXRangeMax);

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve* RimWellLogPlotTrack::curveDefinitionFromCurve(const QwtPlotCurve* curve) const
{
    for (size_t idx = 0; idx < curves.size(); idx++)
    {
        if (curves[idx]->plotCurve() == curve)
        {
            return curves[idx];
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_userName);

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Visible X Axis Range");
    gridGroup->add(&m_visibleXRangeMin);
    gridGroup->add(&m_visibleXRangeMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimWellLogPlotTrack::curveIndex(RimWellLogPlotCurve* curve)
{
    return curves.index(curve);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotTrack::isVisible()
{
    return m_show;
}
