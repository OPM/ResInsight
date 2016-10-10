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
#include "RimSummaryYAxisProperties.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionColors.h"
#include "RiuSummaryQwtPlot.h"

#include "cvfBase.h"
#include "cvfColor3.h"

#include <QDateTime>
#include <QRectF>

#include "qwt_plot_curve.h"
#include "qwt_plot_renderer.h"
#include "cafPdmUiTreeOrdering.h"


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

    CAF_PDM_InitFieldNoDefault(&m_curveFilters, "SummaryCurveFilters", "", "", "", "");
    m_curveFilters.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curves, "SummaryCurves", "",  "", "", "");
    m_curves.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitField(&m_visibleWindow, "VisibleWindow", std::vector<float>(), "Visible Display Window", "", "", "");
    m_visibleWindow.uiCapability()->setUiHidden(true);

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
void RimSummaryPlot::updateLeftAndRightYAxis()
{
    updateAxis(RimDefines::PLOT_AXIS_LEFT);
    updateAxis(RimDefines::PLOT_AXIS_RIGHT);

    if (m_qwtPlot) m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxis(RimDefines::PlotAxis plotAxis)
{
    if (!m_qwtPlot) return;

    std::vector<RimSummaryCurve*> curvesForAxis;
    std::vector<RimSummaryCurveFilter*> curveFiltersForAxis;

    std::vector<RimSummaryCurve*> childCurves;
    this->descendantsIncludingThisOfType(childCurves);

    for (RimSummaryCurve* curve : childCurves)
    {
        if (curve->associatedPlotAxis() == plotAxis)
        {
            curvesForAxis.push_back(curve);
        }
    }

    for (RimSummaryCurveFilter* cs : m_curveFilters)
    {
        if (cs->associatedPlotAxis() == plotAxis)
        {
            curveFiltersForAxis.push_back(cs);
        }
    }

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

    if (curvesForAxis.size() > 0)
    {
        m_qwtPlot->enableAxis(qwtAxis, true);

        RimSummaryCurvesCalculator calc(yAxisProperties, curvesForAxis, curveFiltersForAxis);
        calc.applyPropertiesToPlot(m_qwtPlot);
    }
    else
    {
        m_qwtPlot->enableAxis(qwtAxis, false);
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
QWidget* RimSummaryPlot::viewer()
{
    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setZoomWindow(const QRectF& zoomWindow)
{
    if(!zoomWindow.isEmpty())
    {
        std::vector<float> window;
        window.push_back(zoomWindow.left());
        window.push_back(zoomWindow.top());
        window.push_back(zoomWindow.width());
        window.push_back(zoomWindow.height());

        m_visibleWindow = window;

        m_leftYAxisProperties->visibleRangeMax = zoomWindow.bottom();
        m_leftYAxisProperties->visibleRangeMin = zoomWindow.top();
        m_leftYAxisProperties->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::zoomAll()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->zoomAll();
        this->setZoomWindow(m_qwtPlot->currentVisibleWindow());
    }
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
            this->updateLeftAndRightYAxis();
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
            this->updateLeftAndRightYAxis();
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
    else if (changedField == &m_userName)
    {
        updateViewerWidgetWindowTitle();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setupBeforeSave()
{
    if (m_qwtPlot)
    {
        if (RiaApplication::instance()->mainPlotWindow())
        {
            this->setMdiWindowGeometry(RiaApplication::instance()->mainPlotWindow()->windowGeometryForViewer(m_qwtPlot));
        }

        this->setZoomWindow(m_qwtPlot->currentVisibleWindow());
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
    uiTreeOrdering.add(&m_leftYAxisProperties);
    uiTreeOrdering.add(&m_rightYAxisProperties);
    uiTreeOrdering.add(&m_curveFilters);
    uiTreeOrdering.add(&m_curves);
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
 
    this->updateLeftAndRightYAxis();

    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomInQwt()
{
    if (!m_qwtPlot) return;

    // Todo: introduce autoscale

    if(m_visibleWindow().size() == 4)
    {
        QRectF visibleWindow(m_visibleWindow()[0], m_visibleWindow()[1], m_visibleWindow()[2], m_visibleWindow()[3]);

        m_qwtPlot->setZoomWindow(visibleWindow);
    }
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
