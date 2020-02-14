/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RiuGroupedBarChartBuilder.h"

#include "RiaColorTables.h"

#include "qwt_column_symbol.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_plot.h"
#include "qwt_plot_barchart.h"
#include "qwt_scale_draw.h"

#include <limits>
#include <map>

//--------------------------------------------------------------------------------------------------
/// Overriding to avoid one-pixel overlap of bars.
//--------------------------------------------------------------------------------------------------
class RiuAvoidPixelOverlapColumnSymbol : public QwtColumnSymbol
{
public:
    RiuAvoidPixelOverlapColumnSymbol( Style style )
        : QwtColumnSymbol( style )
    {
    }

    void draw( QPainter* painter, const QwtColumnRect& rect ) const override
    {
        painter->save();

        switch ( this->style() )
        {
            case QwtColumnSymbol::Box:
            {
                switch ( this->frameStyle() )
                {
                    case QwtColumnSymbol::NoFrame:
                    {
                        QRectF r = rect.toRect();
                        if ( QwtPainter::roundingAlignment( painter ) )
                        {
                            r.setLeft( qRound( r.left() ) );
                            r.setRight( qRound( r.right() ) );
                            r.setTop( qRound( r.top() ) );
                            r.setBottom( qRound( r.bottom() ) );
                        }

                        painter->fillRect( r,
                                           this->palette()
                                               .window() ); // This line here is the difference. Qwt adds a 1 to width and height.
                    }
                    break;
                    default:
                        QwtColumnSymbol::drawBox( painter, rect );
                }
                break;
            }
            default:;
        }

        painter->restore();
    }
};

//--------------------------------------------------------------------------------------------------
/// Overridden ScaleDraw to add labels for med and min ticks, and to add newlines to get the
/// tick texts on different height according to tick level
//--------------------------------------------------------------------------------------------------
class RiuBarChartScaleDraw : public QwtScaleDraw
{
public:
    RiuBarChartScaleDraw( const std::map<double, std::pair<QwtScaleDiv::TickType, QString>>& posTickTypeAndTexts )
        : m_posTickTypeAndTexts( posTickTypeAndTexts )
    {
    }

    /// Override to add new lines to the labels according to the tick level

    QwtText label( double v ) const override
    {
        auto posTypeTextPairIt = m_posTickTypeAndTexts.find( v );
        if ( posTypeTextPairIt != m_posTickTypeAndTexts.end() )
        {
            if ( this->alignment() == BottomScale )
            {
                if ( posTypeTextPairIt->second.first == QwtScaleDiv::MediumTick )
                {
                    return "\n" + posTypeTextPairIt->second.second;
                }
                else if ( posTypeTextPairIt->second.first == QwtScaleDiv::MajorTick )
                {
                    return "\n\n" + posTypeTextPairIt->second.second;
                }
                else
                {
                    return posTypeTextPairIt->second.second;
                }
            }
            else if ( this->alignment() == LeftScale )
            {
                if ( posTypeTextPairIt->second.first == QwtScaleDiv::MediumTick )
                {
                    return posTypeTextPairIt->second.second + "      ";
                }
                else if ( posTypeTextPairIt->second.first == QwtScaleDiv::MajorTick )
                {
                    return posTypeTextPairIt->second.second + "                   ";
                }

                else
                {
                    return posTypeTextPairIt->second.second;
                }
            }
            else
            {
                return posTypeTextPairIt->second.second;
            }
        }
        else
        {
            return "X"; // Just for debugging
        }
    }

    // Override to draw text labels at medium and minor ticks also

