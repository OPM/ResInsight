/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RiuRftCorrelationPlotTools.h"

#include "RiuGroupedBarChartBuilder.h"
#include "RiuPlotCurve.h"
#include "RiuQwtCurveSelectorFilter.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtSymbol.h"

#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_zoneitem.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Shows the title of the curve closest to the mouse cursor.
//--------------------------------------------------------------------------------------------------
class CurveTracker : public QwtPlotPicker
{
public:
    CurveTracker( QwtPlot* plot )
        : QwtPlotPicker( plot->canvas() )
    {
        setStateMachine( new QwtPickerTrackerMachine() );
        setRubberBand( QwtPicker::NoRubberBand );
        setTrackerMode( QwtPicker::AlwaysOn );
    }

protected:
    QwtText trackerText( const QPoint& pos ) const override
    {
        double  minDistance = std::numeric_limits<double>::max();
        QString closestCurveLabel;

        for ( QwtPlotItem* item : plot()->itemList() )
        {
            if ( item->rtti() == QwtPlotItem::Rtti_PlotCurve )
            {
                auto   curve    = static_cast<QwtPlotCurve*>( item );
                double distance = std::numeric_limits<double>::max();
                curve->closestPoint( pos, &distance );

                if ( distance < minDistance )
                {
                    minDistance       = distance;
                    closestCurveLabel = curve->title().text();
                }
            }
        }

        if ( minDistance < 20.0 )
        {
            QwtText text( closestCurveLabel );
            text.setBackgroundBrush( QBrush( Qt::white ) );
            text.setColor( Qt::black );
            return text;
        }
        return QwtText();
    }
};

//--------------------------------------------------------------------------------------------------
/// Thin gray translucent bar drawn in pixel width along the left edge (vertical depth axis) or
/// bottom edge (horizontal depth axis) of the canvas, spanning a depth interval.
//--------------------------------------------------------------------------------------------------
class DepthIntervalBar : public QwtPlotItem
{
public:
    DepthIntervalBar( double top, double base, bool isDepthVertical )
        : m_top( top )
        , m_base( base )
        , m_isDepthVertical( isDepthVertical )
    {
        setZ( 5.0 );
    }

    void draw( QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect ) const override
    {
        constexpr double barWidth = 6.0;

        QRectF bar;
        if ( m_isDepthVertical )
        {
            const double y0 = yMap.transform( m_top );
            const double y1 = yMap.transform( m_base );
            bar             = QRectF( canvasRect.left(), std::min( y0, y1 ), barWidth, std::abs( y1 - y0 ) );
        }
        else
        {
            const double x0 = xMap.transform( m_top );
            const double x1 = xMap.transform( m_base );
            bar             = QRectF( std::min( x0, x1 ), canvasRect.bottom() - barWidth, std::abs( x1 - x0 ), barWidth );
        }

        painter->fillRect( bar, QColor( 90, 90, 90, 110 ) );
    }

private:
    double m_top;
    double m_base;
    bool   m_isDepthVertical;
};

//--------------------------------------------------------------------------------------------------
/// Reports the depth value under a left-button click (not drag) on a depth track canvas.
//--------------------------------------------------------------------------------------------------
class DepthClickFilter : public QObject
{
public:
    DepthClickFilter( QwtPlot* plot, QwtAxisId depthAxis, bool isDepthVertical, std::function<void( double )> callback, QObject* parent )
        : QObject( parent )
        , m_plot( plot )
        , m_depthAxis( depthAxis )
        , m_isDepthVertical( isDepthVertical )
        , m_callback( std::move( callback ) )
    {
    }

