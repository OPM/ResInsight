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

#include "RiuWellLogTrack.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellLogCurve.h"

#include "RiuMainPlotWindow.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_text.h"

#include <QFont>
#include <QMouseEvent>
#include <QScrollArea>
#include <QWheelEvent>

#include <float.h>
#include "RiaApplication.h"

#define RIU_SCROLLWHEEL_ZOOMFACTOR  1.1
#define RIU_SCROLLWHEEL_PANFACTOR   0.1

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::RiuWellLogTrack(RimWellLogTrack* plotTrackDefinition, QWidget* parent)
    : QwtPlot(parent)
{
    Q_ASSERT(plotTrackDefinition);
    m_plotTrackDefinition = plotTrackDefinition;

    m_grid = new QwtPlotGrid();
    m_grid->attach(this);
   
    setFocusPolicy(Qt::ClickFocus);
    setDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::~RiuWellLogTrack()
{
    m_grid->detach();
    delete m_grid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setDefaults()
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setDepthZoom(double minDepth, double maxDepth)
{
    // Note: Y-axis is inverted
    setAxisScale(QwtPlot::yLeft, maxDepth, minDepth);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setXRange(double min, double max)
{
    setAxisScale(QwtPlot::xTop, min, max);
    setAxisScale(QwtPlot::xBottom, min, max);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setDepthTitle(const QString& title)
{
    QwtText axisTitleY = axisTitle(QwtPlot::yLeft);
    axisTitleY.setText(title);
    setAxisTitle(QwtPlot::yLeft, axisTitleY);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuWellLogTrack::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == canvas())
    {
        QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(event);
        if (wheelEvent)
        {
            if (!m_plotTrackDefinition)
            {
                return QwtPlot::eventFilter(watched, event);
            }

            RimWellLogPlot* plotDefinition;
            m_plotTrackDefinition->firstAnchestorOrThisOfType(plotDefinition);
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
                    plotDefinition->setDepthZoomByFactorAndCenter(RIU_SCROLLWHEEL_ZOOMFACTOR, zoomCenter);
                }
                else
                {
                    plotDefinition->setDepthZoomByFactorAndCenter(1.0/RIU_SCROLLWHEEL_ZOOMFACTOR, zoomCenter);
                }
            }
            else
            {
                plotDefinition->panDepth(wheelEvent->delta() < 0 ? RIU_SCROLLWHEEL_PANFACTOR : -RIU_SCROLLWHEEL_PANFACTOR);
            }

            event->accept();
            return true;
        }
        else
        {
            QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (mouseEvent)
            {
                if (mouseEvent->button() == Qt::LeftButton && mouseEvent->type() == QMouseEvent::MouseButtonRelease)
                {
                    selectClosestCurve(mouseEvent->pos());
                }
            }
        }
    }

    return QwtPlot::eventFilter(watched, event);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::focusInEvent(QFocusEvent* event)
{
    if (m_plotTrackDefinition)
    {
        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(m_plotTrackDefinition);
        clearFocus();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::selectClosestCurve(const QPoint& pos)
{
    QwtPlotCurve* closestCurve = NULL;
    double distMin = DBL_MAX;

    const QwtPlotItemList& itmList = itemList();
    for (QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); it++)
    {
        if ((*it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve* candidateCurve = static_cast<QwtPlotCurve*>(*it);
            double dist = DBL_MAX;
            candidateCurve->closestPoint(pos, &dist);
            if (dist < distMin)
            {
                closestCurve = candidateCurve;
                distMin = dist;
            }
        }
    }

    if (closestCurve && distMin < 20)
    {
        RimWellLogCurve* selectedCurve = m_plotTrackDefinition->curveDefinitionFromCurve(closestCurve);
        if (selectedCurve)
        {
            RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(selectedCurve);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellLogTrack::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellLogTrack::minimumSizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuWellLogTrack::isRimTrackVisible()
{
    if (m_plotTrackDefinition)
    {
        return m_plotTrackDefinition->isVisible();
    }
   
   return false;
}

