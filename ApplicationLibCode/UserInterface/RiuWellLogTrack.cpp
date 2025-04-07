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

#include "RiaDefines.h"
#include "RiaPlotDefines.h"

#include "RimPlotAxisAnnotation.h"
#include "RimWellLogCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogTrack.h"

#include "Well/RigWellLogCurveData.h"

#include "RiuGuiTheme.h"
#include "RiuPlotAnnotationTool.h"
#include "RiuPlotCurve.h"
#include "RiuPlotCurveInfoTextProvider.h"

#include "RiuQwtPlotTools.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_map.h"
#include "qwt_scale_widget.h"

#include <QWheelEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::RiuWellLogTrack( RimWellLogTrack* track, QWidget* parent /*= nullptr */ )
    : RiuQwtPlotWidget( track, parent )
{
    auto wlp = track->firstAncestorOfType<RimWellLogPlot>();

    bool isVertical = ( wlp && wlp->depthOrientation() == RiaDefines::Orientation::VERTICAL );
    setAxisEnabled( QwtAxis::YLeft, true );
    setAxisEnabled( QwtAxis::YRight, false );
    setAxisEnabled( QwtAxis::XTop, !isVertical );
    setAxisEnabled( QwtAxis::XBottom, isVertical );

    m_annotationTool = std::make_unique<RiuPlotAnnotationTool>();
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
void RiuWellLogTrack::setAxisEnabled( QwtAxis::Position axis, bool enabled )
{
    RiuPlotAxis plotAxis = RiuPlotAxis( RiuQwtPlotTools::fromQwtPlotAxis( axis ) );
    RiuQwtPlotWidget::enableAxis( plotAxis, enabled );

    if ( enabled )
    {
        // Align the canvas with the actual min and max values of the curves
        qwtPlot()->axisScaleEngine( axis )->setAttribute( QwtScaleEngine::Floating, true );
        setAxisScale( plotAxis, 0.0, 100.0 );
        qwtPlot()->axisScaleDraw( axis )->setMinimumExtent( axisExtent( plotAxis ) );

        qwtPlot()->axisWidget( axis )->setMargin( 0 );
    }

    setAxisTitleEnabled( plotAxis, enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::createAnnotationsInPlot( const std::vector<RimPlotAxisAnnotation*>& annotations )
{
    m_annotationTool->detachAllAnnotations();

    // Not required to update annotations in an invisible plot
    if ( !plotDefinition()->showWindow() ) return;

    auto depthTrackPlot = m_plotDefinition->firstAncestorOfType<RimDepthTrackPlot>();
    if ( !depthTrackPlot ) return;

    auto orientation = depthTrackPlot->depthOrientation() == RiaDefines::Orientation::HORIZONTAL ? RiaDefines::Orientation::VERTICAL
                                                                                                 : RiaDefines::Orientation::HORIZONTAL;
    for ( auto annotation : annotations )
    {
        m_annotationTool->attachAnnotationLine( qwtPlot(),
                                                annotation->color(),
                                                annotation->name(),
                                                annotation->penStyle(),
                                                annotation->value(),
                                                orientation,
                                                Qt::AlignRight );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::onMouseMoveEvent( QMouseEvent* mouseEvent )
{
    // The mouse move event here is a mouse move event local to one track. The depth information here must be
    // communicated to all tracks in the same depth track plot. And then, after the depth information has been updated,
    // all well log tracks must be updated with the new depth marker line location.

    if ( !m_plotDefinition ) return;
    if ( mouseEvent->type() != QMouseEvent::MouseMove ) return;

    auto depthTrackPlot = m_plotDefinition->firstAncestorOfType<RimDepthTrackPlot>();
    if ( !depthTrackPlot || !depthTrackPlot->isDepthMarkerLineEnabled() ) return;

    auto plotwidget = dynamic_cast<RiuQwtPlotWidget*>( m_plotDefinition->plotWidget() );
    if ( !plotwidget ) return;

    auto qwtPlot = plotwidget->qwtPlot();
    if ( !qwtPlot ) return;

    auto              riuPlotAxis = depthTrackPlot->depthAxis();
    auto              qwtAxis     = plotwidget->toQwtPlotAxis( riuPlotAxis );
    const QwtScaleMap axisMap     = qwtPlot->canvasMap( qwtAxis );

    double depth = 0.0;
    if ( depthTrackPlot->depthOrientation() == RiaDefines::Orientation::HORIZONTAL )
    {
        depth = axisMap.invTransform( mouseEvent->position().x() );
    }
    else
    {
        depth = axisMap.invTransform( mouseEvent->position().y() );
    }

    depthTrackPlot->setDepthMarkerPosition( depth );

    for ( auto p : depthTrackPlot->plots() )
    {
        auto wellLogTrack = dynamic_cast<RimWellLogTrack*>( p );
        if ( wellLogTrack ) wellLogTrack->updateDepthMarkerLine();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogCurvePointTracker::RiuWellLogCurvePointTracker( QwtPlot*                      plot,
                                                          RiuPlotCurveInfoTextProvider* curveInfoTextProvider,
                                                          RimWellLogTrack*              track )
    : RiuQwtCurvePointTracker( plot, false, curveInfoTextProvider )
    , m_wellLogTrack( track )
{
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtText RiuWellLogCurvePointTracker::trackerText( const QPoint& pos ) const
{
    QwtText txt;

    if ( m_plot )
    {
        QwtAxisId relatedYAxis( QwtAxis::YLeft, 0 );
        QwtAxisId relatedXAxis( QwtAxis::XTop, 0 );

        QString curveInfoText;
        QString depthAxisValueString;
        QString xAxisValueString;
        QPointF closestPoint = closestCurvePoint( pos, &curveInfoText, &xAxisValueString, &depthAxisValueString, &relatedXAxis, &relatedYAxis );
        if ( !closestPoint.isNull() )
        {
            QString str;

            auto wlp = m_wellLogTrack->firstAncestorOfType<RimWellLogPlot>();
            if ( wlp && wlp->depthOrientation() == RiaDefines::Orientation::HORIZONTAL )
            {
                str = QString( "%1\nDepth: %2" ).arg( depthAxisValueString ).arg( xAxisValueString );
            }
            else
            {
                str = QString( "%1\nDepth: %2" ).arg( xAxisValueString ).arg( depthAxisValueString );
            }

            if ( !curveInfoText.isEmpty() )
            {
                str = QString( "%1:\n" ).arg( curveInfoText ) + str;
            }

            txt.setText( str );
        }

        updateClosestCurvePointMarker( closestPoint, relatedXAxis, relatedYAxis );
    }

    auto color = RiuGuiTheme::getColorByVariableName( "markerColor" );
    txt.setColor( color );

    return txt;
}
