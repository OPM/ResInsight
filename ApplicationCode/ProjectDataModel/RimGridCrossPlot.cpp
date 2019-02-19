/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RimGridCrossPlot.h"

#include "RimGridCrossPlotCurve.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <QDebug>

CAF_PDM_SOURCE_INIT(RimGridCrossPlot, "RimGridCrossPlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::RimGridCrossPlot()
{
    CAF_PDM_InitObject("Grid Cross Plot", ":/SummaryXPlotsLight16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_crossPlotCurve, "CrossPlotCurve", "Cross Plot Data Set", "", "", "");
    m_crossPlotCurve.uiCapability()->setUiHidden(true);
    m_crossPlotCurve = new RimGridCrossPlotCurve();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridCrossPlot::viewWidget()
{
    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimGridCrossPlot::snapshotWindowContent()
{
    QImage image;

    if (m_qwtPlot)
    {
        QPixmap pix = QPixmap::grabWidget(m_qwtPlot);
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::zoomAll()
{
    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridCrossPlot::createViewWidget(QWidget* mainWindowParent)
{
    if (!m_qwtPlot)
    {
        m_qwtPlot = new QwtPlot(QString("Grid Cross Plot"), mainWindowParent);
    }

    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::deleteViewWidget()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->deleteLater();
        m_qwtPlot = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::onLoadDataAndUpdate()
{
    qDebug() << "Loading Data for X-Plot";

    updateMdiWindowVisibility();
    CVF_ASSERT(m_qwtPlot);
    
    m_qwtPlot->setTitle("Cross Plot Test");
    m_qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
    m_qwtPlot->setAxisAutoScale(QwtPlot::xBottom);

    m_crossPlotCurve->setLineStyle(RiuQwtPlotCurve::STYLE_NONE);
    m_crossPlotCurve->setSymbol(RiuQwtSymbol::SYMBOL_CROSS);
    m_crossPlotCurve->setSymbolSize(6);
    m_crossPlotCurve->setColor(cvf::Color3::RED);
    m_crossPlotCurve->loadDataAndUpdate(false);
    m_crossPlotCurve->setParentQwtPlotAndReplot(m_qwtPlot);
    m_crossPlotCurve->updateCurveAppearance();
    m_qwtPlot->show();
}
