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
#include "RimSummaryPlotCollection.h"
#include "RimSummaryYAxisProperties.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionColors.h"
#include "RiuSummaryQwtPlot.h"

#include "cvfBase.h"
#include "cvfColor3.h"

#include <QDateTime>
#include <QRectF>

#include "qwt_plot_renderer.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"


//--------------------------------------------------------------------------------------------------
// e	format as [-]9.9e[+|-]999
// E	format as[-]9.9E[+| -]999
// f	format as[-]9.9
// g	use e or f format, whichever is the most concise
// G	use E or f format, whichever is the most concise

//--------------------------------------------------------------------------------------------------
class DecimalScaleDraw : public QwtScaleDraw
{
public:
    virtual QwtText label(double value) const override
    {
        if (qFuzzyCompare(value + 1.0, 1.0))
            value = 0.0;

        return QString::number(value, 'f', 3);
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class ScientificScaleDraw : public QwtScaleDraw
{

public:
    virtual QwtText label(double value) const override
    {
        if (qFuzzyCompare(value + 1.0, 1.0))
            value = 0.0;

        return QString::number(value, 'e', 3);
    }
};




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
    m_curveFilters.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curves, "SummaryCurves", "",  "", "", "");
    m_curves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_visibleWindow, "VisibleWindow", std::vector<float>(), "Visible Display Window", "", "", "");
    m_visibleWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_leftYAxisProperties, "LeftYAxisProperties", "Left Y Axis", "", "", "");
    m_leftYAxisProperties.uiCapability()->setUiHidden(true);

    m_leftYAxisPropertiesObject = std::unique_ptr<RimSummaryYAxisProperties>(new RimSummaryYAxisProperties);
    m_leftYAxisPropertiesObject->setName("Left Y-Axis");
    m_leftYAxisProperties = m_leftYAxisPropertiesObject.get();

    CAF_PDM_InitFieldNoDefault(&m_rightYAxisProperties, "RightYAxisProperties", "Right Y Axis", "", "", "");
    m_rightYAxisProperties.uiCapability()->setUiHidden(true);

    m_rightYAxisPropertiesObject = std::unique_ptr<RimSummaryYAxisProperties>(new RimSummaryYAxisProperties);
    m_rightYAxisPropertiesObject->setName("Right Y-Axis");
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
    updateLeftYAxis();
    updateRightYAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateLeftYAxis()
{
    if (!m_qwtPlot) return;

    {
        QString axisTitle = m_leftYAxisPropertiesObject->customTitle;
        if (m_leftYAxisPropertiesObject->isAutoTitle) axisTitle = autoAxisTitle();
    
        QwtText axisTitleY = m_qwtPlot->axisTitle(QwtPlot::yLeft);

        QFont axisTitleYFont = axisTitleY.font();
        axisTitleYFont.setBold(true);
        axisTitleYFont.setPixelSize(m_leftYAxisPropertiesObject->fontSize);
        axisTitleY.setFont(axisTitleYFont);

        axisTitleY.setText(axisTitle);
        m_qwtPlot->setAxisTitle(QwtPlot::yLeft, axisTitleY);
    }

    {
        QFont yAxisFont = m_qwtPlot->axisFont(QwtPlot::yLeft);
        yAxisFont.setBold(false);
        yAxisFont.setPixelSize(m_leftYAxisPropertiesObject->fontSize);
        m_qwtPlot->setAxisFont(QwtPlot::yLeft, yAxisFont);
    }

    {
        if (m_leftYAxisPropertiesObject->numberFormat == RimSummaryYAxisProperties::NUMBER_FORMAT_AUTO)
        {
            m_qwtPlot->setAxisScaleDraw(QwtPlot::yLeft, new QwtScaleDraw);
        }
        else if (m_leftYAxisPropertiesObject->numberFormat == RimSummaryYAxisProperties::NUMBER_FORMAT_DECIMAL)
        {
            m_qwtPlot->setAxisScaleDraw(QwtPlot::yLeft, new DecimalScaleDraw);
        }
        else if (m_leftYAxisPropertiesObject->numberFormat == RimSummaryYAxisProperties::NUMBER_FORMAT_SCIENTIFIC)
        {
            m_qwtPlot->setAxisScaleDraw(QwtPlot::yLeft, new ScientificScaleDraw());
        }
    }

    {
        if (m_leftYAxisPropertiesObject->isLogarithmicScaleEnabled)
        {
            QwtLogScaleEngine* currentScaleEngine = dynamic_cast<QwtLogScaleEngine*>(m_qwtPlot->axisScaleEngine(QwtPlot::yLeft));
            if (!currentScaleEngine)
            {
                m_qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);

                m_qwtPlot->replot();
            }
        }
        else
        {
            QwtLinearScaleEngine* currentScaleEngine = dynamic_cast<QwtLinearScaleEngine*>(m_qwtPlot->axisScaleEngine(QwtPlot::yLeft));
            if (!currentScaleEngine)
            {
                m_qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);

                m_qwtPlot->replot();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateRightYAxis()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::autoAxisTitle()
{
    std::set<std::string> unitNames;

    for (RimSummaryCurve* rimCurve : m_curves)
    {
        if (rimCurve->isCurveVisible()) unitNames.insert(rimCurve->unitName());
    }

    for (RimSummaryCurveFilter* curveFilter : m_curveFilters)
    {
        std::set<std::string> filterUnitNames = curveFilter->unitNames();
        unitNames.insert(filterUnitNames.begin(), filterUnitNames.end());
    }

    QString assembledYAxisText;

    for (const std::string& unitName : unitNames)
    {
        assembledYAxisText += "[" + QString::fromStdString(unitName) + "] ";
    }

    return assembledYAxisText;
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
void RimSummaryPlot::updateYAxisUnit()
{
    updateLeftAndRightYAxis();

/*
    if (!m_qwtPlot) return;

    std::set<std::string> unitNames;

    for(RimSummaryCurve* rimCurve: m_curves)
    {
        if (rimCurve->isCurveVisible()) unitNames.insert(rimCurve->unitName());
    }

    for(RimSummaryCurveFilter* curveFilter: m_curveFilters)
    {
        std::set<std::string> filterUnitNames = curveFilter->unitNames();
        unitNames.insert(filterUnitNames.begin(), filterUnitNames.end());
    }

    QString assembledYAxisText;

    for (const std::string& unitName : unitNames)
    {
        assembledYAxisText += "[" + QString::fromStdString(unitName) + "] ";
    }

    m_qwtPlot->setYAxisTitle(assembledYAxisText);
*/
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
            this->updateYAxisUnit();
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
            this->updateYAxisUnit();
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
 
    this->updateYAxisUnit();

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
