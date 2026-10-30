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

#include <cmath>
#include <set>

namespace
{
//==================================================================================================
/// Scale draw replacing the numeric tick label by a custom text for integer tick values
//==================================================================================================
class RiuDepthAxisTickLabelScaleDraw : public QwtScaleDraw
{
public:
    void setTickLabels( const std::map<int, QString>& tickLabels )
    {
        m_tickLabels = tickLabels;
        invalidateCache();
    }

    QwtText label( double value ) const override
    {
        const double rounded = std::round( value );
        if ( std::abs( value - rounded ) < 1.0e-6 )
        {
            auto it = m_tickLabels.find( std::lround( value ) );
            if ( it != m_tickLabels.end() ) return QwtText( it->second );
        }

        return QwtScaleDraw::label( value );
    }

private:
    std::map<int, QString> m_tickLabels;
};

//==================================================================================================
/// Linear scale engine placing ticks at integer values only. Major ticks are either automatic (with a
/// step of at least one) or at every integer. Minor ticks are placed at the remaining integers.
//==================================================================================================
class RiuIntegerTickScaleEngine : public QwtLinearScaleEngine
{
public:
    enum class Mode
    {
        OFF,
        AUTO_MAJOR_TICKS,
        MAJOR_TICK_AT_EVERY_INTEGER
    };

    void setMode( Mode mode ) { m_mode = mode; }

    QwtScaleDiv divideScale( double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize ) const override
    {
        if ( m_mode == Mode::OFF ) return QwtLinearScaleEngine::divideScale( x1, x2, maxMajorSteps, maxMinorSteps, stepSize );

        // Avoid creating a huge number of ticks when the axis range is large
        const double maxIntegerTickCount = 1000.0;
        const bool   isRangeLimited      = std::abs( x2 - x1 ) < maxIntegerTickCount;

        if ( m_mode == Mode::MAJOR_TICK_AT_EVERY_INTEGER && isRangeLimited ) stepSize = 1.0;

        QwtScaleDiv scaleDiv = QwtLinearScaleEngine::divideScale( x1, x2, maxMajorSteps, 0, stepSize );

        QList<double> majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
        if ( majorTicks.size() > 1 )
        {
            // Use an integer step of at least one to keep the major ticks at integer values
            const double step        = std::abs( majorTicks[1] - majorTicks[0] );
            const double integerStep = std::max( 1.0, std::round( step ) );
            if ( std::abs( step - integerStep ) > 1e-6 )
            {
                scaleDiv   = QwtLinearScaleEngine::divideScale( x1, x2, maxMajorSteps, 0, integerStep );
                majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
            }
        }

        QList<double> minorTicks;
        if ( m_mode == Mode::AUTO_MAJOR_TICKS && isRangeLimited )
        {
            std::set<long long> majorTickValues;
            for ( double tick : majorTicks )
            {
                majorTickValues.insert( std::llround( tick ) );
            }

            const double first = std::ceil( std::min( x1, x2 ) );
            const double last  = std::floor( std::max( x1, x2 ) );

            for ( double value = first; value <= last; value += 1.0 )
            {
                if ( !majorTickValues.contains( std::llround( value ) ) ) minorTicks.push_back( value );
            }
        }

        scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );
        scaleDiv.setTicks( QwtScaleDiv::MediumTick, {} );

        return scaleDiv;
    }

private:
    Mode m_mode = Mode::OFF;
};
} // namespace

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
/// Replace the numeric depth axis tick labels by custom text for the given integer depth values.
/// Ticks without a custom text show the numeric value.
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setDepthAxisTickLabels( const std::map<int, QString>& tickLabels )
{
    auto depthTrackPlot = m_plotDefinition ? m_plotDefinition->firstAncestorOfType<RimDepthTrackPlot>() : nullptr;
    if ( !depthTrackPlot ) return;

    const auto qwtAxis = toQwtPlotAxis( depthTrackPlot->depthAxis() );

    auto tickLabelScaleDraw = dynamic_cast<RiuDepthAxisTickLabelScaleDraw*>( qwtPlot()->axisScaleDraw( qwtAxis ) );
    if ( !tickLabelScaleDraw )
    {
        if ( tickLabels.empty() ) return;

        const QwtScaleDraw* currentScaleDraw = qwtPlot()->axisScaleDraw( qwtAxis );

        tickLabelScaleDraw = new RiuDepthAxisTickLabelScaleDraw;
        tickLabelScaleDraw->setMinimumExtent( currentScaleDraw->minimumExtent() );
        for ( auto component : { QwtAbstractScaleDraw::Backbone, QwtAbstractScaleDraw::Ticks, QwtAbstractScaleDraw::Labels } )
        {
            tickLabelScaleDraw->enableComponent( component, currentScaleDraw->hasComponent( component ) );
        }

        // The plot takes ownership of the scale draw
        qwtPlot()->setAxisScaleDraw( qwtAxis, tickLabelScaleDraw );
    }

    tickLabelScaleDraw->setTickLabels( tickLabels );
    scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
/// Place depth axis ticks at integer values only, e.g. for connection numbers. When tickAtEveryInteger is
/// set, every integer gets a major tick, otherwise the major ticks are automatic and minor ticks fill in.
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setDepthAxisIntegerTicks( bool enabled, bool tickAtEveryInteger )
{
    using Mode = RiuIntegerTickScaleEngine::Mode;

    auto depthTrackPlot = m_plotDefinition ? m_plotDefinition->firstAncestorOfType<RimDepthTrackPlot>() : nullptr;
    if ( !depthTrackPlot ) return;

    const auto qwtAxis = toQwtPlotAxis( depthTrackPlot->depthAxis() );

    auto scaleEngine = dynamic_cast<RiuIntegerTickScaleEngine*>( qwtPlot()->axisScaleEngine( qwtAxis ) );
    if ( !scaleEngine )
    {
        if ( !enabled ) return;

        const QwtScaleEngine* currentScaleEngine = qwtPlot()->axisScaleEngine( qwtAxis );

        scaleEngine = new RiuIntegerTickScaleEngine;
        scaleEngine->setAttributes( currentScaleEngine->attributes() );
        scaleEngine->setReference( currentScaleEngine->reference() );
        scaleEngine->setMargins( currentScaleEngine->lowerMargin(), currentScaleEngine->upperMargin() );

        // The plot takes ownership of the scale engine
        qwtPlot()->setAxisScaleEngine( qwtAxis, scaleEngine );
    }

    Mode mode = Mode::OFF;
    if ( enabled )
    {
        mode = tickAtEveryInteger ? Mode::MAJOR_TICK_AT_EVERY_INTEGER : Mode::AUTO_MAJOR_TICKS;
    }
    scaleEngine->setMode( mode );

    // Reapply the axis range to recompute the ticks
    const QwtInterval currentRange = qwtPlot()->axisInterval( qwtAxis );
    setAxisScale( qwtAxis, currentRange.minValue(), currentRange.maxValue() );
    scheduleReplot();
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
