/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiuQtChartsPlotCurve.h"

#include "RiaPlotDefines.h"

#include "RiuQtChartsPlotCurveSymbol.h"
#include "RiuQtChartsPlotWidget.h"

#include "cvfBoundingBox.h"

#include <QLegend>
#include <QLegendMarker>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotCurve::RiuQtChartsPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title )
    : RiuPlotCurve( ownerRimCurve, title )
{
    m_plotWidget = nullptr;

    m_lineSeries = new QLineSeries();
    m_lineSeries->setName( title );

    m_areaSeries = new QAreaSeries();
    m_areaSeries->setName( title );

    m_scatterSeries = new QScatterSeries();
    m_scatterSeries->setName( title );

    m_axisX = RiuPlotAxis::defaultBottom();
    m_axisY = RiuPlotAxis::defaultLeft();

    m_isVisibleInLegend = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotCurve::~RiuQtChartsPlotCurve()
{
    if ( m_plotWidget && m_plotWidget->qtChart() )
    {
        m_plotWidget->detach( this );

        auto* line = lineSeries();
        if ( line )
        {
            m_plotWidget->qtChart()->removeSeries( line );

            // removeSeries() releases chart ownership of the data, delete data to avoid memory leak
            delete line;
        }

        auto* area = areaSeries();
        if ( area )
        {
            m_plotWidget->qtChart()->removeSeries( area );

            // removeSeries() releases chart ownership of the data, delete data to avoid memory leak
            delete area;
        }

        auto* scatter = scatterSeries();
        if ( scatter )
        {
            m_plotWidget->qtChart()->removeSeries( scatter );

            // removeSeries() releases chart ownership of the data, delete data to avoid memory leak
            delete scatter;
        }
    }

    // Delete if it is still owned by by plot curve
    delete m_lineSeries;
    m_lineSeries = nullptr;

    delete m_areaSeries;
    m_areaSeries = nullptr;

    delete m_scatterSeries;
    m_scatterSeries = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setTitle( const QString& title )
{
    if ( !isQtChartObjectsPresent() ) return;

    lineSeries()->setName( title );
    areaSeries()->setName( title );
    scatterSeries()->setName( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum          lineStyle,
                                          RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolationType,
                                          int                                            requestedCurveThickness,
                                          const QColor&                                  curveColor,
                                          const QBrush&                                  fillBrush /* = QBrush( Qt::NoBrush )*/ )
{
    if ( !isQtChartObjectsPresent() ) return;

    Qt::PenStyle penStyle = RiuQwtPlotCurveDefines::convertToPenStyle( lineStyle );

    QPen curvePen( curveColor );
    curvePen.setWidth( requestedCurveThickness );
    curvePen.setStyle( penStyle );

    lineSeries()->setPen( curvePen );
    lineSeries()->setBrush( fillBrush );

    areaSeries()->setPen( curvePen );
    areaSeries()->setBrush( fillBrush );

    if ( fillBrush.style() == Qt::NoBrush )
    {
        lineSeries()->show();
        areaSeries()->hide();
        setVisibleInLegend( m_isVisibleInLegend );
    }
    else
    {
        lineSeries()->hide();
        areaSeries()->show();
        setVisibleInLegend( m_isVisibleInLegend );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setBrush( const QBrush& brush )
{
    if ( !isQtChartObjectsPresent() ) return;

    lineSeries()->setBrush( brush );
    areaSeries()->setBrush( brush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setColor( const QColor& color )
{
    QPen curvePen = lineSeries()->pen();
    curvePen.setColor( color );
    lineSeries()->setPen( curvePen );

    curvePen = areaSeries()->pen();
    curvePen.setColor( color );
    areaSeries()->setPen( curvePen );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::attachToPlot( RiuPlotWidget* plotWidget )
{
    m_plotWidget = dynamic_cast<RiuQtChartsPlotWidget*>( plotWidget );
    CAF_ASSERT( m_plotWidget );

    if ( m_plotWidget->getLineSeries( this ) && m_plotWidget->getScatterSeries( this ) )
    {
        m_plotWidget->qtChart()->legend()->setMarkerShape( QLegend::MarkerShape::MarkerShapeFromSeries );
        setVisibleInLegend( true );

        lineSeries()->show();
    }
    else
    {
        if ( !m_lineSeries ) m_lineSeries = new QLineSeries();
        if ( !m_areaSeries ) m_areaSeries = new QAreaSeries();
        if ( !m_scatterSeries ) m_scatterSeries = new QScatterSeries();

        m_plotWidget->attach( this, m_lineSeries, m_areaSeries, m_scatterSeries, m_axisX, m_axisY );
        // Plot widget takes ownership.
        m_lineSeries    = nullptr;
        m_areaSeries    = nullptr;
        m_scatterSeries = nullptr;
    }

    if ( scatterSeries() )
    {
        if ( m_symbol )
        {
            scatterSeries()->show();
            auto qtChartsSymbol = dynamic_cast<RiuQtChartsPlotCurveSymbol*>( m_symbol.get() );
            CAF_ASSERT( qtChartsSymbol );
            qtChartsSymbol->applyToScatterSeries( scatterSeries() );
        }
        else
        {
            scatterSeries()->hide();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::detach()
{
    QLineSeries* line = lineSeries();
    if ( line )
    {
        line->hide();
    }

    QAreaSeries* area = areaSeries();
    if ( area )
    {
        area->hide();
    }

    if ( scatterSeries() )
    {
        scatterSeries()->hide();
    }

    if ( m_plotWidget )
    {
        m_plotWidget->detach( this );
        setVisibleInLegend( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::showInPlot()
{
    //  show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setSamplesInPlot( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    if ( !isQtChartObjectsPresent() ) return;

    CAF_ASSERT( xValues.size() == yValues.size() );

    QVector<QPointF> values( static_cast<int>( xValues.size() ) );

    for ( int i = 0; i < static_cast<int>( xValues.size() ); i++ )
    {
        values[i] = QPointF( xValues[i], yValues[i] );
    }

    QLineSeries* line = lineSeries();
    line->replace( values );

    QLineSeries* upper = new QLineSeries;
    upper->replace( values );
    areaSeries()->setUpperSeries( upper );

    updateScatterSeries();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::updateScatterSeries()
{
    if ( !scatterSeries() ) return;

    double minX = std::numeric_limits<double>::max();
    double maxX = -std::numeric_limits<double>::max();

    QVector<QPointF> points;

    if ( lineSeries() )
    {
        points = lineSeries()->pointsVector();

        bool foundAxis = false;

        auto axes = lineSeries()->attachedAxes();
        for ( auto axis : axes )
        {
            if ( axis->orientation() == Qt::Orientation::Horizontal )
            {
                QValueAxis*    valueAxis    = dynamic_cast<QValueAxis*>( axis );
                QDateTimeAxis* dateTimeAxis = dynamic_cast<QDateTimeAxis*>( axis );
                if ( valueAxis )
                {
                    minX      = valueAxis->min();
                    maxX      = valueAxis->max();
                    foundAxis = true;
                }
                else if ( dateTimeAxis )
                {
                    minX      = dateTimeAxis->min().toMSecsSinceEpoch();
                    maxX      = dateTimeAxis->max().toMSecsSinceEpoch();
                    foundAxis = true;
                }
            }
        }

        if ( !foundAxis )
        {
            for ( auto p : points )
            {
                minX = std::min( minX, p.x() );
                maxX = std::max( maxX, p.x() );
            }
        }
    }

    QVector<QPointF> scatterValues;
    if ( !points.empty() )
    {
        double range = maxX - minX;

        double displaySize = 1400;
        if ( m_plotWidget && m_plotWidget->qtChart() )
        {
            // Use the max size since plot area can be small before the widget is shown
            displaySize = std::max( displaySize, m_plotWidget->qtChart()->plotArea().width() );
        }

        double rangePerPixel = range / displaySize;

        double skipDistance = rangePerPixel * m_symbolSkipPixelDistance;

        // Always have symbol on first point
        scatterValues << points[0];

        int lastDrawnIndex = 0;
        for ( int i = 1; i < static_cast<int>( points.size() ); i++ )
        {
            // Skip points until skip distance is reached
            double diff = points[i].x() - points[lastDrawnIndex].x();

            // Always add last point.
            bool isLastPoint = i == points.size() - 1;
            if ( diff > skipDistance || isLastPoint )
            {
                scatterValues << points[i];
                lastDrawnIndex = i;
            }
        }
    }

    scatterSeries()->replace( scatterValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotCurve::isQtChartObjectsPresent() const
{
    return lineSeries() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setZ( int z )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::clearErrorBars()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::updateErrorBarsAppearance( bool showErrorBars, const QColor& curveColor )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setXAxis( RiuPlotAxis axis )
{
    m_axisX = axis;
    if ( m_plotWidget )
    {
        m_plotWidget->setXAxis( axis, lineSeries(), this );
        m_plotWidget->setXAxis( axis, areaSeries(), this );
        m_plotWidget->setXAxis( axis, scatterSeries(), this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setYAxis( RiuPlotAxis axis )
{
    m_axisY = axis;
    if ( m_plotWidget )
    {
        m_plotWidget->setYAxis( axis, lineSeries(), this );
        m_plotWidget->setYAxis( axis, areaSeries(), this );
        m_plotWidget->setYAxis( axis, scatterSeries(), this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotCurve::numSamples() const
{
    if ( !lineSeries() ) return 0;

    return lineSeries()->count();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQtChartsPlotCurve::sample( int index ) const
{
    CAF_ASSERT( index >= 0 && index <= numSamples() );
    auto p = lineSeries()->at( index );
    return std::make_pair( p.x(), p.y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQtChartsPlotCurve::xDataRange() const
{
    cvf::BoundingBox bb = computeBoundingBox();
    return std::make_pair( bb.min().x(), bb.max().x() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQtChartsPlotCurve::yDataRange() const
{
    cvf::BoundingBox bb = computeBoundingBox();
    return std::make_pair( bb.min().y(), bb.max().y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RiuQtChartsPlotCurve::computeBoundingBox() const
{
    auto points = lineSeries()->pointsVector();

    cvf::BoundingBox bb;
    for ( auto p : points )
        bb.add( cvf::Vec3d( p.x(), p.y(), 0.0 ) );

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setVisibleInLegend( bool isVisibleInLegend )
{
    if ( !m_plotWidget ) return;

    CAF_ASSERT( m_plotWidget->qtChart() );
    CAF_ASSERT( m_plotWidget->qtChart()->legend() );

    // The markers can be set visible independent to the visibility state of the containing legend. Use the
    // visibility state of the legend to override the visibility flag
    if ( !m_plotWidget->qtChart()->legend()->isAttachedToChart() ) isVisibleInLegend = false;
    if ( !m_plotWidget->qtChart()->legend()->isVisible() ) isVisibleInLegend = false;

    bool showScatterMarker = isVisibleInLegend && m_symbol;
    bool showLineMarker    = isVisibleInLegend && !m_symbol;

    auto setLegendVisibility = [this]( auto series, bool isVisible )
    {
        if ( series )
        {
            auto markers = m_plotWidget->qtChart()->legend()->markers( series );
            if ( !markers.isEmpty() ) markers[0]->setVisible( isVisible );
        }
    };

    m_isVisibleInLegend = showLineMarker || showScatterMarker;
    setLegendVisibility( lineSeries(), showLineMarker );
    setLegendVisibility( areaSeries(), false );
    setLegendVisibility( scatterSeries(), showScatterMarker );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLineSeries* RiuQtChartsPlotCurve::lineSeries() const
{
    if ( m_lineSeries ) return m_lineSeries;
    if ( m_plotWidget ) return dynamic_cast<QLineSeries*>( m_plotWidget->getLineSeries( this ) );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QScatterSeries* RiuQtChartsPlotCurve::scatterSeries() const
{
    if ( m_scatterSeries ) return m_scatterSeries;
    if ( m_plotWidget ) return dynamic_cast<QScatterSeries*>( m_plotWidget->getScatterSeries( this ) );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAreaSeries* RiuQtChartsPlotCurve::areaSeries() const
{
    if ( m_areaSeries ) return m_areaSeries;
    if ( m_plotWidget ) return dynamic_cast<QAreaSeries*>( m_plotWidget->getAreaSeries( this ) );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setSymbol( RiuPlotCurveSymbol* symbol )
{
    if ( symbol )
    {
        auto qtChartsSymbol = dynamic_cast<RiuQtChartsPlotCurveSymbol*>( symbol );
        CAF_ASSERT( qtChartsSymbol );
        m_symbol.reset( symbol );

        if ( scatterSeries() )
        {
            qtChartsSymbol->applyToScatterSeries( scatterSeries() );
            updateScatterSeries();
            updateLineAndAreaSeries();
        }
    }
    else
    {
        m_symbol.reset();
        if ( scatterSeries() ) scatterSeries()->hide();
        updateLineAndAreaSeries();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::updateLineAndAreaSeries()
{
    bool isFilled = areaSeries() && areaSeries()->brush().style() != Qt::NoBrush;
    bool isLine   = lineSeries() && lineSeries()->pen().style() != Qt::PenStyle::NoPen;
    if ( areaSeries() ) areaSeries()->setVisible( isFilled );
    if ( lineSeries() ) lineSeries()->setVisible( isLine );
    setVisibleInLegend( m_isVisibleInLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol* RiuQtChartsPlotCurve::createSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbol ) const
{
    return new RiuQtChartsPlotCurveSymbol( symbol );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setCurveFittingTolerance( double tolerance )
{
    // Not supported
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setLegendIconSize( const QSize& iconSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQtChartsPlotCurve::legendIconSize() const
{
    // Default from Qwt
    return QSize( 8, 8 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPixmap RiuQtChartsPlotCurve::legendIcon( const QSizeF& iconSize ) const
{
    return QPixmap();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::axisRangeChanged()
{
    updateScatterSeries();
}
