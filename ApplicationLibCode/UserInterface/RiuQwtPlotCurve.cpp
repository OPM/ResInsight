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

#include "RiuQwtPlotCurve.h"

#include "RiaCurveDataTools.h"
#include "RiaImageTools.h"

#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtSymbol.h"

#include "qwt_date.h"
#include "qwt_graphic.h"
#include "qwt_interval_symbol.h"
#include "qwt_painter.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_intervalcurve.h"
#include "qwt_point_mapper.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurve::RiuQwtPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title )
    : RiuPlotCurve( ownerRimCurve, title )
    , QwtPlotCurve( title )
    , m_showErrorBars( false )
{
    this->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    this->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );
    this->setLegendAttribute( QwtPlotCurve::LegendShowBrush, true );

    this->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    m_qwtCurveErrorBars = new QwtPlotIntervalCurve();
    m_qwtCurveErrorBars->setStyle( QwtPlotIntervalCurve::CurveStyle::NoCurve );
    m_qwtCurveErrorBars->setSymbol( new QwtIntervalSymbol( QwtIntervalSymbol::Bar ) );
    m_qwtCurveErrorBars->setItemAttribute( QwtPlotItem::Legend, false );
    m_qwtCurveErrorBars->setZ( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ERROR_BARS ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurve::~RiuQwtPlotCurve()
{
    detach();

    delete m_qwtCurveErrorBars;
    m_qwtCurveErrorBars = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setTitle( const QString& title )
{
    QwtPlotCurve::setTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::drawCurve( QPainter*          p,
                                 int                style,
                                 const QwtScaleMap& xMap,
                                 const QwtScaleMap& yMap,
                                 const QRectF&      canvasRect,
                                 int                from,
                                 int                to ) const
{
    size_t intervalCount = m_polyLineStartStopIndices.size();
    if ( intervalCount > 0 )
    {
        for ( const auto& [segmentFromCandiate, segmentToCandidate] : m_polyLineStartStopIndices )
        {
            // Skip segments outside the requested index range
            if ( static_cast<int>( segmentToCandidate ) < from ) continue;
            if ( static_cast<int>( segmentFromCandiate ) > to ) continue;

            // Draw the curve points limited to incoming from/to indices
            auto actualFromIndex = std::max( from, static_cast<int>( segmentFromCandiate ) );
            auto actualToIndex   = std::min( to, static_cast<int>( segmentToCandidate ) );

            if ( actualFromIndex == actualToIndex )
            {
                // Use a symbol to draw a single value, as a single value will not be visible
                // when using QwtPlotCurve::drawCurve without symbols activated

                QwtSymbol symbol( QwtSymbol::XCross );
                symbol.setSize( 10, 10 );

                QwtPlotCurve::drawSymbols( p, symbol, xMap, yMap, canvasRect, actualFromIndex, actualToIndex );
            }
            else
            {
                if ( actualFromIndex < actualToIndex )
                {
                    QwtPlotCurve::drawCurve( p, style, xMap, yMap, canvasRect, actualFromIndex, actualToIndex );
                }
            }
        }
    }
    else
    {
        QwtPlotCurve::drawCurve( p, style, xMap, yMap, canvasRect, from, to );
    }
};

//--------------------------------------------------------------------------------------------------
/// Drawing symbols but skipping if they are to close to the previous one
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::drawSymbols( QPainter*          painter,
                                   const QwtSymbol&   symbol,
                                   const QwtScaleMap& xMap,
                                   const QwtScaleMap& yMap,
                                   const QRectF&      canvasRect,
                                   int                from,
                                   int                to ) const
{
    QwtPointMapper mapper;
    bool           filterSymbols = m_symbolSkipPixelDistance > 0;

    if ( filterSymbols )
    {
        mapper.setFlag( QwtPointMapper::RoundPoints, QwtPainter::roundingAlignment( painter ) );
        mapper.setFlag( QwtPointMapper::WeedOutPoints, testPaintAttribute( QwtPlotCurve::FilterPoints ) );
        mapper.setBoundingRect( canvasRect );
    }

    const QPolygonF points     = mapper.toPointsF( xMap, yMap, data(), from, to );
    int             pointCount = points.size();
    QPolygonF       pointsToDisplay;

    if ( filterSymbols )
    {
        QPointF lastDrawnSymbolPos;

        if ( pointCount > 0 )
        {
            pointsToDisplay.push_back( points[0] );
            lastDrawnSymbolPos = points[0];
        }

        float sqSkipDist       = m_symbolSkipPixelDistance * m_symbolSkipPixelDistance;
        float sqSkipToLastDiff = m_symbolSkipPixelDistance / 10 * m_symbolSkipPixelDistance / 10;
        for ( int pIdx = 1; pIdx < pointCount - 1; ++pIdx )
        {
            QPointF diff                 = points[pIdx] - lastDrawnSymbolPos;
            float   sqDistBetweenSymbols = diff.x() * diff.x() + diff.y() * diff.y();

            if ( sqDistBetweenSymbols > sqSkipDist )
            {
                if ( pIdx == pointCount - 2 )
                {
                    QPointF diffToBack   = points.back() - points[pIdx];
                    float   sqDistToBack = diffToBack.x() * diffToBack.x() + diffToBack.y() * diffToBack.y();
                    if ( sqDistToBack < sqSkipToLastDiff ) continue;
                }
                pointsToDisplay.push_back( points[pIdx] );
                lastDrawnSymbolPos = points[pIdx];
            }
        }

        if ( pointCount > 1 ) pointsToDisplay.push_back( points.back() );
    }
    else
    {
        pointsToDisplay = points;
    }

    if ( pointsToDisplay.size() > 0 )
    {
        symbol.drawSymbols( painter, pointsToDisplay );

        const RiuQwtSymbol* sym = dynamic_cast<const RiuQwtSymbol*>( &symbol );

        if ( sym )
        {
            if ( m_perPointLabels.size() == static_cast<size_t>( pointsToDisplay.size() ) )
            {
                for ( int i = 0; i < pointsToDisplay.size(); ++i )
                {
                    sym->renderSymbolLabel( painter, pointsToDisplay[i], m_perPointLabels[i] );
                }
            }
            else if ( !sym->globalLabel().isEmpty() )
            {
                for ( auto& pt : pointsToDisplay )
                {
                    sym->renderSymbolLabel( painter, pt, sym->globalLabel() );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum          lineStyle,
                                     RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolationType,
                                     int                                            requestedCurveThickness,
                                     const QColor&                                  curveColor,
                                     const QBrush& fillBrush /* = QBrush( Qt::NoBrush )*/ )
{
    QwtPlotCurve::CurveStyle curveStyle = QwtPlotCurve::NoCurve;
    Qt::PenStyle             penStyle   = RiuQwtPlotCurveDefines::convertToPenStyle( lineStyle );

    // Qwt bug workaround (#4135): need to set 0 curve thickness for STYLE_NONE
    int curveThickness = 0;
    if ( lineStyle != RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE )
    {
        curveThickness = requestedCurveThickness;
        switch ( interpolationType )
        {
            case RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT:
                curveStyle = QwtPlotCurve::Steps;
                setCurveAttribute( QwtPlotCurve::Inverted, false );
                break;
            case RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT: // Fall through
            default:
                curveStyle = QwtPlotCurve::Lines;
                break;
        }
    }
    QPen curvePen( curveColor );
    curvePen.setWidth( curveThickness );
    curvePen.setStyle( penStyle );

    setPen( curvePen );
    setStyle( curveStyle );
    setBrush( fillBrush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setBrush( const QBrush& brush )
{
    QwtPlotCurve::setBrush( brush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtGraphic RiuQwtPlotCurve::legendIcon( int index, const QSizeF& size ) const
{
    QwtGraphic icon = QwtPlotCurve::legendIcon( index, size );
    if ( m_blackAndWhiteLegendIcon )
    {
        QImage image = icon.toImage();
        RiaImageTools::makeGrayScale( image );

        QPainter painter( &icon );
        painter.drawImage( QPoint( 0, 0 ), image );
    }
    return icon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPixmap RiuQwtPlotCurve::legendIcon( const QSizeF& size ) const
{
    return legendIcon( 0, size ).toPixmap();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setLegendIconSize( const QSize& iconSize )
{
    QwtPlotCurve::setLegendIconSize( iconSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlotCurve::legendIconSize() const
{
    return QwtPlotCurve::legendIconSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::attachToPlot( RiuPlotWidget* plotWidget )
{
    RiuQwtPlotWidget* qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidget );
    attach( qwtPlotWidget->qwtPlot() );

    if ( m_showErrorBars )
    {
        m_qwtCurveErrorBars->attach( qwtPlotWidget->qwtPlot() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::detach()
{
    QwtPlotCurve::detach();
    m_qwtCurveErrorBars->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::showInPlot()
{
    show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSamplesInPlot( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    CAF_ASSERT( xValues.size() == yValues.size() );

    setSamples( xValues.data(), yValues.data(), static_cast<int>( xValues.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setZ( int z )
{
    QwtPlotCurve::setZ( z );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::updateErrorBarsAppearance( bool showErrorBars, const QColor& curveColor )
{
    m_showErrorBars = showErrorBars;
    if ( m_qwtCurveErrorBars )
    {
        QwtIntervalSymbol* newSymbol = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
        newSymbol->setPen( QPen( curveColor ) );
        m_qwtCurveErrorBars->setSymbol( newSymbol );
        m_qwtCurveErrorBars->setVisible( showErrorBars );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::clearErrorBars()
{
    m_showErrorBars = false;

    m_qwtCurveErrorBars->setSamples( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotCurve::numSamples() const
{
    return static_cast<int>( dataSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQwtPlotCurve::sample( int index ) const
{
    CAF_ASSERT( index >= 0 && index <= numSamples() );
    auto p = QwtPlotCurve::sample( index );
    return std::make_pair( p.x(), p.y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQwtPlotCurve::xDataRange() const
{
    return std::make_pair( minXValue(), maxXValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQwtPlotCurve::yDataRange() const
{
    return std::make_pair( minYValue(), maxYValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSamplesFromXYErrorValues( const std::vector<double>&   xValues,
                                                   const std::vector<double>&   yValues,
                                                   const std::vector<double>&   errorValues,
                                                   bool                         useLogarithmicScale,
                                                   RiaCurveDataTools::ErrorAxis errorAxis )
{
    CVF_ASSERT( xValues.size() == yValues.size() );
    CVF_ASSERT( xValues.size() == errorValues.size() );

    auto intervalsOfValidValues = RiaCurveDataTools::calculateIntervalsOfValidValues( yValues, useLogarithmicScale );
    std::vector<double> filteredYValues;
    std::vector<double> filteredXValues;

    RiaCurveDataTools::getValuesByIntervals( yValues, intervalsOfValidValues, &filteredYValues );
    RiaCurveDataTools::getValuesByIntervals( xValues, intervalsOfValidValues, &filteredXValues );

    std::vector<double> filteredErrorValues;
    RiaCurveDataTools::getValuesByIntervals( errorValues, intervalsOfValidValues, &filteredErrorValues );

    QVector<QwtIntervalSample> errorIntervals;

    errorIntervals.reserve( static_cast<int>( filteredXValues.size() ) );

    for ( size_t i = 0; i < filteredXValues.size(); i++ )
    {
        if ( !std::isinf( filteredYValues[i] ) && !std::isinf( filteredErrorValues[i] ) )
        {
            if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
            {
                errorIntervals << QwtIntervalSample( filteredXValues[i],
                                                     filteredYValues[i] - filteredErrorValues[i],
                                                     filteredYValues[i] + filteredErrorValues[i] );
            }
            else
            {
                errorIntervals << QwtIntervalSample( filteredYValues[i],
                                                     filteredXValues[i] - filteredErrorValues[i],
                                                     filteredXValues[i] + filteredErrorValues[i] );
            }
        }
    }

    setSamplesInPlot( filteredXValues, filteredYValues );

    setLineSegmentStartStopIndices( intervalsOfValidValues );

    if ( m_qwtCurveErrorBars )
    {
        m_qwtCurveErrorBars->setSamples( errorIntervals );
        if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
        {
            m_qwtCurveErrorBars->setOrientation( Qt::Vertical );
        }
        else
        {
            m_qwtCurveErrorBars->setOrientation( Qt::Horizontal );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setXAxis( RiuPlotAxis axis )
{
    QwtPlotCurve::setXAxis( RiuQwtPlotTools::toQwtPlotAxis( axis ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setYAxis( RiuPlotAxis axis )
{
    QwtPlotCurve::setYAxis( RiuQwtPlotTools::toQwtPlotAxis( axis ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setVisibleInLegend( bool isVisibleInLegend )
{
    setItemAttribute( QwtPlotItem::Legend, isVisibleInLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSymbol( RiuPlotCurveSymbol* symbol )
{
    if ( symbol )
    {
        auto qwtSymbol = dynamic_cast<RiuQwtSymbol*>( symbol );
        CAF_ASSERT( qwtSymbol );
        QwtPlotCurve::setSymbol( qwtSymbol );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol* RiuQwtPlotCurve::createSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbol ) const
{
    return new RiuQwtSymbol( symbol );
}
