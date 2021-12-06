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
    // QwtPlotCurve::CurveStyle curveStyle = QwtPlotCurve::NoCurve;
    Qt::PenStyle penStyle = Qt::NoPen;

    // Qwt bug workaround (#4135): need to set 0 curve thickness for STYLE_NONE
    int curveThickness = 0;
    if ( lineStyle != RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE )
    {
        curveThickness = requestedCurveThickness;
        // switch ( interpolationType )
        // {
        //     case RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT:
        //         curveStyle = QwtPlotCurve::Steps;
        //         setCurveAttribute( QwtPlotCurve::Inverted, false );
        //         break;
        //     case RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT: // Fall through
        //     default:
        //         curveStyle = QwtPlotCurve::Lines;
        //         break;
        // }

        switch ( lineStyle )
        {
            case RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID:
                penStyle = Qt::SolidLine;
                break;
            case RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH:
                penStyle = Qt::DashLine;
                break;
            case RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DOT:
                penStyle = Qt::DotLine;
                break;
            case RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH_DOT:
                penStyle = Qt::DashDotLine;
                break;

            default:
                break;
        }
    }
    QPen curvePen( curveColor );
    curvePen.setWidth( curveThickness );
    curvePen.setStyle( penStyle );

    lineSeries()->setPen( curvePen );
    // setStyle( curveStyle );
    // setBrush( fillBrush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setSymbolAppearance( RiuQwtSymbol::PointSymbolEnum, int size, const QColor& color )
{
    lineSeries()->setPointsVisible();
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

    if ( m_plotWidget->getSeries( this ) )
    {
        lineSeries()->show();
    }
    else
    {
        m_plotWidget->attach( this, lineSeries(), m_axisX, m_axisY );
        // Plot widget takes ownership.
        m_lineSeries = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::detach()
{
    // TODO: not sure about this one..
    if ( lineSeries() )
    {
        lineSeries()->hide();
    }
    m_plotWidget = nullptr;

    // if ( m_lineSeries && m_lineSeries->chart() )
    // {
    //     m_lineSeries->hide();
    //     // m_lineSeries->chart()->removeSeries( m_lineSeries );
    //     // m_lineSeries = nullptr;
    // }
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

    lineSeries()->clear();
    for ( int i = 0; i < numValues; i++ )
    {
        lineSeries()->append( xValues[i], yValues[i] );
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
        return dynamic_cast<QtCharts::QLineSeries*>( m_plotWidget->getSeries( this ) );
    else
        return nullptr;
}
