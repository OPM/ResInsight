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
    m_lineSeries = new QtCharts::QLineSeries();
    m_lineSeries->setName( title );
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

// void RiuQtChartsPlotCurve::drawCurve( QPainter*          p,
//                                  int                style,
//                                  const QwtScaleMap& xMap,
//                                  const QwtScaleMap& yMap,
//                                  const QRectF&      canvasRect,
//                                  int                from,
//                                  int                to ) const
// {
//     size_t intervalCount = m_polyLineStartStopIndices.size();
//     if ( intervalCount > 0 )
//     {
//         for ( size_t intIdx = 0; intIdx < intervalCount; intIdx++ )
//         {
//             if ( m_polyLineStartStopIndices[intIdx].first == m_polyLineStartStopIndices[intIdx].second )
//             {
//                 // Use a symbol to draw a single value, as a single value will not be visible
//                 // when using QtChartsPlotCurve::drawCurve without symbols activated

//                 QwtSymbol symbol( QwtSymbol::XCross );
//                 symbol.setSize( 10, 10 );

//                 QtChartsPlotCurve::drawSymbols( p,
//                                            symbol,
//                                            xMap,
//                                            yMap,
//                                            canvasRect,
//                                            (int)m_polyLineStartStopIndices[intIdx].first,
//                                            (int)m_polyLineStartStopIndices[intIdx].second );
//             }
//             else
//             {
//                 QtChartsPlotCurve::drawCurve( p,
//                                          style,
//                                          xMap,
//                                          yMap,
//                                          canvasRect,
//                                          (int)m_polyLineStartStopIndices[intIdx].first,
//                                          (int)m_polyLineStartStopIndices[intIdx].second );
//             }
//         }
//     }
//     else
//     {
//         QtChartsPlotCurve::drawCurve( p, style, xMap, yMap, canvasRect, from, to );
//     }
// };

//--------------------------------------------------------------------------------------------------
/// Drawing symbols but skipping if they are to close to the previous one
//--------------------------------------------------------------------------------------------------
// void RiuQtChartsPlotCurve::drawSymbols( QPainter*          painter,
//                                    const QwtSymbol&   symbol,
//                                    const QwtScaleMap& xMap,
//                                    const QwtScaleMap& yMap,
//                                    const QRectF&      canvasRect,
//                                    int                from,
//                                    int                to ) const
// {
//     QwtPointMapper mapper;
//     bool           filterSymbols = m_symbolSkipPixelDistance > 0;

//     if ( filterSymbols )
//     {
//         mapper.setFlag( QwtPointMapper::RoundPoints, QwtPainter::roundingAlignment( painter ) );
//         mapper.setFlag( QwtPointMapper::WeedOutPoints, testPaintAttribute( QtChartsPlotCurve::FilterPoints ) );
//         mapper.setBoundingRect( canvasRect );
//     }

//     const QPolygonF points     = mapper.toPointsF( xMap, yMap, data(), from, to );
//     int             pointCount = points.size();
//     QPolygonF       pointsToDisplay;

//     if ( filterSymbols )
//     {
//         QPointF lastDrawnSymbolPos;

//         if ( pointCount > 0 )
//         {
//             pointsToDisplay.push_back( points[0] );
//             lastDrawnSymbolPos = points[0];
//         }

//         float sqSkipDist       = m_symbolSkipPixelDistance * m_symbolSkipPixelDistance;
//         float sqSkipToLastDiff = m_symbolSkipPixelDistance / 10 * m_symbolSkipPixelDistance / 10;
//         for ( int pIdx = 1; pIdx < pointCount - 1; ++pIdx )
//         {
//             QPointF diff                 = points[pIdx] - lastDrawnSymbolPos;
//             float   sqDistBetweenSymbols = diff.x() * diff.x() + diff.y() * diff.y();

//             if ( sqDistBetweenSymbols > sqSkipDist )
//             {
//                 if ( pIdx == pointCount - 2 )
//                 {
//                     QPointF diffToBack   = points.back() - points[pIdx];
//                     float   sqDistToBack = diffToBack.x() * diffToBack.x() + diffToBack.y() * diffToBack.y();
//                     if ( sqDistToBack < sqSkipToLastDiff ) continue;
//                 }
//                 pointsToDisplay.push_back( points[pIdx] );
//                 lastDrawnSymbolPos = points[pIdx];
//             }
//         }

//         if ( pointCount > 1 ) pointsToDisplay.push_back( points.back() );
//     }
//     else
//     {
//         pointsToDisplay = points;
//     }

//     if ( pointsToDisplay.size() > 0 )
//     {
//         symbol.drawSymbols( painter, pointsToDisplay );

//         const RiuQwtSymbol* sym = dynamic_cast<const RiuQwtSymbol*>( &symbol );

//         if ( sym )
//         {
//             if ( m_perPointLabels.size() == static_cast<size_t>( pointsToDisplay.size() ) )
//             {
//                 for ( int i = 0; i < pointsToDisplay.size(); ++i )
//                 {
//                     sym->renderSymbolLabel( painter, pointsToDisplay[i], m_perPointLabels[i] );
//                 }
//             }
//             else if ( !sym->globalLabel().isEmpty() )
//             {
//                 for ( auto& pt : pointsToDisplay )
//                 {
//                     sym->renderSymbolLabel( painter, pt, sym->globalLabel() );
//                 }
//             }
//         }
//     }
// }

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

#include <cmath>
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
    RiuQtChartsPlotWidget* qtChartsPlotWidget = dynamic_cast<RiuQtChartsPlotWidget*>( plotWidget );

    qtChartsPlotWidget->attach( m_lineSeries );
    // qtChart()->addSeries( m_lineSeries );
    // qtChartsPlotWidget->qtChart()->setAxisX(axisX, series);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurve::detach()
{
    // TODO: not sure about this one..
    m_lineSeries->hide();
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
