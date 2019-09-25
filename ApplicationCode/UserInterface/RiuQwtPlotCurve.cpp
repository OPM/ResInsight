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
#include "RiuQwtSymbol.h"

#include "qwt_date.h"
#include "qwt_interval_symbol.h"
#include "qwt_painter.h"
#include "qwt_plot_intervalcurve.h"
#include "qwt_point_mapper.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurve::RiuQwtPlotCurve( const QString& title )
    : QwtPlotCurve( title )
{
    this->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    this->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );
    this->setLegendAttribute( QwtPlotCurve::LegendShowBrush, true );

    this->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    m_symbolSkipPixelDistance = 10.0f;

    m_errorBars = new QwtPlotIntervalCurve();
    m_errorBars->setStyle( QwtPlotIntervalCurve::CurveStyle::NoCurve );
    m_errorBars->setSymbol( new QwtIntervalSymbol( QwtIntervalSymbol::Bar ) );
    m_errorBars->setItemAttribute( QwtPlotItem::Legend, false );
    m_errorBars->setZ( Z_ERROR_BARS );

    m_showErrorBars           = true;
    m_attachedToPlot          = nullptr;
    m_blackAndWhiteLegendIcon = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurve::~RiuQwtPlotCurve() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSamplesFromXValuesAndYValues( const std::vector<double>& xValues,
                                                       const std::vector<double>& yValues,
                                                       const std::vector<double>& errorValues,
                                                       bool                       keepOnlyPositiveValues,
                                                       ErrorAxis                  errorAxis )
{
    CVF_ASSERT( xValues.size() == yValues.size() );
    CVF_ASSERT( yErrorValues.empty() || yErrorValues.size() == xValues.size() );

    bool                                   showErrorBars = m_showErrorBars && !errorValues.empty();
    QPolygonF                              points;
    QVector<QwtIntervalSample>             errorIntervals;
    std::vector<std::pair<size_t, size_t>> filteredIntervals;
    {
        std::vector<double> filteredYValues;
        std::vector<double> filteredXValues;
        std::vector<double> filteredErrorValues;

        {
            auto intervalsOfValidValues = RiaCurveDataTools::calculateIntervalsOfValidValues( yValues,
                                                                                              keepOnlyPositiveValues );

            RiaCurveDataTools::getValuesByIntervals( yValues, intervalsOfValidValues, &filteredYValues );
            RiaCurveDataTools::getValuesByIntervals( xValues, intervalsOfValidValues, &filteredXValues );

            if ( showErrorBars )
                RiaCurveDataTools::getValuesByIntervals( errorValues, intervalsOfValidValues, &filteredErrorValues );

            filteredIntervals = RiaCurveDataTools::computePolyLineStartStopIndices( intervalsOfValidValues );
        }

        points.reserve( static_cast<int>( filteredXValues.size() ) );
        errorIntervals.reserve( static_cast<int>( filteredXValues.size() ) );
        for ( size_t i = 0; i < filteredXValues.size(); i++ )
        {
            points << QPointF( filteredXValues[i], filteredYValues[i] );

            if ( showErrorBars && filteredYValues[i] != DOUBLE_INF && filteredErrorValues[i] != DOUBLE_INF )
            {
                if ( errorAxis == ERROR_ALONG_Y_AXIS )
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
    }

    this->setSamples( points );
    this->setLineSegmentStartStopIndices( filteredIntervals );

    if ( showErrorBars )
    {
        m_errorBars->setSamples( errorIntervals );
        if ( errorAxis == ERROR_ALONG_Y_AXIS )
        {
            m_errorBars->setOrientation( Qt::Vertical );
        }
        else
        {
            m_errorBars->setOrientation( Qt::Horizontal );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSamplesFromXValuesAndYValues( const std::vector<double>& xValues,
                                                       const std::vector<double>& yValues,
                                                       bool                       keepOnlyPositiveValues )
{
    setSamplesFromXValuesAndYValues( xValues, yValues, std::vector<double>(), keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                                     const std::vector<double>&    yValues,
                                                     bool                          keepOnlyPositiveValues )
{
    setSamplesFromXValuesAndYValues( RiuQwtPlotCurve::fromQDateTime( dateTimes ),
                                     yValues,
                                     std::vector<double>(),
                                     keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                                     const std::vector<double>& yValues,
                                                     bool                       keepOnlyPositiveValues )
{
    setSamplesFromXValuesAndYValues( RiuQwtPlotCurve::fromTime_t( dateTimes ),
                                     yValues,
                                     std::vector<double>(),
                                     keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                                     const std::vector<double>& yValues,
                                                     const std::vector<double>& yErrorValues,
                                                     bool                       keepOnlyPositiveValues )
{
    setSamplesFromXValuesAndYValues( RiuQwtPlotCurve::fromTime_t( dateTimes ),
                                     yValues,
                                     yErrorValues,
                                     keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::drawCurve(
    QPainter* p, int style, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to ) const
{
    size_t intervalCount = m_polyLineStartStopIndices.size();
    if ( intervalCount > 0 )
    {
        for ( size_t intIdx = 0; intIdx < intervalCount; intIdx++ )
        {
            if ( m_polyLineStartStopIndices[intIdx].first == m_polyLineStartStopIndices[intIdx].second )
            {
                // Use a symbol to draw a single value, as a single value will not be visible
                // when using QwtPlotCurve::drawCurve without symbols activated

                QwtSymbol symbol( QwtSymbol::XCross );
                symbol.setSize( 10, 10 );

                QwtPlotCurve::drawSymbols( p,
                                           symbol,
                                           xMap,
                                           yMap,
                                           canvasRect,
                                           (int)m_polyLineStartStopIndices[intIdx].first,
                                           (int)m_polyLineStartStopIndices[intIdx].second );
            }
            else
            {
                QwtPlotCurve::drawCurve( p,
                                         style,
                                         xMap,
                                         yMap,
                                         canvasRect,
                                         (int)m_polyLineStartStopIndices[intIdx].first,
                                         (int)m_polyLineStartStopIndices[intIdx].second );
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
            if ( m_perPointLabels.size() == pointsToDisplay.size() )
            {
                for ( int i = 0; i < (int)pointsToDisplay.size(); ++i )
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
void RiuQwtPlotCurve::setLineSegmentStartStopIndices(
    const std::vector<std::pair<size_t, size_t>>& lineSegmentStartStopIndices )
{
    m_polyLineStartStopIndices = lineSegmentStartStopIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setSymbolSkipPixelDistance( float distance )
{
    m_symbolSkipPixelDistance = distance >= 0.0f ? distance : 0.0f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::attach( QwtPlot* plot )
{
    QwtPlotItem::attach( plot );
    if ( m_showErrorBars )
    {
        m_errorBars->attach( plot );
    }
    m_attachedToPlot = plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::detach()
{
    QwtPlotItem::detach();
    m_errorBars->detach();
    m_attachedToPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::clearErrorBars()
{
    m_errorBars->setSamples( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::showErrorBars( bool show )
{
    m_showErrorBars = show;
    if ( m_showErrorBars && m_attachedToPlot )
        m_errorBars->attach( m_attachedToPlot );
    else
        m_errorBars->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setErrorBarsColor( QColor color )
{
    QwtIntervalSymbol* newSymbol = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
    newSymbol->setPen( QPen( color ) );
    m_errorBars->setSymbol( newSymbol );
}

//--------------------------------------------------------------------------------------------------
/// Set the Qwt X-Axis (QwtPlot::xBottom or QwtPlot::xTop).
/// This is important to make sure the x-axis interval is set correctly.
/// WellLogPlots use top-axis and Summary uses bottom axis.
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setErrorBarsXAxis( int axis )
{
    m_errorBars->setXAxis( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setPerPointLabels( const std::vector<QString>& labels )
{
    m_perPointLabels = labels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setAppearance( LineStyleEnum          lineStyle,
                                     CurveInterpolationEnum interpolationType,
                                     int                    requestedCurveThickness,
                                     const QColor&          curveColor )
{
    QwtPlotCurve::CurveStyle curveStyle = QwtPlotCurve::NoCurve;
    Qt::PenStyle             penStyle   = Qt::NoPen;

    // Qwt bug workaround (#4135): need to set 0 curve thickness for STYLE_NONE
    int curveThickness = 0;
    if ( lineStyle != STYLE_NONE )
    {
        curveThickness = requestedCurveThickness;
        switch ( interpolationType )
        {
            case INTERPOLATION_STEP_LEFT:
                curveStyle = QwtPlotCurve::Steps;
                setCurveAttribute( QwtPlotCurve::Inverted, false );
                break;
            case INTERPOLATION_POINT_TO_POINT: // Fall through
            default:
                curveStyle = QwtPlotCurve::Lines;
                break;
        }

        switch ( lineStyle )
        {
            case STYLE_SOLID:
                penStyle = Qt::SolidLine;
                break;
            case STYLE_DASH:
                penStyle = Qt::DashLine;
                break;
            case STYLE_DOT:
                penStyle = Qt::DotLine;
                break;
            case STYLE_DASH_DOT:
                penStyle = Qt::DashDotLine;
                break;

            default:
                break;
        }
    }
    QPen curvePen( curveColor );
    curvePen.setWidth( curveThickness );
    curvePen.setStyle( penStyle );

    setPen( curvePen );
    setStyle( curveStyle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotCurve::setBlackAndWhiteLegendIcon( bool blackAndWhite )
{
    m_blackAndWhiteLegendIcon = blackAndWhite;
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
std::vector<double> RiuQwtPlotCurve::fromQDateTime( const std::vector<QDateTime>& dateTimes )
{
    std::vector<double> doubleValues;

    if ( !dateTimes.empty() )
    {
        doubleValues.reserve( dateTimes.size() );

        for ( const auto& dt : dateTimes )
        {
            doubleValues.push_back( QwtDate::toDouble( dt ) );
        }
    }

    return doubleValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiuQwtPlotCurve::fromTime_t( const std::vector<time_t>& timeSteps )
{
    std::vector<double> doubleValues;

    if ( !timeSteps.empty() )
    {
        doubleValues.reserve( timeSteps.size() );
        for ( const auto& time : timeSteps )
        {
            double milliSecSinceEpoch = time *
                                        1000; // This is kind of hack, as the c++ standard does not state what time_t
                                              // is. "Almost always" secs since epoch according to cppreference.com

            doubleValues.push_back( milliSecSinceEpoch );
        }
    }

    return doubleValues;
}
