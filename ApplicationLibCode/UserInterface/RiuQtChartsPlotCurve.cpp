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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setTitle( const QString& title )
{
    m_lineSeries->setName( title );
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

    m_lineSeries->setPen( curvePen );
    // setStyle( curveStyle );
    // setBrush( fillBrush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setSymbolAppearance( RiuQwtSymbol::PointSymbolEnum, int size, const QColor& color )
{
    m_lineSeries->setPointsVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setBrush( const QBrush& brush )
{
    m_lineSeries->setBrush( brush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::attachToPlot( RiuPlotWidget* plotWidget )
{
    m_plotWidget = dynamic_cast<RiuQtChartsPlotWidget*>( plotWidget );
    CAF_ASSERT( m_plotWidget );
    m_plotWidget->attach( m_lineSeries, m_axisX, m_axisY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::detach()
{
    // TODO: not sure about this one..
    m_plotWidget = nullptr;
    m_lineSeries->hide();
    m_lineSeries->chart()->removeSeries( m_lineSeries );
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

    for ( int i = 0; i < numValues; i++ )
    {
        m_lineSeries->append( xValues[i], yValues[i] );
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
    if ( m_plotWidget ) m_plotWidget->setXAxis( axis, m_lineSeries );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::setYAxis( RiaDefines::PlotAxis axis )
{
    m_axisY = axis;
    if ( m_plotWidget ) m_plotWidget->setYAxis( axis, m_lineSeries );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotCurve::numSamples() const
{
    return m_lineSeries->count();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQtChartsPlotCurve::sample( int index ) const
{
    CAF_ASSERT( index >= 0 && index <= numSamples() );
    auto p = m_lineSeries->at( index );
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
    auto points = m_lineSeries->pointsVector();

    cvf::BoundingBox bb;
    for ( auto p : points )
        bb.add( cvf::Vec3d( p.x(), p.y(), 0.0 ) );

    return bb;
}
