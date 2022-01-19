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
#include <QtCharts/QChartView>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotCurve::RiuQtChartsPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title )
    : RiuPlotCurve( ownerRimCurve, title )
{
    m_plotWidget = nullptr;

    m_lineSeries = new QtCharts::QLineSeries();
    m_lineSeries->setName( title );

    m_scatterSeries = new QtCharts::QScatterSeries();
    m_scatterSeries->setName( title );

    m_axisX = RiuPlotAxis::defaultBottom();
    m_axisY = RiuPlotAxis::defaultLeft();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotCurve::~RiuQtChartsPlotCurve()
{
    detach();

    // Delete if it is still owned by by plot curve
    delete m_lineSeries;
    m_lineSeries = nullptr;

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
    scatterSeries()->setName( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum          lineStyle,
                                          RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolationType,
                                          int                                            requestedCurveThickness,
                                          const QColor&                                  curveColor,
                                          const QBrush& fillBrush /* = QBrush( Qt::NoBrush )*/ )
{
    if ( !isQtChartObjectsPresent() ) return;

    Qt::PenStyle penStyle = RiuQwtPlotCurveDefines::convertToPenStyle( lineStyle );

    QPen curvePen( curveColor );
    curvePen.setWidth( requestedCurveThickness );
    curvePen.setStyle( penStyle );

    lineSeries()->setPen( curvePen );
    lineSeries()->setBrush( fillBrush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setBrush( const QBrush& brush )
{
    if ( !isQtChartObjectsPresent() ) return;

    lineSeries()->setBrush( brush );
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
        m_plotWidget->qtChart()->legend()->setMarkerShape( QtCharts::QLegend::MarkerShape::MarkerShapeFromSeries );
        setVisibleInLegend( true );
        lineSeries()->show();
    }
    else
    {
        if ( !m_lineSeries ) m_lineSeries = new QtCharts::QLineSeries();
        if ( !m_scatterSeries ) m_scatterSeries = new QtCharts::QScatterSeries();

        m_plotWidget->attach( this, m_lineSeries, m_scatterSeries, m_axisX, m_axisY );
        // Plot widget takes ownership.
        m_lineSeries    = nullptr;
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
    QtCharts::QLineSeries* line = lineSeries();
    if ( line )
    {
        line->hide();
    }

    if ( scatterSeries() )
    {
        scatterSeries()->hide();
    }

    if ( m_plotWidget ) setVisibleInLegend( false );

    m_plotWidget = nullptr;
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
void RiuQtChartsPlotCurve::setSamplesInPlot( const std::vector<double>& xValues,
                                             const std::vector<double>& yValues,
                                             int                        numValues )
{
    if ( !isQtChartObjectsPresent() ) return;

    CAF_ASSERT( xValues.size() == yValues.size() );
    CAF_ASSERT( numValues <= static_cast<int>( xValues.size() ) );
    CAF_ASSERT( numValues >= 0 );

    QtCharts::QLineSeries*    line    = lineSeries();
    QtCharts::QScatterSeries* scatter = scatterSeries();

    line->clear();
    scatter->clear();
    for ( int i = 0; i < numValues; i++ )
    {
        line->append( xValues[i], yValues[i] );
        scatter->append( xValues[i], yValues[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotCurve::isQtChartObjectsPresent() const
{
    if ( !lineSeries() ) return false;

    return true;
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
        m_plotWidget->setXAxis( axis, lineSeries() );
        m_plotWidget->setXAxis( axis, scatterSeries() );
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
        m_plotWidget->setYAxis( axis, lineSeries() );
        m_plotWidget->setYAxis( axis, scatterSeries() );
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

    // The markers can be set visible independent to the visibility state of the containing legend. Use the visibility
    // state of the legend to override the visibility flag
    if ( !m_plotWidget->qtChart()->legend()->isAttachedToChart() ) isVisibleInLegend = false;
    if ( !m_plotWidget->qtChart()->legend()->isVisible() ) isVisibleInLegend = false;

    bool showScatterMarker = isVisibleInLegend;
    if ( !m_symbol ) showScatterMarker = false;

    if ( scatterSeries() )
    {
        auto markers = m_plotWidget->qtChart()->legend()->markers( scatterSeries() );
        if ( !markers.isEmpty() ) markers[0]->setVisible( showScatterMarker );
    }

    bool showLineMarker = isVisibleInLegend;
    if ( showScatterMarker ) showLineMarker = false;

    if ( lineSeries() )
    {
        auto lineSeriesMarkers = m_plotWidget->qtChart()->legend()->markers( lineSeries() );
        if ( !lineSeriesMarkers.isEmpty() ) lineSeriesMarkers[0]->setVisible( showLineMarker );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QtCharts::QLineSeries* RiuQtChartsPlotCurve::lineSeries() const
{
    if ( m_lineSeries ) return m_lineSeries;
    if ( m_plotWidget ) return dynamic_cast<QtCharts::QLineSeries*>( m_plotWidget->getLineSeries( this ) );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QtCharts::QScatterSeries* RiuQtChartsPlotCurve::scatterSeries() const
{
    if ( m_scatterSeries ) return m_scatterSeries;
    if ( m_plotWidget ) return dynamic_cast<QtCharts::QScatterSeries*>( m_plotWidget->getScatterSeries( this ) );

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
        }
    }
    else
    {
        m_symbol.reset();
        if ( scatterSeries() ) scatterSeries()->hide();
    }
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
