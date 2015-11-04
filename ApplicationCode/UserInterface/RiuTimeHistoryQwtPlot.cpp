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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTimeHistoryQwtPlot::RiuTimeHistoryQwtPlot(QWidget* parent)
    : QwtPlot(parent)
{
/*
    setFocusPolicy(Qt::ClickFocus);
*/
    setDefaults();

    std::vector<double> xValues;
    std::vector<double> yValues;

    xValues.push_back(1);
    xValues.push_back(2);
    xValues.push_back(3);
    xValues.push_back(4);

    yValues.push_back(10);
    yValues.push_back(12);
    yValues.push_back(15);
    yValues.push_back(11);

    addCurve("Test", xValues, yValues);
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
void RiuTimeHistoryQwtPlot::addCurve(const QString& curveName, const std::vector<double>& xValues, const std::vector<double>& yValues)
{
    CVF_ASSERT(xValues.size() == yValues.size());

    QwtPlotCurve* plotCurve = new QwtPlotCurve("Curve 1");
    plotCurve->setSamples(xValues.data(), yValues.data(), (int) xValues.size());
    plotCurve->setTitle(curveName);

    plotCurve->attach(this);
    m_plotCurves.push_back(plotCurve);
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

/*
    QPen gridPen(Qt::SolidLine);
    gridPen.setColor(Qt::lightGray);
    m_grid->setPen(gridPen);
*/

    enableAxis(QwtPlot::xTop, true);
    enableAxis(QwtPlot::yLeft, true);
    enableAxis(QwtPlot::xBottom, false);
    enableAxis(QwtPlot::yRight, false);

    plotLayout()->setAlignCanvasToScales(true);

    axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, true);

    // Align the canvas with the actual min and max values of the curves
    axisScaleEngine(QwtPlot::xTop)->setAttribute(QwtScaleEngine::Floating, true);
    axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);
    setAxisScale(QwtPlot::yLeft, 1000, 0);
    setAxisScale(QwtPlot::xTop, -10, 100);

    QFont xAxisFont = axisFont(QwtPlot::xTop);
    xAxisFont.setPixelSize(9);
    setAxisFont(QwtPlot::xTop, xAxisFont);

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
