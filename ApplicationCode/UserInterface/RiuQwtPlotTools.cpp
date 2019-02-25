/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiuQwtPlotTools.h"

#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::setCommonPlotBehaviour(QwtPlot* plot)
{
    // Plot background and frame look

    QPalette newPalette(plot->palette());
    newPalette.setColor(QPalette::Background, Qt::white);
    plot->setPalette(newPalette);

    plot->setAutoFillBackground(true);
    plot->setCanvasBackground(Qt::white);

    QFrame* canvasFrame = dynamic_cast<QFrame*>(plot->canvas());
    if (canvasFrame)
    {
        canvasFrame->setFrameShape(QFrame::NoFrame);
    }

    // Grid

    QwtPlotGrid* grid = new QwtPlotGrid;
    grid->attach(plot);
    QPen gridPen(Qt::SolidLine);
    gridPen.setColor(Qt::lightGray);
    grid->setPen(gridPen);

    // Axis number font
    QFont axisFont = plot->axisFont(QwtPlot::xBottom);
    axisFont.setPixelSize(11);

    plot->setAxisFont(QwtPlot::xBottom, axisFont);
    plot->setAxisFont(QwtPlot::xTop, axisFont);
    plot->setAxisFont(QwtPlot::yLeft, axisFont);
    plot->setAxisFont(QwtPlot::yRight, axisFont);

    // Axis title font
    std::vector<QwtPlot::Axis> axes = { QwtPlot::xBottom, QwtPlot::xTop, QwtPlot::yLeft, QwtPlot::yRight };
    
    for (QwtPlot::Axis axis : axes)
    {
        QwtText axisTitle     = plot->axisTitle(axis);
        QFont   axisTitleFont = axisTitle.font();
        axisTitleFont.setPixelSize(11);
        axisTitleFont.setBold(false);
        axisTitle.setFont(axisTitleFont);
        axisTitle.setRenderFlags(Qt::AlignRight);

        plot->setAxisTitle(axis, axisTitle);
    }

    // Set a focus policy to allow it taking key press events.
    // This is not strictly necessary since this widget inherit QwtPlot
    // which already has a focus policy.
    // However, for completeness we still do it here.
    plot->setFocusPolicy(Qt::WheelFocus);

    // Enable mousetracking and event filter
    plot->canvas()->setMouseTracking(true);
    plot->canvas()->installEventFilter(plot);
    plot->plotLayout()->setAlignCanvasToScales(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::setDefaultAxes(QwtPlot* plot)
{
    plot->enableAxis(QwtPlot::xBottom, true);
    plot->enableAxis(QwtPlot::yLeft, true);
    plot->enableAxis(QwtPlot::xTop, false);
    plot->enableAxis(QwtPlot::yRight, false);

    plot->setAxisMaxMinor(QwtPlot::xBottom, 2);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 3);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::enableDateBasedBottomXAxis(QwtPlot* plot)
{
    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw(Qt::UTC);
    scaleDraw->setDateFormat(QwtDate::Year, QString("dd-MM-yyyy"));

    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine(Qt::UTC);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
    plot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
}