    void draw( QPainter* painter, const QPalette& palette ) const override
    {
        QwtScaleDraw::draw( painter, palette );

        if ( hasComponent( QwtAbstractScaleDraw::Labels ) )
        {
            painter->save();
            painter->setPen( palette.color( QPalette::Text ) );

            const QList<double>& mediumTicks = scaleDiv().ticks( QwtScaleDiv::MediumTick );

            for ( int i = 0; i < mediumTicks.count(); i++ )
            {
                const double v = mediumTicks[i];
                if ( scaleDiv().contains( v ) ) drawLabel( painter, mediumTicks[i] );
            }

            const QList<double>& minorTicks = scaleDiv().ticks( QwtScaleDiv::MinorTick );

            for ( int i = 0; i < minorTicks.count(); i++ )
            {
                const double v = minorTicks[i];
                if ( scaleDiv().contains( v ) ) drawLabel( painter, minorTicks[i] );
            }

            painter->restore();
        }
    }

private:
    std::map<double, std::pair<QwtScaleDiv::TickType, QString>> m_posTickTypeAndTexts;
    bool                                                        m_hasMinorTickText;
    bool                                                        m_hasMediumTickText;

protected:
    virtual void drawBackbone( QPainter* ) const override {}
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupedBarChartBuilder::RiuGroupedBarChartBuilder( Qt::Orientation orientation )
    : m_orientation( orientation )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::addBarEntry( const QString& majorTickText,
                                             const QString& midTickText,
                                             const QString& minTickText,
                                             const double   sortValue,
                                             const QString& legendText,
                                             const double   value )
{
    m_sortedBarEntries.insert( BarEntry( majorTickText, midTickText, minTickText, sortValue, legendText, value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::addBarChartToPlot( QwtPlot* plot )
{
    const double majGroupSpacing = 1.6;
    const double midGroupSpacing = 0.5;
    const double minGroupSpacing = 0.2;

    std::set<QString> majTickTexts;
    std::set<QString> midTickTexts;
    std::set<QString> minTickTexts;

    double currentBarPosition      = 1.0;
    double currentMajGroupStartPos = 1.0;
    double currentMidGroupStartPos = 1.0;
    double currentMinGroupStartPos = 1.0;

    QString previousMajText;
    QString previousMidText;
    QString previousMinText;

    std::map<QString, QVector<QPointF>> legendToBarPointsMap;

    std::map<double, std::pair<QwtScaleDiv::TickType, QString>> positionedAxisTexts;

    QList<double> majTickPoss;
    QList<double> midTickPoss;
    QList<double> minTickPoss;

    // clang-format off
    auto addGroupTickText = [&]( double groupStartPos, QString tickText, QList<double>& groupTickPosList )
    {
        if( tickText.isEmpty() ) return;

        double tickPos = midPoint( groupStartPos, currentBarPosition );

        QwtScaleDiv::TickType ttyp = (&groupTickPosList == &majTickPoss ) ? QwtScaleDiv::MajorTick
                                                                          : ( &groupTickPosList == &midTickPoss ) ? QwtScaleDiv::MediumTick
                                                                                                                  : QwtScaleDiv::MinorTick;

        // Make sure we do not get ticks of different level exactly at the same spot, 
        // so that the drawing is able to distinguish

        if( ttyp == QwtScaleDiv::MinorTick ) tickPos += 2e-4;
        if( ttyp == QwtScaleDiv::MediumTick ) tickPos += 1e-4;

        positionedAxisTexts[tickPos] = { ttyp, tickText };

        groupTickPosList.push_back( tickPos );
    };
    // clang-format on

    for ( const BarEntry& barDef : m_sortedBarEntries )
    {
        bool hasAnyMajTics         = !majTickTexts.empty();
        auto majInsertResult       = majTickTexts.insert( barDef.m_majTickText );
        bool isStartingNewMajGroup = majInsertResult.second;
        bool isFinishingMajGroup   = isStartingNewMajGroup && hasAnyMajTics;

        if ( isFinishingMajGroup )
        {
            addGroupTickText( currentMajGroupStartPos, previousMajText, majTickPoss );
            addGroupTickText( currentMidGroupStartPos, previousMidText, midTickPoss );
            addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPoss );

            currentBarPosition += majGroupSpacing;
        }

        if ( isStartingNewMajGroup )
        {
            previousMajText = barDef.m_majTickText;
            previousMidText = "";
            previousMinText = "";

            midTickTexts.clear();
            minTickTexts.clear();

            currentMajGroupStartPos = currentBarPosition;
            currentMidGroupStartPos = currentBarPosition;
            currentMinGroupStartPos = currentBarPosition;
        }

        bool hasAnyMidTics         = !midTickTexts.empty();
        auto midInsertResult       = midTickTexts.insert( barDef.m_midTickText );
        bool isStartingNewMidGroup = midInsertResult.second;
        bool isFinishingMidGroup   = isStartingNewMidGroup && hasAnyMidTics;

        if ( isFinishingMidGroup )
        {
            addGroupTickText( currentMidGroupStartPos, previousMidText, midTickPoss );
            addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPoss );

            currentBarPosition += midGroupSpacing;
        }

        if ( isStartingNewMidGroup )
        {
            previousMidText = barDef.m_midTickText;
            previousMinText = "";

            minTickTexts.clear();

            currentMidGroupStartPos = currentBarPosition;
            currentMinGroupStartPos = currentBarPosition;
        }

        bool hasAnyMinTics         = !minTickTexts.empty();
        auto minInsertResult       = minTickTexts.insert( barDef.m_minTickText );
        bool isStartingNewMinGroup = minInsertResult.second;
        bool isFinishingMinGroup   = minInsertResult.second && hasAnyMinTics;

        if ( isFinishingMinGroup )
        {
            addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPoss );

            currentBarPosition += minGroupSpacing;
        }

        if ( isStartingNewMinGroup )
        {
            previousMinText         = barDef.m_minTickText;
            currentMinGroupStartPos = currentBarPosition;
        }

        // Insert bar value in correct set of colored bars

        auto legendToBarPointsPair = legendToBarPointsMap.find( barDef.m_legendText );

        QVector<QPointF>* barPoints = nullptr;

        if ( legendToBarPointsPair == legendToBarPointsMap.end() )
        {
            barPoints = &( legendToBarPointsMap[barDef.m_legendText] );
        }
        else
        {
            barPoints = &( legendToBarPointsPair->second );
        }

        barPoints->push_back( {currentBarPosition, barDef.m_value} );

        // Increment the bar position for the next bar
        currentBarPosition += 1.0;
    }

    // Add group tick texts for the last groups

    if ( !previousMajText.isEmpty() )
    {
        addGroupTickText( currentMajGroupStartPos, previousMajText, majTickPoss );
    }

    if ( !previousMidText.isEmpty() )
    {
        addGroupTickText( currentMidGroupStartPos, previousMidText, midTickPoss );
    }

    if ( !previousMinText.isEmpty() )
    {
        addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPoss );
    }

