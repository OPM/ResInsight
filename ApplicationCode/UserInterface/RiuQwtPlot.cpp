/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RiuQwtPlot.h"

#include "RiaColorTools.h"

#include "RimProject.h"

#include "RiuPlotMainWindowTools.h" 
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuQwtScalePicker.h"

#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_magnifier.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"
#include "qwt_symbol.h"

#include <QEvent>
#include <QMenu>
#include <QWheelEvent>

#include <cfloat>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQwtPlot::RiuQwtPlot(RimViewWindow* viewWindow, QWidget* parent) : QwtPlot(parent)
{
    Q_ASSERT(viewWindow);
    m_ownerViewWindow = viewWindow;

    // LeftButton for the zooming
    m_zoomerLeft = new RiuQwtPlotZoomer(canvas());
    m_zoomerLeft->setRubberBandPen(QColor(Qt::black));
    m_zoomerLeft->setTrackerMode(QwtPicker::AlwaysOff);
    m_zoomerLeft->setTrackerPen(QColor(Qt::black));
    m_zoomerLeft->initMousePattern(1);

    // Attach a zoomer for the right axis
    m_zoomerRight = new RiuQwtPlotZoomer(canvas());
    m_zoomerRight->setAxis(xTop, yRight);
    m_zoomerRight->setTrackerMode(QwtPicker::AlwaysOff);
    m_zoomerRight->initMousePattern(1);

    // MidButton for the panning
    QwtPlotPanner* panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::MidButton);

    auto wheelZoomer = new RiuQwtPlotWheelZoomer(this);

    connect(wheelZoomer, SIGNAL(zoomUpdated()), SLOT(onZoomedSlot()));
    connect(m_zoomerLeft, SIGNAL(zoomed( const QRectF & )), SLOT(onZoomedSlot()));
    connect(panner, SIGNAL(panned( int , int  )), SLOT(onZoomedSlot()));

    RiuQwtScalePicker* scalePicker = new RiuQwtScalePicker(this);
    connect(scalePicker, SIGNAL(clicked(int, double)), this, SLOT(onAxisClicked(int, double)));

    RiuQwtPlotTools::setCommonPlotBehaviour(this);
    RiuQwtPlotTools::setDefaultAxes(this);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQwtPlot::~RiuQwtPlot()
{
    if (ownerPlotDefinition())
    {
        ownerPlotDefinition()->detachAllCurves();
    }  
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRiuQwtPlotOwnerInterface* RiuQwtPlot::ownerPlotDefinition() const
{
    RimRiuQwtPlotOwnerInterface* plotDefinition = dynamic_cast<RimRiuQwtPlotOwnerInterface*>(ownerViewWindow());
    return plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuQwtPlot::ownerViewWindow() const
{
    return m_ownerViewWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlot::minimumSizeHint() const
{
    return QSize(0, 100);
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::selectSample(QwtPlotCurve* curve, int sampleNumber)
{ 
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::clearSampleSelection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::hideEvent(QHideEvent* event)
{
    resetCurveHighlighting();
    QwtPlot::hideEvent(event);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtInterval RiuQwtPlot::currentAxisRange(QwtPlot::Axis axis)
{
    return axisScaleDiv(axis).interval();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlot::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == canvas())
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if(mouseEvent)
        {
            if(mouseEvent->button() == Qt::LeftButton && mouseEvent->type() == QMouseEvent::MouseButtonRelease)
            {
                selectClosestCurve(mouseEvent->pos());
            }
        }
    }

    return QwtPlot::eventFilter(watched, event);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::selectClosestCurve(const QPoint& pos)
{
    QwtPlotCurve* closestCurve = nullptr;
    double distMin = DBL_MAX;
    int closestCurvePoint = -1;
    const QwtPlotItemList& itmList = itemList();
    for(QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); it++)
    {
        if((*it)->rtti() == QwtPlotItem::Rtti_PlotCurve)
        {
            QwtPlotCurve* candidateCurve = static_cast<QwtPlotCurve*>(*it);
            double dist = DBL_MAX;
            int curvePoint = candidateCurve->closestPoint(pos, &dist);
            if(dist < distMin)
            {
                closestCurve = candidateCurve;
                distMin = dist;
                closestCurvePoint = curvePoint;
            }
        }
    }

    resetCurveHighlighting();
    if (closestCurve && distMin < 20)
    {
        CVF_ASSERT(closestCurvePoint >= 0);
        caf::PdmObject* selectedPlotObject = ownerPlotDefinition()->findRimPlotObjectFromQwtCurve(closestCurve);

        if (selectedPlotObject)
        {
            RimProject* proj = nullptr;
            selectedPlotObject->firstAncestorOrThisOfType(proj);

            if (proj)
            {
                RiuPlotMainWindowTools::showPlotMainWindow();
                RiuPlotMainWindowTools::selectAsCurrentItem(selectedPlotObject);
                highlightCurve(closestCurve);
            }        
        }
    }
    
    if (closestCurve && distMin < 10)
    {
        selectSample(closestCurve, closestCurvePoint);        
    }
    else
    {
        clearSampleSelection();
    }

    replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::highlightCurve(const QwtPlotCurve* closestCurve)
{
    // NB! Create a copy of the item list before the loop to avoid invalidated iterators when iterating the list
    // plotCurve->setZ() causes the ordering of items in the list to change
    auto plotItemList = this->itemList();
    for (QwtPlotItem* plotItem : plotItemList)
    {
        QwtPlotCurve* plotCurve = dynamic_cast<QwtPlotCurve*>(plotItem);
        if (plotCurve)
        {
            QPen   existingPen = plotCurve->pen();
            QColor bgColor     = this->canvasBackground().color();

            QColor curveColor = existingPen.color();
            QColor symbolColor;
            QColor symbolLineColor;

            QwtSymbol* symbol = const_cast<QwtSymbol*>(plotCurve->symbol());
            if (symbol)
            {
                symbolColor     = symbol->brush().color();
                symbolLineColor = symbol->pen().color();
            }

            double zValue = plotCurve->z();
            if (plotCurve == closestCurve)
            {
                plotCurve->setZ(zValue + 100.0);
            }
            else
            {
                QColor blendedColor           = RiaColorTools::blendQColors(bgColor, curveColor, 3, 1);
                QColor blendedSymbolColor     = RiaColorTools::blendQColors(bgColor, symbolColor, 3, 1);
                QColor blendedSymbolLineColor = RiaColorTools::blendQColors(bgColor, symbolLineColor, 3, 1);

                plotCurve->setPen(blendedColor, existingPen.width(), existingPen.style());
                if (symbol)
                {
                    symbol->setColor(blendedSymbolColor);
                    symbol->setPen(blendedSymbolLineColor, symbol->pen().width(), symbol->pen().style());
                }
            }
            CurveColors curveColors = {curveColor, symbolColor, symbolLineColor};
            m_originalCurveColors.insert(std::make_pair(plotCurve, curveColors));
            m_originalZValues.insert(std::make_pair(plotCurve, zValue));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::resetCurveHighlighting()
{
    // NB! Create a copy of the item list before the loop to avoid invalidated iterators when iterating the list
    // plotCurve->setZ() causes the ordering of items in the list to change
    auto plotItemList = this->itemList();
    for (QwtPlotItem* plotItem : plotItemList)
    {
        QwtPlotCurve* plotCurve = dynamic_cast<QwtPlotCurve*>(plotItem);
        if (plotCurve && m_originalCurveColors.count(plotCurve))
        {
            const QPen& existingPen = plotCurve->pen();
            auto        colors      = m_originalCurveColors[plotCurve];
            double      zValue      = m_originalZValues[plotCurve];
            
            plotCurve->setPen(colors.lineColor, existingPen.width(), existingPen.style());
            plotCurve->setZ(zValue);
            QwtSymbol* symbol = const_cast<QwtSymbol*>(plotCurve->symbol());
            if (symbol)
            {
                symbol->setColor(colors.symbolColor);
                symbol->setPen(colors.symbolLineColor, symbol->pen().width(), symbol->pen().style());
            }
        }
    }
    m_originalCurveColors.clear();
    m_originalZValues.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::onZoomedSlot()
{
    ownerPlotDefinition()->updateZoomWindowFromQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQwtPlot::onAxisClicked(int axis, double value)
{    
    ownerPlotDefinition()->selectAxisInPropertyEditor(axis);
}
