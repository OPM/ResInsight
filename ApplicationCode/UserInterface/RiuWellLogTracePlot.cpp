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

#include "RiuWellLogTracePlot.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrace.h"

#include "qwt_plot_grid.h"
#include "qwt_legend.h"
#include "qwt_scale_engine.h"
#include "qwt_plot_layout.h"

#include <QWheelEvent>
#include <QFont>

#define RIU_SCROLLWHEEL_ZOOMFACTOR  1.1
#define RIU_SCROLLWHEEL_PANFACTOR   0.1

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTracePlot::RiuWellLogTracePlot(RimWellLogPlotTrace* plotTraceDefinition, QWidget* parent)
    : QwtPlot(parent)
{
    Q_ASSERT(plotTraceDefinition);
    m_plotTraceDefinition = plotTraceDefinition;

    m_grid = new QwtPlotGrid();
    m_grid->attach(this);

    m_legend = new QwtLegend(this);
    insertLegend(m_legend, QwtPlot::TopLegend);
    
    setDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTracePlot::~RiuWellLogTracePlot()
{
    m_grid->detach();
    delete m_grid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTracePlot::setDefaults()
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

    setAxisAutoScale(QwtPlot::xTop, true);

    QFont xAxisFont = axisFont(QwtPlot::xTop);
    xAxisFont.setPixelSize(9);
    setAxisFont(QwtPlot::xTop, xAxisFont);

    QFont yAxisFont = axisFont(QwtPlot::yLeft);
    yAxisFont.setPixelSize(9);
    setAxisFont(QwtPlot::yLeft, yAxisFont);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTracePlot::setDepthRange(double minDepth, double maxDepth)
{
    // Note: Y-axis is inverted
    setAxisScale(QwtPlot::yLeft, maxDepth, minDepth);
    //replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuWellLogTracePlot::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == canvas())
    {
        QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(event);
        if (wheelEvent)
        {
            if (!m_plotTraceDefinition)
            {
                return QwtPlot::eventFilter(watched, event);
            }

            RimWellLogPlot* plotDefinition;
            m_plotTraceDefinition->firstAnchestorOrThisOfType(plotDefinition);
            if (!plotDefinition)
            {
                return QwtPlot::eventFilter(watched, event);
            }

            if (wheelEvent->modifiers() & Qt::ControlModifier)
            {
                QwtScaleMap scaleMap = canvasMap(QwtPlot::yLeft);
                double zoomCenter = scaleMap.invTransform(wheelEvent->pos().y());

                if (wheelEvent->delta() > 0)
                {
                    plotDefinition->zoomDepth(RIU_SCROLLWHEEL_ZOOMFACTOR, zoomCenter);
                }
                else
                {
                    plotDefinition->zoomDepth(1.0/RIU_SCROLLWHEEL_ZOOMFACTOR, zoomCenter);
                }
            }
            else
            {
                plotDefinition->panDepth(wheelEvent->delta() < 0 ? RIU_SCROLLWHEEL_PANFACTOR : -RIU_SCROLLWHEEL_PANFACTOR);
            }

            event->accept();
            return true;
        }
    }

    return QwtPlot::eventFilter(watched, event);
}
