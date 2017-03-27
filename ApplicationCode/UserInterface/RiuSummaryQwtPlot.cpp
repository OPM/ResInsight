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

#include "RiuSummaryQwtPlot.h"

#include "RiaApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include "RiuMainPlotWindow.h"
#include "RiuQwtScalePicker.h"
#include "RiuQwtCurvePointTracker.h"

#include "cafSelectionManager.h"

#include "qwt_date_scale_engine.h"
#include "qwt_date_scale_draw.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_engine.h"

#include <QEvent>
#include <QMenu>
#include <QWheelEvent>

#include <float.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::RiuSummaryQwtPlot(RimSummaryPlot* plotDefinition, QWidget* parent) : QwtPlot(parent)
{
    Q_ASSERT(plotDefinition);
    m_plotDefinition = plotDefinition;

    m_grid = new QwtPlotGrid;
    m_grid->attach(this);

    setDefaults();

    // LeftButton for the zooming
    m_zoomerLeft = new QwtPlotZoomer(canvas());
    m_zoomerLeft->setRubberBandPen(QColor(Qt::black));
    m_zoomerLeft->setTrackerMode(QwtPicker::AlwaysOff);
    m_zoomerLeft->setTrackerPen(QColor(Qt::black));
    m_zoomerLeft->initMousePattern(1);

    // Attach a zoomer for the right axis
    m_zoomerRight = new QwtPlotZoomer(canvas());
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

    // Create a plot picker to display values next to mouse cursor
    m_curvePointTracker = new RiuQwtCurvePointTracker(this, true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::~RiuSummaryQwtPlot()
{
    m_grid->detach();
    delete m_grid;

    if (m_plotDefinition)
    {
        m_plotDefinition->detachAllCurves();
        m_plotDefinition->handleMdiWindowClosed();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RiuSummaryQwtPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::currentVisibleWindow(QwtInterval* leftAxis, QwtInterval* rightAxis, QwtInterval* timeAxis) const
{
    *leftAxis  = axisScaleDiv(yLeft).interval();
    *rightAxis = axisScaleDiv(yRight).interval();
    *timeAxis  = axisScaleDiv(xBottom).interval();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setZoomWindow(const QwtInterval& leftAxis, const QwtInterval& rightAxis, const QwtInterval& timeAxis)
{
    {
        QRectF zoomWindow;
        zoomWindow.setLeft(timeAxis.minValue());
        zoomWindow.setRight(timeAxis.maxValue());
        zoomWindow.setTop(leftAxis.maxValue());
        zoomWindow.setBottom(leftAxis.minValue());

        m_zoomerLeft->zoom(zoomWindow);
    }

    {
        QRectF zoomWindow;
        zoomWindow.setLeft(timeAxis.minValue());
        zoomWindow.setRight(timeAxis.maxValue());
        zoomWindow.setTop(rightAxis.maxValue());
        zoomWindow.setBottom(rightAxis.minValue());

        m_zoomerRight->zoom(zoomWindow);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuSummaryQwtPlot::minimumSizeHint() const
{
    return QSize(0, 100);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    QStringList commandIds;

    caf::SelectionManager::instance()->setSelectedItem(ownerPlotDefinition());

    commandIds << "RicShowPlotDataFeature";

    RimContextCommandBuilder::appendCommandsToMenu(commandIds, &menu);

    if (menu.actions().size() > 0)
    {
        menu.exec(event->globalPos());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuSummaryQwtPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setDefaults()
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

    useDateBasedTimeAxis();

    QFont xAxisFont = axisFont(QwtPlot::xBottom);
    xAxisFont.setPixelSize(11);
    setAxisFont(QwtPlot::xBottom, xAxisFont);
    setAxisMaxMinor(QwtPlot::xBottom, 2);

    QFont yAxisFont = axisFont(QwtPlot::yLeft);
    yAxisFont.setPixelSize(11);
    setAxisFont(QwtPlot::yLeft, yAxisFont);

    setAxisMaxMinor(QwtPlot::yLeft, 3);

    QwtText axisTitleY = axisTitle(QwtPlot::yLeft);
    QFont yAxisTitleFont = axisTitleY.font();
    yAxisTitleFont.setPixelSize(11);
    yAxisTitleFont.setBold(false);
    axisTitleY.setFont(yAxisTitleFont);
    axisTitleY.setRenderFlags(Qt::AlignRight);
    setAxisTitle(QwtPlot::yLeft, axisTitleY);

    QwtLegend* legend = new QwtLegend(this);
    // The legend will be deleted in the destructor of the plot or when 
    // another legend is inserted.
    this->insertLegend(legend, BottomLegend);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useDateBasedTimeAxis()
{
    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw(Qt::UTC);
    scaleDraw->setDateFormat(QwtDate::Year, QString("dd-MM-yyyy"));

    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine(Qt::UTC);
    setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
    setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useTimeBasedTimeAxis()
{
    setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine());
    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuSummaryQwtPlot::eventFilter(QObject* watched, QEvent* event)
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
void RiuSummaryQwtPlot::leaveEvent(QEvent *)
{
    m_curvePointTracker->removeMarkerOnFocusLeave();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::selectClosestCurve(const QPoint& pos)
{
    QwtPlotCurve* closestCurve = NULL;
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

    if(closestCurve && distMin < 20)
    {
        caf::PdmObject* selectedCurve = m_plotDefinition->findRimCurveFromQwtCurve(closestCurve);
        if(selectedCurve)
        {
            RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(selectedCurve);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::onZoomedSlot()
{
    QwtInterval left, right, time;
    currentVisibleWindow(&left, &right, &time);
    
    this->setZoomWindow(left, right, time);
    
    m_plotDefinition->updateZoomWindowFromQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::onAxisClicked(int axis, double value)
{
    if (!m_plotDefinition) return;

    m_plotDefinition->selectAxisInPropertyEditor(axis);
}
