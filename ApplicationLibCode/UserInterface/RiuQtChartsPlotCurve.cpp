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

#include "RiaCurveDataTools.h"
#include "RiaImageTools.h"
#include "RiaPlotDefines.h"
#include "RiaTimeTTools.h"

#include "RiuQtChartsPlotWidget.h"
#include "RiuQwtSymbol.h"

#include "cvfBoundingBox.h"

#include <QLegendMarker>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotCurve::RiuQtChartsPlotCurve( const QString& title )
    : RiuPlotCurve()
{
    m_plotWidget = nullptr;

    m_lineSeries = new QtCharts::QLineSeries();
    m_lineSeries->setName( title );

    m_scatterSeries = new QtCharts::QScatterSeries();
    m_scatterSeries->setName( title );
    m_scatterSeries->setMarkerShape( QtCharts::QScatterSeries::MarkerShapeRectangle );
    m_scatterSeries->setMarkerSize( 20.0 );

    m_axisX = RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
    m_axisY = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
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
    lineSeries()->setName( title );
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
void RiuQtChartsPlotCurve::setSymbolAppearance( RiuQwtSymbol::PointSymbolEnum symbol, int size, const QColor& color )
{
    //    lineSeries()->setPointsVisible();
    if ( symbol == RiuQwtSymbol::PointSymbolEnum::SYMBOL_NONE )
        scatterSeries()->hide();
    else
    {
        if ( symbol == RiuQwtSymbol::PointSymbolEnum::SYMBOL_RECT )
            scatterSeries()->setMarkerShape( QtCharts::QScatterSeries::MarkerShapeRectangle );
        else if ( symbol == RiuQwtSymbol::PointSymbolEnum::SYMBOL_ELLIPSE )
            scatterSeries()->setMarkerShape( QtCharts::QScatterSeries::MarkerShapeCircle );

        scatterSeries()->setMarkerSize( size );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setBrush( const QBrush& brush )
{
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
        lineSeries()->show();
        scatterSeries()->show();
    }
    else
    {
        m_plotWidget->attach( this, lineSeries(), scatterSeries(), m_axisX, m_axisY );
        // Plot widget takes ownership.
        m_lineSeries    = nullptr;
        m_scatterSeries = nullptr;
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

    QtCharts::QScatterSeries* scatter = scatterSeries();
    if ( scatter )
    {
        scatter->hide();
    }

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
void RiuQtChartsPlotCurve::setXAxis( RiaDefines::PlotAxis axis )
{
    m_axisX = axis;
    if ( m_plotWidget ) m_plotWidget->setXAxis( axis, lineSeries() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setYAxis( RiaDefines::PlotAxis axis )
{
    m_axisY = axis;
    if ( m_plotWidget ) m_plotWidget->setYAxis( axis, lineSeries() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotCurve::numSamples() const
{
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
    CAF_ASSERT( m_plotWidget );
    CAF_ASSERT( m_plotWidget->qtChart() );
    CAF_ASSERT( m_plotWidget->qtChart()->legend() );
    if ( lineSeries() )
    {
        auto markers = m_plotWidget->qtChart()->legend()->markers( lineSeries() );
        if ( !markers.isEmpty() ) markers[0]->setVisible( isVisibleInLegend );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QtCharts::QLineSeries* RiuQtChartsPlotCurve::lineSeries() const
{
    if ( m_lineSeries )
        return m_lineSeries;
    else if ( m_plotWidget )
        return dynamic_cast<QtCharts::QLineSeries*>( m_plotWidget->getLineSeries( this ) );
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QtCharts::QScatterSeries* RiuQtChartsPlotCurve::scatterSeries() const
{
    if ( m_scatterSeries )
        return m_scatterSeries;
    else if ( m_plotWidget )
        return dynamic_cast<QtCharts::QScatterSeries*>( m_plotWidget->getScatterSeries( this ) );
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setSymbol( RiuPlotCurveSymbol* symbol )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol* RiuQtChartsPlotCurve::createSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbol ) const
{
    return nullptr;
}
