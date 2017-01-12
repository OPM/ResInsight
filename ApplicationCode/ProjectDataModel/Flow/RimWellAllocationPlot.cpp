/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimWellAllocationPlot.h"

#include "RiaApplication.h"
#include "RiuMainPlotWindow.h"

#include "RiuWellAllocationPlot.h"
#include "qwt_plot_renderer.h"



CAF_PDM_SOURCE_INIT(RimWellAllocationPlot, "WellAllocationPlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::RimWellAllocationPlot()
{
    CAF_PDM_InitObject("Well Allocation Plot", ":/SummaryPlot16x16.png", "", "");
    CAF_PDM_InitField(&m_showWindow, "ShowWindow", true, "Show Summary Plot", "", "", "");
    m_showWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Summary Plot"), "Name", "", "", "");
    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::~RimWellAllocationPlot()
{
    if (RiaApplication::instance()->mainPlotWindow())
    {
        RiaApplication::instance()->mainPlotWindow()->removeViewer(m_qwtPlot);
    }

    deletePlotWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::deletePlotWidget()
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
void RimWellAllocationPlot::updateAxes()
{
    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationPlot::viewWidget()
{
    return m_qwtPlot;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::handleViewerDeletion()
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
void RimWellAllocationPlot::zoomAll()
{


}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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
void RimWellAllocationPlot::setupBeforeSave()
{
    if (m_qwtPlot && RiaApplication::instance()->mainPlotWindow())
    {
        this->setMdiWindowGeometry(RiaApplication::instance()->mainPlotWindow()->windowGeometryForViewer(m_qwtPlot));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimWellAllocationPlot::snapshotWindowContent()
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
void RimWellAllocationPlot::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::loadDataAndUpdate()
{
    updateViewerWidget();

//     for (RimSummaryCurveFilter* curveFilter : m_curveFilters)
//     {
//         curveFilter->loadDataAndUpdate();
//     }
// 
//     for (RimSummaryCurve* curve : m_curves)
//     {
//         curve->loadDataAndUpdate();
//     }

    this->updateAxes();

    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateZoomInQwt()
{
    if (!m_qwtPlot) return;

    zoomAll();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateViewerWidget()
{
    RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
    if (!mainPlotWindow) return;

    if (m_showWindow())
    {
        if (!m_qwtPlot)
        {
            m_qwtPlot = new RiuWellAllocationPlot(this, mainPlotWindow);

//             for (RimSummaryCurveFilter* curveFilter : m_curveFilters)
//             {
//                 curveFilter->setParentQwtPlot(m_qwtPlot);
//             }
// 
//             for (RimSummaryCurve* curve : m_curves)
//             {
//                 curve->setParentQwtPlot(m_qwtPlot);
//             }

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
void RimWellAllocationPlot::updateViewerWidgetWindowTitle()
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
void RimWellAllocationPlot::detachAllCurves()
{
//     for (RimSummaryCurveFilter* curveFilter : m_curveFilters)
//     {
//         curveFilter->detachQwtCurves();
//     }
// 
//     for (RimSummaryCurve* curve : m_curves)
//     {
//         curve->detachQwtCurve();
//     }
}

