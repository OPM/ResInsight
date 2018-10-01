/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiuQwtCurvePointTracker.h"

#include "RiaQDateTimeTools.h"

#include "qwt_plot_marker.h"
#include "qwt_symbol.h"

#include "qwt_plot_curve.h"
#include "qwt_date_scale_draw.h"

#include <float.h> // For DBL_MAX

#include <QEvent>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQwtCurvePointTracker::RiuQwtCurvePointTracker(QwtPlot* plot, bool isMainAxisHorizontal, IPlotCurveInfoTextProvider* curveInfoTextProvider)
    : QwtPlotPicker(plot->canvas()), m_plot(plot), m_isMainAxisHorizontal(isMainAxisHorizontal), m_curveInfoTextProvider(curveInfoTextProvider)
{
    this->setTrackerMode(QwtPicker::AlwaysOn);
    m_plotMarker = new QwtPlotMarker;

    // QwtPlotMarker takes ownership of the symbol, it is deleted in destructor of QwtPlotMarker
    QwtSymbol* mySymbol = new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush, QPen(Qt::black, 2.0), QSize(12, 12));
    m_plotMarker->setSymbol(mySymbol);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQwtCurvePointTracker::~RiuQwtCurvePointTracker()
{
    m_plotMarker->detach();
    delete m_plotMarker;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQwtCurvePointTracker::removeMarkerOnFocusLeave()
{
    if ( m_plotMarker->plot() )
    {
        m_plotMarker->detach();

        m_plot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuQwtCurvePointTracker::eventFilter(QObject *watched, QEvent *event)
{
    if ( event->type() == QEvent::Leave )
    {
        this->removeMarkerOnFocusLeave();
    }

    // pass the event on to the parent class
    return QwtPlotPicker::eventFilter(watched, event);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtText RiuQwtCurvePointTracker::trackerText(const QPoint& pos) const
{
    QwtText txt;

    if ( m_plot )
    {
        QwtPlot::Axis relatedYAxis = QwtPlot::yLeft;
        QwtPlot::Axis relatedXAxis = QwtPlot::xBottom;

        QString curveInfoText;
        QString mainAxisValueString;
        QString valueAxisValueString;
        QPointF closestPoint = closestCurvePoint(pos, &curveInfoText, &valueAxisValueString, &mainAxisValueString, &relatedXAxis, &relatedYAxis);
        if ( !closestPoint.isNull() )
        {
            QString str = !curveInfoText.isEmpty() ?
                QString("%1: %2").arg(curveInfoText).arg(valueAxisValueString) :
                valueAxisValueString;

            if ( !mainAxisValueString.isEmpty() )
            {
                str += QString(" (%1)").arg(mainAxisValueString);
            }

            txt.setText(str);
        }

        updateClosestCurvePointMarker(closestPoint, relatedXAxis, relatedYAxis);
    }

    return txt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QPointF RiuQwtCurvePointTracker::closestCurvePoint(const QPoint& cursorPosition,
                                                   QString* curveInfoText,
                                                   QString* valueAxisValueString,
                                                   QString* mainAxisValueString,
                                                   QwtPlot::Axis* relatedXAxis,
                                                   QwtPlot::Axis* relatedYAxis) const
{
    QPointF samplePoint;

    QwtPlotCurve* closestCurve = nullptr;
    double distMin = DBL_MAX;
    int closestPointSampleIndex = -1;

    const QwtPlotItemList& itmList = m_plot->itemList();
    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); it++ )
    {
        if ( (*it)->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve* candidateCurve = static_cast<QwtPlotCurve*>(*it);
            double dist = DBL_MAX;
            int candidateSampleIndex = candidateCurve->closestPoint(cursorPosition, &dist);
            if ( dist < distMin )
            {
                closestCurve = candidateCurve;
                distMin = dist;
                closestPointSampleIndex = candidateSampleIndex;
            }
        }
    }

    if ( closestCurve && distMin < 50 )
    {
        samplePoint = closestCurve->sample(closestPointSampleIndex);

        if ( relatedXAxis ) *relatedXAxis = static_cast<QwtPlot::Axis>(closestCurve->xAxis());
        if ( relatedYAxis ) *relatedYAxis = static_cast<QwtPlot::Axis>(closestCurve->yAxis());
    }


    if ( mainAxisValueString )
    {
        const QwtScaleDraw* mainAxisScaleDraw = m_isMainAxisHorizontal ? m_plot->axisScaleDraw(*relatedXAxis): m_plot->axisScaleDraw(*relatedYAxis);
        auto dateScaleDraw = dynamic_cast<const QwtDateScaleDraw*>(mainAxisScaleDraw) ;

        qreal mainAxisSampleVal = 0.0;
        if ( m_isMainAxisHorizontal )
            mainAxisSampleVal = samplePoint.x();
        else
            mainAxisSampleVal = samplePoint.y();

        if (curveInfoText && closestCurve && m_curveInfoTextProvider)
        {
            *curveInfoText = m_curveInfoTextProvider->curveInfoText(closestCurve);
        }

        if ( dateScaleDraw )
        {
            QDateTime date = dateScaleDraw->toDateTime(mainAxisSampleVal);
            
            QString dateString = RiaQDateTimeTools::toStringUsingApplicationLocale(date, "hh:mm dd.MMMM.yyyy");
            *mainAxisValueString = dateString;
        }
        else if ( mainAxisScaleDraw )
        {
            *mainAxisValueString = mainAxisScaleDraw->label(mainAxisSampleVal).text();
        }
    }

    if ( valueAxisValueString && closestCurve )
    {
        const QwtScaleDraw* valueAxisScaleDraw =  m_isMainAxisHorizontal ? m_plot->axisScaleDraw(*relatedYAxis): m_plot->axisScaleDraw(*relatedXAxis);

        qreal valueAxisSampleVal = 0.0;
        if ( m_isMainAxisHorizontal )
            valueAxisSampleVal = samplePoint.y();
        else
            valueAxisSampleVal = samplePoint.x();

        if ( valueAxisScaleDraw )
        {
            *valueAxisValueString = valueAxisScaleDraw->label(valueAxisSampleVal).text();
        }
    }

    return samplePoint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQwtCurvePointTracker::updateClosestCurvePointMarker(const QPointF& closestPoint, QwtPlot::Axis relatedXAxis, QwtPlot::Axis relatedYAxis) const
{
    bool replotRequired = false;

    if ( !closestPoint.isNull() )
    {
        if ( !m_plotMarker->plot() )
        {
            m_plotMarker->attach(m_plot);

            replotRequired = true;
        }

        if ( m_plotMarker->value() != closestPoint )
        {
            m_plotMarker->setValue(closestPoint.x(), closestPoint.y());

            // Set the axes that the marker realtes to, to make the positioning correct
            m_plotMarker->setAxes(relatedXAxis, relatedYAxis);

            // TODO : Should use a color or other visual indicator to show what axis the curve relates to

            replotRequired = true;
        }
    }
    else
    {
        if ( m_plotMarker->plot() )
        {
            m_plotMarker->detach();

            replotRequired = true;
        }
    }

    if ( replotRequired ) m_plot->replot();
}
