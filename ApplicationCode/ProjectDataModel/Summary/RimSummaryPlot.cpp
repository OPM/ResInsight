/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryPlot.h"

#include "RiaApplication.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryCurvesCalculator.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryTimeAxisProperties.h"
#include "RimSummaryYAxisProperties.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionColors.h"
#include "RiuSummaryQwtPlot.h"

#include "cvfBase.h"
#include "cvfColor3.h"

#include "cafPdmUiTreeOrdering.h"

#include <QDateTime>
#include <QRectF>

#include "qwt_plot_curve.h"
#include "qwt_plot_renderer.h"


CAF_PDM_SOURCE_INIT(RimSummaryPlot, "SummaryPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot()
{
    CAF_PDM_InitObject("Summary Plot", ":/SummaryPlot16x16.png", "", "");
    CAF_PDM_InitField(&m_showWindow, "ShowWindow", true, "Show Summary Plot", "", "", "");
    m_showWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Summary Plot"), "Name", "", "", "");
    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curveFilters, "SummaryCurveFilters", "", "", "", "");
    m_curveFilters.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curves, "SummaryCurves", "",  "", "", "");
    m_curves.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_leftYAxisProperties, "LeftYAxisProperties", "Left Y Axis", "", "", "");
    m_leftYAxisProperties.uiCapability()->setUiTreeHidden(true);

    m_leftYAxisPropertiesObject = std::unique_ptr<RimSummaryYAxisProperties>(new RimSummaryYAxisProperties);
    m_leftYAxisPropertiesObject->setNameAndAxis("Left Y-Axis", QwtPlot::yLeft);
    m_leftYAxisProperties = m_leftYAxisPropertiesObject.get();

    CAF_PDM_InitFieldNoDefault(&m_rightYAxisProperties, "RightYAxisProperties", "Right Y Axis", "", "", "");
    m_rightYAxisProperties.uiCapability()->setUiTreeHidden(true);

    m_rightYAxisPropertiesObject = std::unique_ptr<RimSummaryYAxisProperties>(new RimSummaryYAxisProperties);
    m_rightYAxisPropertiesObject->setNameAndAxis("Right Y-Axis", QwtPlot::yRight);
    m_rightYAxisProperties = m_rightYAxisPropertiesObject.get();

    CAF_PDM_InitFieldNoDefault(&m_timeAxisProperties, "TimeAxisProperties", "Time Axis", "", "", "");
    m_timeAxisProperties.uiCapability()->setUiTreeHidden(true);

    m_timeAxisPropertiesObject = std::unique_ptr<RimSummaryTimeAxisProperties>(new RimSummaryTimeAxisProperties);
    m_timeAxisProperties = m_timeAxisPropertiesObject.get();

    CAF_PDM_InitField(&m_isAutoZoom, "AutoZoom", true, "Auto Zoom", "", "", "");
    m_isAutoZoom.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::~RimSummaryPlot()
{
    if (RiaApplication::instance()->mainPlotWindow())
    {
        RiaApplication::instance()->mainPlotWindow()->removeViewer(m_qwtPlot);
    }

    deletePlotWidget();

    m_curves.deleteAllChildObjects();
    m_curveFilters.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deletePlotWidget()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->deleteLater();
        m_qwtPlot = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxes()
{
    updateAxis(RimDefines::PLOT_AXIS_LEFT);
    updateAxis(RimDefines::PLOT_AXIS_RIGHT);

    updateZoomInQwt();

    updateTimeAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isLogarithmicScaleEnabled(RimDefines::PlotAxis plotAxis) const
{
    if (plotAxis == RimDefines::PLOT_AXIS_LEFT)
    {
        return m_leftYAxisProperties->isLogarithmicScaleEnabled();
    }
    else
    {
        return m_rightYAxisProperties->isLogarithmicScaleEnabled();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties* RimSummaryPlot::timeAxisProperties()
{
    return m_timeAxisProperties();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::selectAxisInPropertyEditor(int axis)
{
    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
    if (axis == QwtPlot::yLeft)
    {
        plotwindow->selectAsCurrentItem(m_leftYAxisProperties);
    }
    else if (axis == QwtPlot::yRight)
    {
        plotwindow->selectAsCurrentItem(m_rightYAxisProperties);
    }
    else if (axis == QwtPlot::xBottom)
    {
        plotwindow->selectAsCurrentItem(m_timeAxisProperties);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
time_t RimSummaryPlot::firstTimeStepOfFirstCurve()
{
    RimSummaryCurve * firstCurve = nullptr;

    for (RimSummaryCurveFilter* curveFilter : m_curveFilters )
    {
        if (curveFilter)
        {
            std::vector<RimSummaryCurve *> curves = curveFilter->curves();
            size_t i = 0;
            while (firstCurve == nullptr && i < curves.size())
            {
                firstCurve = curves[i];
                i++;
            }

            if (firstCurve) break;
        }
    }

    size_t i = 0;
    while (firstCurve == nullptr && i < m_curves.size())
    {
        firstCurve = m_curves[i];
        ++i;
    }

    if (firstCurve && firstCurve->timeSteps().size() > 0)
    {
        return firstCurve->timeSteps()[0];
    }
    else return time_t(0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryPlot::viewWidget()
{
    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxis(RimDefines::PlotAxis plotAxis)
{
    if (!m_qwtPlot) return;

    QwtPlot::Axis qwtAxis = QwtPlot::yLeft;
    RimSummaryYAxisProperties* yAxisProperties = nullptr;
    if (plotAxis == RimDefines::PLOT_AXIS_LEFT)
    {
        qwtAxis = QwtPlot::yLeft;
        yAxisProperties = m_leftYAxisProperties();
    }
    else
    {
        qwtAxis = QwtPlot::yRight;
        yAxisProperties = m_rightYAxisProperties();
    }

    if (!yAxisProperties->isActive())
    {
        m_qwtPlot->enableAxis(qwtAxis, false);
    }
    else
    {
        if (hasVisibleCurvesForAxis(plotAxis))
        {
            std::vector<RimSummaryCurve*> curves;
            curves.insert(curves.begin(), m_curves.begin(), m_curves.end());

            std::vector<RimSummaryCurveFilter*> curveFilters;
            curveFilters.insert(curveFilters.begin(), m_curveFilters.begin(), m_curveFilters.end());

            m_qwtPlot->enableAxis(qwtAxis, true);

            RimSummaryPlotYAxisFormater calc(yAxisProperties, curves, curveFilters);
            calc.applyYAxisPropertiesToPlot(m_qwtPlot);
        }
        else
        {
            m_qwtPlot->enableAxis(qwtAxis, false);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::curvesForAxis(RimDefines::PlotAxis plotAxis) const
{
    std::vector<RimSummaryCurve*> curves;

    std::vector<RimSummaryCurve*> childCurves;
    this->descendantsIncludingThisOfType(childCurves);

    for (RimSummaryCurve* curve : childCurves)
    {
        if (curve->yAxis() == plotAxis)
        {
            curves.push_back(curve);
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::hasVisibleCurvesForAxis(RimDefines::PlotAxis plotAxis) const
{
    for (RimSummaryCurve* curve : m_curves)
    {
        if (curve->isCurveVisible() && curve->yAxis() == plotAxis) return true;
    }

    for (RimSummaryCurveFilter * curveFilter : m_curveFilters)
    {
        if (curveFilter->isCurvesVisible())
        {
            for ( RimSummaryCurve* curve : curveFilter->curves() )
            {
                if ( curve->isCurveVisible() && curve->yAxis() == plotAxis ) return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateTimeAxis()
{
    if (!m_qwtPlot) return;

    if (!m_timeAxisProperties->isActive())
    {
        m_qwtPlot->enableAxis(QwtPlot::xBottom, false);

        return;
    }

    if (m_timeAxisProperties->timeMode() == RimSummaryTimeAxisProperties::DATE)
    {
        m_qwtPlot->useDateBasedTimeAxis();
    }
    else 
    {
        m_qwtPlot->useTimeBasedTimeAxis();
    }   

    m_qwtPlot->enableAxis(QwtPlot::xBottom, true);

    {
        QString axisTitle;
        if (m_timeAxisProperties->showTitle) axisTitle = m_timeAxisProperties->title();

        QwtText timeAxisTitle = m_qwtPlot->axisTitle(QwtPlot::xBottom);

        QFont font = timeAxisTitle.font();
        font.setBold(true);
        font.setPixelSize(m_timeAxisProperties->fontSize);
        timeAxisTitle.setFont(font);

        timeAxisTitle.setText(axisTitle);

        switch ( m_timeAxisProperties->titlePositionEnum() )
        {
            case RimSummaryTimeAxisProperties::AXIS_TITLE_CENTER:
            timeAxisTitle.setRenderFlags(Qt::AlignCenter);
            break;
            case RimSummaryTimeAxisProperties::AXIS_TITLE_END:
            timeAxisTitle.setRenderFlags(Qt::AlignRight);
            break;
        }

        m_qwtPlot->setAxisTitle(QwtPlot::xBottom, timeAxisTitle);
    }

    {
        QFont timeAxisFont = m_qwtPlot->axisFont(QwtPlot::xBottom);
        timeAxisFont.setBold(false);
        timeAxisFont.setPixelSize(m_timeAxisProperties->fontSize);
        m_qwtPlot->setAxisFont(QwtPlot::xBottom, timeAxisFont);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::handleViewerDeletion()
{
    m_showWindow = false;

    if (m_qwtPlot)
    {
        detachAllCurves();
    }

    uiCapability()->updateUiIconFromToggleField();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateCaseNameHasChanged()
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->updateCurveName();
        curve->updateConnectedEditors();
    }

    for (RimSummaryCurveFilter* curveFilter : m_curveFilters)
    {
        curveFilter->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setZoomWindow(const QwtInterval& leftAxis, const QwtInterval& rightAxis, const QwtInterval& timeAxis)
{
    m_leftYAxisProperties->visibleRangeMax = leftAxis.maxValue();
    m_leftYAxisProperties->visibleRangeMin = leftAxis.minValue();
    m_leftYAxisProperties->updateConnectedEditors();

    m_rightYAxisProperties->visibleRangeMax = rightAxis.maxValue();
    m_rightYAxisProperties->visibleRangeMin = rightAxis.minValue();
    m_rightYAxisProperties->updateConnectedEditors();

    m_timeAxisProperties->setVisibleRangeMin(timeAxis.minValue());
    m_timeAxisProperties->setVisibleRangeMax(timeAxis.maxValue());
    m_timeAxisProperties->updateConnectedEditors();

    disableAutoZoom();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::zoomAll()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);

        if (m_leftYAxisProperties->isLogarithmicScaleEnabled)
        {
            std::vector<RimSummaryCurve*> curves = curvesForAxis(RimDefines::PLOT_AXIS_LEFT);

            double min, max;
            RimSummaryPlotYAxisRangeCalculator calc(m_leftYAxisProperties, curves);
            calc.computeYRange(&min, &max);

            m_qwtPlot->setAxisScale(m_leftYAxisProperties->qwtPlotAxisType(), min, max);
        }
        else
        {
            m_qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
        }

        if (m_rightYAxisProperties->isLogarithmicScaleEnabled)
        {
            std::vector<RimSummaryCurve*> curves = curvesForAxis(RimDefines::PLOT_AXIS_RIGHT);

            double min, max;
            RimSummaryPlotYAxisRangeCalculator calc(m_rightYAxisProperties, curves);
            calc.computeYRange(&min, &max);

            m_qwtPlot->setAxisScale(m_rightYAxisProperties->qwtPlotAxisType(), min, max);
        }
        else
        {
            m_qwtPlot->setAxisAutoScale(QwtPlot::yRight, true);
        }

        m_qwtPlot->replot();
    }

    updateZoomFromQwt();

    m_isAutoZoom = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurve(RimSummaryCurve* curve)
{
    if (curve)
    {
        m_curves.push_back(curve);
        if (m_qwtPlot)
        {
            curve->setParentQwtPlot(m_qwtPlot);
            this->updateAxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveFilter(RimSummaryCurveFilter* curveFilter)
{
    if(curveFilter)
    {
        m_curveFilters.push_back(curveFilter);
        if(m_qwtPlot)
        {
            curveFilter->setParentQwtPlot(m_qwtPlot);
            this->updateAxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showWindow)
    {
        if (m_showWindow)
        {
            loadDataAndUpdate();
        }
        else
        {
            updateViewerWidget();
        }

        uiCapability()->updateUiIconFromToggleField();
    }
    else if (changedField == &m_userName || 
             changedField == &m_showPlotTitle)
    {
        updateViewerWidgetWindowTitle();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setupBeforeSave()
{
    if (m_qwtPlot && RiaApplication::instance()->mainPlotWindow())
    {
        this->setMdiWindowGeometry(RiaApplication::instance()->mainPlotWindow()->windowGeometryForViewer(m_qwtPlot));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimSummaryPlot::snapshotWindowContent()
{
    QImage image;

    if (m_qwtPlot)
    {
        image = QImage(m_qwtPlot->size(), QImage::Format_ARGB32);
        image.fill(QColor(Qt::white).rgb());

        QPainter painter(&image);
        QRectF rect(0, 0, m_qwtPlot->size().width(), m_qwtPlot->size().height());

        QwtPlotRenderer plotRenderer;
        plotRenderer.render(m_qwtPlot, &painter, rect);
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    caf::PdmUiTreeOrdering* axisFolder = uiTreeOrdering.add("Axes", ":/Axes16x16.png");
    axisFolder->add(&m_timeAxisProperties);
    axisFolder->add(&m_leftYAxisProperties);
    axisFolder->add(&m_rightYAxisProperties);

    uiTreeOrdering.add(&m_curveFilters);
    uiTreeOrdering.add(&m_curves);

    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::loadDataAndUpdate()
{
   updateViewerWidget();    

   for (RimSummaryCurveFilter* curveFilter: m_curveFilters)
   {
        curveFilter->loadDataAndUpdate();
   }

    for (RimSummaryCurve* curve : m_curves)
    {
        curve->loadDataAndUpdate();
    }
 
    this->updateAxes();

    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomInQwt()
{
    if (!m_qwtPlot) return;
    
    if (m_isAutoZoom)
    {
        zoomAll();
    }
    else
    {
        setZoomIntervalsInQwtPlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setZoomIntervalsInQwtPlot()
{
    QwtInterval left, right, time;

    left.setMinValue(m_leftYAxisProperties->visibleRangeMin());
    left.setMaxValue(m_leftYAxisProperties->visibleRangeMax());
    right.setMinValue(m_rightYAxisProperties->visibleRangeMin());
    right.setMaxValue(m_rightYAxisProperties->visibleRangeMax());
    time.setMinValue(m_timeAxisProperties->visibleRangeMin());
    time.setMaxValue(m_timeAxisProperties->visibleRangeMax());

    m_qwtPlot->setZoomWindow(left, right, time);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomFromQwt()
{
    if (!m_qwtPlot) return;

    QwtInterval left, right, time;
    m_qwtPlot->currentVisibleWindow(&left, &right, &time);

    setZoomWindow(left, right, time);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::disableAutoZoom()
{
    m_isAutoZoom = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateViewerWidget()
{
    RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
    if (!mainPlotWindow) return;

    if (m_showWindow())
    {
        if (!m_qwtPlot)
        {
            m_qwtPlot = new RiuSummaryQwtPlot(this, mainPlotWindow);

            for(RimSummaryCurveFilter* curveFilter: m_curveFilters)
            {
                curveFilter->setParentQwtPlot(m_qwtPlot);
            }

            for(RimSummaryCurve* curve : m_curves)
            {
                curve->setParentQwtPlot(m_qwtPlot);
            }

            mainPlotWindow->addViewer(m_qwtPlot, this->mdiWindowGeometry());
            mainPlotWindow->setActiveViewer(m_qwtPlot);
        }

        updateViewerWidgetWindowTitle();
    }
    else
    {
        if (m_qwtPlot)
        {
            this->setMdiWindowGeometry(mainPlotWindow->windowGeometryForViewer(m_qwtPlot));

            mainPlotWindow->removeViewer(m_qwtPlot);
            detachAllCurves();

            deletePlotWidget();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateViewerWidgetWindowTitle()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->setWindowTitle(m_userName);

        if (m_showPlotTitle)
        {
            m_qwtPlot->setTitle(m_userName);
        }
        else
        {
            m_qwtPlot->setTitle("");
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::detachAllCurves()
{
    for(RimSummaryCurveFilter* curveFilter: m_curveFilters)
    {
        curveFilter->detachQwtCurves();
    }

    for(RimSummaryCurve* curve : m_curves)
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryPlot::findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const
{
    for(RimSummaryCurve* rimCurve: m_curves)
    {
        if(rimCurve->qwtPlotCurve() == qwtCurve)
        {
            return rimCurve;
        }
    }

    for (RimSummaryCurveFilter* curveFilter: m_curveFilters)
    {
        RimSummaryCurve* foundCurve = curveFilter->findRimCurveFromQwtCurve(qwtCurve);
        if (foundCurve) return foundCurve;
    }

    return NULL;
}