    bool eventFilter( QObject*, QEvent* event ) override
    {
        if ( !m_plot ) return false;

        if ( event->type() == QEvent::MouseButtonPress )
        {
            auto* mouseEvent = static_cast<QMouseEvent*>( event );
            if ( mouseEvent->button() == Qt::LeftButton ) m_pressPos = mouseEvent->pos();
        }
        else if ( event->type() == QEvent::MouseButtonRelease )
        {
            auto* mouseEvent = static_cast<QMouseEvent*>( event );
            if ( mouseEvent->button() == Qt::LeftButton && m_pressPos && ( mouseEvent->pos() - *m_pressPos ).manhattanLength() < 4 )
            {
                const double pixel = m_isDepthVertical ? mouseEvent->pos().y() : mouseEvent->pos().x();
                const double depth = m_plot->invTransform( m_depthAxis, pixel );

                // Defer so the model and plot updates run after this mouse event is fully handled
                QTimer::singleShot( 0, this, [this, depth]() { m_callback( depth ); } );
            }
            m_pressPos.reset();
        }
        return false;
    }

private:
    QPointer<QwtPlot>             m_plot;
    QwtAxisId                     m_depthAxis;
    bool                          m_isDepthVertical;
    std::function<void( double )> m_callback;
    std::optional<QPoint>         m_pressPos;
};
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::detachAllItems( RiuQwtPlotWidget* plotWidget )
{
    if ( plotWidget ) plotWidget->qwtPlot()->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::attachPointSeries( RiuQwtPlotWidget* plotWidget, const PointSeries& series )
{
    auto* plotCurve = new RiuQwtPlotCurve;
    plotCurve->setSamplesValues( series.x, series.y );
    plotCurve->setStyle( QwtPlotCurve::NoCurve );

    auto* symbol = new RiuQwtSymbol( series.isSelected ? RiuPlotCurveSymbol::SYMBOL_XCROSS : RiuPlotCurveSymbol::SYMBOL_ELLIPSE );
    symbol->setSize( 8, 8 );
    symbol->setColor( series.color );
    plotCurve->setSymbol( symbol );

    plotCurve->setTitle( series.title );
    plotCurve->attach( plotWidget->qwtPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::attachHorizontalLine( RiuQwtPlotWidget* plotWidget,
                                                       double            yValue,
                                                       Qt::PenStyle      style,
                                                       const QString&    label,
                                                       const QColor&     color )
{
    auto* marker = new QwtPlotMarker();
    marker->setLineStyle( QwtPlotMarker::HLine );
    marker->setYValue( yValue );
    QPen pen( color );
    pen.setStyle( style );
    pen.setWidth( 1 );
    marker->setLinePen( pen );

    if ( !label.isEmpty() )
    {
        QwtText text( label );
        text.setColor( Qt::black );
        marker->setLabel( text );
        marker->setLabelAlignment( Qt::AlignTop | Qt::AlignLeft );
    }

    marker->setZ( 1000.0 );
    marker->attach( plotWidget->qwtPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::attachHorizontalBand( RiuQwtPlotWidget* plotWidget, double minValue, double maxValue, const QColor& color )
{
    auto* shading = new QwtPlotZoneItem();
    shading->setOrientation( Qt::Horizontal );
    shading->setInterval( minValue, maxValue );
    shading->setPen( color, 0.0, Qt::NoPen );
    shading->setBrush( QBrush( color ) );
    shading->setZ( 999.0 );
    shading->attach( plotWidget->qwtPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::installCurveTracker( RiuQwtPlotWidget* plotWidget )
{
    new CurveTracker( plotWidget->qwtPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::installCurveSelector( RiuQwtPlotWidget*                                     plotWidget,
                                                       std::function<const caf::PdmUiItem*( const QPoint& )> finder )
{
    new RiuQwtCurveSelectorFilter( plotWidget->qwtPlot(), std::move( finder ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuRftCorrelationPlotTools::closestPointIndex( RiuQwtPlotWidget*                             plotWidget,
                                                   const QPoint&                                 canvasPos,
                                                   const std::vector<std::pair<double, double>>& points )
{
    return RiuQwtCurveSelectorFilter::closestPointIndex( plotWidget->qwtPlot(), canvasPos, points );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::installDepthClickFilter( RiuQwtPlotWidget*             plotWidget,
                                                          const RiuPlotAxis&            depthAxis,
                                                          bool                          isDepthVertical,
                                                          std::function<void( double )> callback,
                                                          QObject*                      filterParent )
{
    if ( !plotWidget || !plotWidget->qwtPlot() ) return;

    auto* filter =
        new DepthClickFilter( plotWidget->qwtPlot(), plotWidget->toQwtPlotAxis( depthAxis ), isDepthVertical, std::move( callback ), filterParent );
    plotWidget->qwtPlot()->canvas()->installEventFilter( filter );
}

//--------------------------------------------------------------------------------------------------
/// Stale bars are found by type and removed before the new ones are attached.
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::setDepthIntervalBars( RiuQwtPlotWidget*                 plotWidget,
                                                       const std::vector<DepthInterval>& intervals,
                                                       bool                              isDepthVertical )
{
    if ( !plotWidget ) return;

    QwtPlot* qwtPlot = plotWidget->qwtPlot();
    if ( !qwtPlot ) return;

    // itemList() is a live reference; copy it before deleting items
    const QwtPlotItemList items = qwtPlot->itemList();
    for ( QwtPlotItem* item : items )
    {
        if ( dynamic_cast<DepthIntervalBar*>( item ) ) delete item;
    }

    for ( const auto& interval : intervals )
    {
        auto* bar = new DepthIntervalBar( interval.top, interval.base, isDepthVertical );
        bar->attach( qwtPlot );
    }

    qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::addHorizontalBarChart( RiuQwtPlotWidget* plotWidget, RiuGroupedBarChartBuilder& chartBuilder, int maxBarCount )
{
    chartBuilder.addBarChartToPlot( plotWidget->qwtPlot(), Qt::Horizontal, maxBarCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::detachBarChartItems( RiuQwtPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotBarChart );
    plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotScale );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRftCorrelationPlotTools::removeLegend( RiuQwtPlotWidget* plotWidget )
{
    plotWidget->qwtPlot()->insertLegend( nullptr );
}
