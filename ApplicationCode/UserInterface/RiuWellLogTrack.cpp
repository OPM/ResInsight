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

#include "RiaApplication.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellLogCurve.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtCurvePointTracker.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text.h"

#include <QFont>
#include <QMouseEvent>
#include <QScrollArea>
#include <QWheelEvent>

#include <float.h>
#include "RiuSummaryQwtPlot.h"

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
   
    setDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::~RiuWellLogTrack()
{
  
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setDefaults()
{
    RiuSummaryQwtPlot::setCommonPlotBehaviour(this);

    enableAxis(QwtPlot::xTop, true);
    enableAxis(QwtPlot::yLeft, true);
    enableAxis(QwtPlot::xBottom, false);
    enableAxis(QwtPlot::yRight, false);

    axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, true);

    // Align the canvas with the actual min and max values of the curves
    axisScaleEngine(QwtPlot::xTop)->setAttribute(QwtScaleEngine::Floating, true);
    axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);
    setAxisScale(QwtPlot::yLeft, 1000, 0);
    setAxisScale(QwtPlot::xTop, -10, 100);

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
void RiuWellLogTrack::setXTitle(const QString& title)
{
    QwtText axisTitleX = axisTitle(QwtPlot::xTop);
    axisTitleX.setText(title);
    setAxisTitle(QwtPlot::xTop, axisTitleX);
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
            m_plotTrackDefinition->firstAncestorOrThisOfType(plotDefinition);
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
void RiuWellLogTrack::selectClosestCurve(const QPoint& pos)
{
    QwtPlotCurve* closestCurve = nullptr;
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
            RiuPlotMainWindowTools::showPlotMainWindow();
            RiuPlotMainWindowTools::selectAsCurrentItem(selectedCurve);

            return;
        }
    }

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem(m_plotTrackDefinition);
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

