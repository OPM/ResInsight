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

#include "RimWellLogCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogTrack.h"

#include "RiuQwtCurvePointTracker.h"
#include "RiuRimQwtPlotCurve.h"

#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

#include <QWheelEvent>

class RiuWellLogCurvePointTracker : public RiuQwtCurvePointTracker
{
public:
    RiuWellLogCurvePointTracker( QwtPlot* plot, IPlotCurveInfoTextProvider* curveInfoTextProvider )
        : RiuQwtCurvePointTracker( plot, false, curveInfoTextProvider )
    {
    }

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QwtText trackerText( const QPoint& pos ) const override
    {
        QwtText txt;

        if ( m_plot )
        {
            QwtPlot::Axis relatedYAxis = QwtPlot::yLeft;
            QwtPlot::Axis relatedXAxis = QwtPlot::xTop;

            QString curveInfoText;
            QString depthAxisValueString;
            QString xAxisValueString;
            QPointF closestPoint = closestCurvePoint( pos,
                                                      &curveInfoText,
                                                      &xAxisValueString,
                                                      &depthAxisValueString,
                                                      &relatedXAxis,
                                                      &relatedYAxis );
            if ( !closestPoint.isNull() )
            {
                QString str = QString( "depth = %1, value = %2" ).arg( depthAxisValueString ).arg( xAxisValueString );

                if ( !curveInfoText.isEmpty() )
                {
                    str = QString( "%1: " ).arg( curveInfoText ) + str;
                }

                txt.setText( str );
            }

            updateClosestCurvePointMarker( closestPoint, relatedXAxis, relatedYAxis );
        }

        return txt;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class WellLogCurveInfoTextProvider : public IPlotCurveInfoTextProvider
{
public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QString curveInfoText( QwtPlotCurve* curve ) override
    {
        RiuRimQwtPlotCurve* riuCurve = dynamic_cast<RiuRimQwtPlotCurve*>( curve );
        RimWellLogCurve*    wlCurve  = nullptr;
        if ( riuCurve )
        {
            wlCurve = dynamic_cast<RimWellLogCurve*>( riuCurve->ownerRimCurve() );
            if ( wlCurve )
            {
                return QString( "%1" ).arg( wlCurve->curveName() );
            }
        }

        return "";
    }
};
static WellLogCurveInfoTextProvider wellLogCurveInfoTextProvider;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::RiuWellLogTrack( RimWellLogTrack* track, QWidget* parent /*= nullptr */ )
    : RiuQwtPlotWidget( track, parent )
{
    setAxisEnabled( QwtPlot::yLeft, true );
    setAxisEnabled( QwtPlot::yRight, false );
    setAxisEnabled( QwtPlot::xTop, true );
    setAxisEnabled( QwtPlot::xBottom, false );

    new RiuWellLogCurvePointTracker( this, &wellLogCurveInfoTextProvider );
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
void RiuWellLogTrack::setAxisEnabled( QwtPlot::Axis axis, bool enabled )
{
    if ( enabled )
    {
        enableAxis( axis, true );

        // Align the canvas with the actual min and max values of the curves
        axisScaleEngine( axis )->setAttribute( QwtScaleEngine::Floating, true );
        setAxisScale( axis, 0.0, 100.0 );
        axisScaleDraw( axis )->setMinimumExtent( axisExtent( axis ) );

        axisWidget( axis )->setMargin( 0 );
        setAxisTitleEnabled( axis, true );
    }
    else
    {
        enableAxis( axis, false );
    }
}
