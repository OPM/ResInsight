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
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"

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
    if (ownerViewWindow())
    {
        ownerViewWindow()->handleMdiWindowClosed();
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

    const QwtPlotItemList& itmList = itemList();
    for(QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); it++)
    {
        if((*it)->rtti() == QwtPlotItem::Rtti_PlotCurve)
        {
            QwtPlotCurve* candidateCurve = static_cast<QwtPlotCurve*>(*it);
            double dist = DBL_MAX;
            candidateCurve->closestPoint(pos, &dist);
            if(dist < distMin)
            {
                closestCurve = candidateCurve;
                distMin = dist;
            }
        }
    }

    if (closestCurve && distMin < 20)
    {
        caf::PdmObject* selectedPlotObject = ownerPlotDefinition()->findRimPlotObjectFromQwtCurve(closestCurve);

        if (selectedPlotObject)
        {
            RimProject* proj = nullptr;
            selectedPlotObject->firstAncestorOrThisOfType(proj);

            if (proj)
            {
                RiuPlotMainWindowTools::showPlotMainWindow();
                RiuPlotMainWindowTools::selectAsCurrentItem(selectedPlotObject);
            }
        }
    }
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
