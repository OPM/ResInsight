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

#include "RiuResultQwtPlot.h"

#include "RigCurveDataTools.h"

#include "RiuLineSegmentQwtPlotCurve.h"

#include "cvfBase.h"
#include "cvfAssert.h"
#include "cvfColor3.h"

#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_engine.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuResultQwtPlot::RiuResultQwtPlot(QWidget* parent)
    : QwtPlot(parent)
{
    m_grid = new QwtPlotGrid;
    m_grid->attach(this);

    setDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuResultQwtPlot::~RiuResultQwtPlot()
{
    deleteAllCurves();

    m_grid->detach();
    delete m_grid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::addCurve(const QString& curveName, const cvf::Color3f& curveColor, const std::vector<QDateTime>& dateTimes, const std::vector<double>& timeHistoryValues)
{
    RiuLineSegmentQwtPlotCurve* plotCurve = new RiuLineSegmentQwtPlotCurve("Curve 1");

    plotCurve->setSamplesFromDateAndValues(dateTimes, timeHistoryValues);
    plotCurve->setTitle(curveName);

    plotCurve->setPen(QPen(QColor(curveColor.rByte(), curveColor.gByte(), curveColor.bByte())));

    plotCurve->attach(this);
    m_plotCurves.push_back(plotCurve);

    this->setAxisScale( QwtPlot::xTop, QwtDate::toDouble(dateTimes.front()), QwtDate::toDouble(dateTimes.back()));

    this->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::addCurve(const QString& curveName, const cvf::Color3f& curveColor, const std::vector<double>& frameTimes, const std::vector<double>& timeHistoryValues)
{
    std::vector<QDateTime> dateTimes;

    for (size_t i = 0; i < frameTimes.size(); i++)
    {
        dateTimes.push_back(QwtDate::toDateTime(frameTimes[i]));
    }

    addCurve(curveName, curveColor, dateTimes, timeHistoryValues);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::deleteAllCurves()
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
QSize RiuResultQwtPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuResultQwtPlot::minimumSizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultQwtPlot::setDefaults()
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

    QPen gridPen(Qt::SolidLine);
    gridPen.setColor(Qt::lightGray);
    m_grid->setPen(gridPen);

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
