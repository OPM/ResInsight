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

#include "RiuTimeHistoryQwtPlot.h"

#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_engine.h"
#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTimeHistoryQwtPlot::RiuTimeHistoryQwtPlot(QWidget* parent)
    : QwtPlot(parent)
{
    setDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTimeHistoryQwtPlot::~RiuTimeHistoryQwtPlot()
{
    deleteAllCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTimeHistoryQwtPlot::addCurve(const QString& curveName, const std::vector<QDateTime>& dateTimes, const std::vector<double>& yValues)
{
    CVF_ASSERT(dateTimes.size() == yValues.size());

    QwtPlotCurve* plotCurve = new QwtPlotCurve("Curve 1");

    QPolygonF points;
    for (int i = 0; i < dateTimes.size(); i++)
    {
        double milliSecSinceEpoch = QwtDate::toDouble(dateTimes[i]);
        points << QPointF(milliSecSinceEpoch, yValues[i]);
    }

    plotCurve->setSamples(points);
    plotCurve->setTitle(curveName);

    plotCurve->attach(this);
    m_plotCurves.push_back(plotCurve);

    this->setAxisScale( QwtPlot::xTop, QwtDate::toDouble(dateTimes.front()), QwtDate::toDouble(dateTimes.back()));

    this->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTimeHistoryQwtPlot::deleteAllCurves()
{
    for (size_t i = 0; i < m_plotCurves.size(); i++)
    {
        m_plotCurves[i]->detach();
        delete m_plotCurves[i];
    }

    m_plotCurves.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTimeHistoryQwtPlot::setDefaults()
{
    QPalette newPalette(palette());
    newPalette.setColor(QPalette::Background, Qt::white);
    setPalette(newPalette);

    setAutoFillBackground(true);
    setCanvasBackground(Qt::white);

    QFrame* canvasFrame = dynamic_cast<QFrame*>(canvas());
    if (canvasFrame)
    {
        canvasFrame->setFrameShape(QFrame::NoFrame);
    }

    canvas()->setMouseTracking(true);
    canvas()->installEventFilter(this);

    enableAxis(QwtPlot::xBottom, true);
    enableAxis(QwtPlot::yLeft, true);
    enableAxis(QwtPlot::xTop, false);
    enableAxis(QwtPlot::yRight, false);

    plotLayout()->setAlignCanvasToScales(true);

    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw(Qt::UTC);
    scaleDraw->setDateFormat(QwtDate::Year, QString("dd-MM-yyyy"));
 
    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine(Qt::UTC);
    setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
    setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);

    QFont xAxisFont = axisFont(QwtPlot::xBottom);
    xAxisFont.setPixelSize(9);
    setAxisFont(QwtPlot::xBottom, xAxisFont);

    QFont yAxisFont = axisFont(QwtPlot::yLeft);
    yAxisFont.setPixelSize(9);
    setAxisFont(QwtPlot::yLeft, yAxisFont);

    QwtText axisTitleY = axisTitle(QwtPlot::yLeft);
    QFont yAxisTitleFont = axisTitleY.font();
    yAxisTitleFont.setPixelSize(9);
    yAxisTitleFont.setBold(false);
    axisTitleY.setFont(yAxisTitleFont);
    axisTitleY.setRenderFlags(Qt::AlignRight);
    setAxisTitle(QwtPlot::yLeft, axisTitleY);

    
    QwtLegend* legend = new QwtLegend(this);
    // The legend will be deleted in the destructor of the plot or when 
    // another legend is inserted.
    this->insertLegend(legend, BottomLegend);
}