    int idx = 0;
    for ( const auto& legendToBarPointsPair : legendToBarPointsMap )
    {
        addQwtBarChart( plot,
                        legendToBarPointsPair.second,
                        legendToBarPointsPair.first,
                        RiaColorTables::summaryCurveDefaultPaletteColors().cycledQColor( idx ) );
        idx++;
    }

    QwtPlot::Axis         axis        = QwtPlot::xBottom;
    RiuBarChartScaleDraw* scaleDrawer = new RiuBarChartScaleDraw( positionedAxisTexts );

    if ( m_orientation == Qt::Horizontal )
    {
        axis = QwtPlot::yLeft;
    }

    plot->setAxisScaleDraw( axis, scaleDrawer );

    QwtScaleDiv scaleDiv( 0, currentBarPosition );

    if ( majTickPoss.size() ) scaleDiv.setTicks( QwtScaleDiv::MajorTick, majTickPoss );
    if ( midTickPoss.size() ) scaleDiv.setTicks( QwtScaleDiv::MediumTick, midTickPoss );
    if ( minTickPoss.size() ) scaleDiv.setTicks( QwtScaleDiv::MinorTick, minTickPoss );

    if ( m_orientation == Qt::Horizontal )
    {
        scaleDiv.invert();
    }

    plot->setAxisScaleDiv( axis, scaleDiv );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::addQwtBarChart( QwtPlot*                plot,
                                                const QVector<QPointF>& posAndValue,
                                                const QString&          legendText,
                                                const QColor&           barColor )
{
    QPalette palette;
    palette.setColor( QPalette::Window, barColor );
    palette.setColor( QPalette::Dark, barColor );

    RiuAvoidPixelOverlapColumnSymbol* barStyle = new RiuAvoidPixelOverlapColumnSymbol( QwtColumnSymbol::Box );
    barStyle->setPalette( palette );
    barStyle->setFrameStyle( QwtColumnSymbol::NoFrame );
    barStyle->setLineWidth( 0 );

    QwtPlotBarChart* barChart = new QwtPlotBarChart( legendText );
    barChart->setSamples( posAndValue );
    barChart->setLegendMode( QwtPlotBarChart::LegendChartTitle );
    barChart->setLayoutPolicy( QwtPlotAbstractBarChart::ScaleSamplesToAxes );
    barChart->setLayoutHint( 1.0 );
    barChart->setSymbol( barStyle );
    barChart->setOrientation( m_orientation );
    barChart->attach( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupedBarChartBuilder::BarEntry::BarEntry()
    : m_sortValue( std::numeric_limits<double>::infinity() )
    , m_value( std::numeric_limits<double>::infinity() )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupedBarChartBuilder::BarEntry::BarEntry( QString majorTickText,
                                               QString midTickText,
                                               QString minTickText,
                                               double  sortValue,
                                               QString legendText,
                                               double  value )
    : m_majTickText( majorTickText )
    , m_midTickText( midTickText )
    , m_minTickText( minTickText )
    , m_sortValue( sortValue )
    , m_legendText( legendText )
    , m_value( value )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGroupedBarChartBuilder::BarEntry::operator<( const BarEntry& other ) const
{
    if ( m_majTickText != other.m_majTickText ) return m_majTickText < other.m_majTickText;
    if ( m_midTickText != other.m_midTickText ) return m_midTickText < other.m_midTickText;
    if ( m_minTickText != other.m_minTickText ) return m_minTickText < other.m_minTickText;

    if ( m_sortValue != other.m_sortValue )
    {
        return m_sortValue > other.m_sortValue;
    }

    if ( m_legendText != other.m_legendText ) return m_legendText < other.m_legendText;

    return false;
}
