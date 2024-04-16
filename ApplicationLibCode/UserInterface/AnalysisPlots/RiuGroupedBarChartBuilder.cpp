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
#include "RiaPreferences.h"

#include "RifCsvDataTableFormatter.h"

#include "RiuGuiTheme.h"

#include "cafFontTools.h"

#include "qwt_column_symbol.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_plot.h"
#include "qwt_plot_barchart.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_scaleitem.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"

#include <QColor>
#include <QPainter>

#include <cmath>
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

        switch ( style() )
        {
            case QwtColumnSymbol::Box:
            {
                switch ( frameStyle() )
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
                                           palette().window() ); // This line here is the difference. Qwt adds a 1
                                                                 // to width and height.
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
    struct RiuBarChartTick
    {
        RiuBarChartTick() = default;
        RiuBarChartTick( QwtScaleDiv::TickType tickType, const QString& label, bool oppositeSide = false )
            : tickType( tickType )
            , label( label )
            , oppositeSide( oppositeSide )
        {
        }

        QwtScaleDiv::TickType tickType;
        QString               label;
        bool                  oppositeSide;
    };

public:
    RiuBarChartScaleDraw( const std::map<double, RiuBarChartTick>& posTickTypeAndTexts,
                          int                                      labelFontPointSize,
                          const QColor&                            textColor = QColor( Qt::white ) )
        : m_posTickTypeAndTexts( posTickTypeAndTexts )
        , m_labelFontPointSize( labelFontPointSize )
        , m_textColor( textColor )
    {
        setTickLength( QwtScaleDiv::MajorTick, 0 );
        setTickLength( QwtScaleDiv::MediumTick, 0 );
        setTickLength( QwtScaleDiv::MinorTick, 0 );

        int minorTextLineCount  = 0;
        int mediumTextLineCount = 0;
        int minTickMaxTextSize  = 0;
        int medTickMaxTextSize  = 0;

        for ( auto& posTickTypeText : m_posTickTypeAndTexts )
        {
            if ( posTickTypeText.second.tickType == QwtScaleDiv::MediumTick )
            {
                int lineCount       = 1 + posTickTypeText.second.label.count( "\n" );
                mediumTextLineCount = std::max( mediumTextLineCount, lineCount );
                medTickMaxTextSize  = std::max( (int)posTickTypeText.second.label.size(), medTickMaxTextSize );
            }
            else if ( posTickTypeText.second.tickType == QwtScaleDiv::MinorTick )
            {
                int lineCount      = 1 + posTickTypeText.second.label.count( "\n" );
                minorTextLineCount = std::max( minorTextLineCount, lineCount );
                minTickMaxTextSize = std::max( (int)posTickTypeText.second.label.size(), minTickMaxTextSize );
            }
        }

        m_medLineBreak = "";
        for ( int i = 0; i < minorTextLineCount; ++i )
        {
            m_medLineBreak += "\n";
        }

        m_majLineBreak = m_medLineBreak;
        for ( int i = 0; i < mediumTextLineCount; ++i )
        {
            m_majLineBreak += "\n";
        }

        m_medSpacing.fill( ' ', 2 * minTickMaxTextSize );
        m_majSpacing = m_medSpacing + QString().fill( ' ', 2 * medTickMaxTextSize );
    }

    QwtText createLabelFromString( const QString& string, int fontSizeChange = 0 ) const
    {
        QwtText text( string );

        QFont font = text.font();
        font.setPixelSize( caf::FontTools::pointSizeToPixelSize( m_labelFontPointSize + fontSizeChange ) );
        text.setFont( font );

        text.setPaintAttribute( QwtText::PaintUsingTextFont, true );
        text.setPaintAttribute( QwtText::PaintUsingTextColor, true );
        text.setColor( m_textColor );

        return text;
    }
    /// Override to add new lines to the labels according to the tick level

    QwtText label( double v ) const override
    {
        auto posTickIt = m_posTickTypeAndTexts.find( v );
        if ( posTickIt != m_posTickTypeAndTexts.end() )
        {
            if ( alignment() == BottomScale )
            {
                if ( posTickIt->second.tickType == QwtScaleDiv::MediumTick )
                {
                    return createLabelFromString( m_medLineBreak + posTickIt->second.label, -1 );
                }
                else if ( posTickIt->second.tickType == QwtScaleDiv::MajorTick )
                {
                    return createLabelFromString( m_majLineBreak + posTickIt->second.label, 0 );
                }
                else
                {
                    return createLabelFromString( posTickIt->second.label, -2 );
                }
            }
            else if ( alignment() == LeftScale )
            {
                if ( posTickIt->second.tickType == QwtScaleDiv::MediumTick )
                {
                    return createLabelFromString( posTickIt->second.label + m_medSpacing, -1 );
                }
                else if ( posTickIt->second.tickType == QwtScaleDiv::MajorTick )
                {
                    return createLabelFromString( posTickIt->second.label + m_majSpacing, 0 );
                }

                else
                {
                    return createLabelFromString( posTickIt->second.label, -2 );
                }
            }
            else
            {
                return createLabelFromString( posTickIt->second.label );
            }
        }
        else
        {
            return QwtText( QString( "X" ) ); // Just for debugging
        }
    }

    QPoint translateToOppositeSide( QPainter* painter, double value ) const
    {
        QwtText lbl           = tickLabel( painter->font(), value );
        QSizeF  labelSize     = lbl.textSize( painter->font() );
        QPointF localLabelPos = labelPosition( value ) - pos();

        if ( alignment() == RightScale )
        {
            return QPoint( -2 * localLabelPos.x() - labelSize.width(), 0 );
        }
        else if ( alignment() == LeftScale )
        {
            return QPoint( 2 * localLabelPos.x() + labelSize.width(), 0 );
        }
        else if ( alignment() == BottomScale )
        {
            return QPoint( 0, 2 * localLabelPos.y() - labelSize.width() );
        }
        if ( alignment() == TopScale )
        {
            return QPoint( 0, -2 * localLabelPos.y() + labelSize.width() );
        }

        return QPoint( 0, 0 );
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
                if ( scaleDiv().contains( v ) )
                {
                    auto it           = m_posTickTypeAndTexts.find( v );
                    bool oppositeSide = it != m_posTickTypeAndTexts.end() ? it->second.oppositeSide : false;
                    if ( oppositeSide )
                    {
                        painter->save();
                        painter->translate( translateToOppositeSide( painter, v ) );
                    }
                    drawLabel( painter, mediumTicks[i] );
                    if ( oppositeSide )
                    {
                        painter->restore();
                    }
                }
            }

            const QList<double>& minorTicks = scaleDiv().ticks( QwtScaleDiv::MinorTick );

            for ( int i = 0; i < minorTicks.count(); i++ )
            {
                const double v = minorTicks[i];
                if ( scaleDiv().contains( v ) )
                {
                    auto it           = m_posTickTypeAndTexts.find( v );
                    bool oppositeSide = it != m_posTickTypeAndTexts.end() ? it->second.oppositeSide : false;
                    if ( oppositeSide )
                    {
                        painter->save();
                        painter->translate( translateToOppositeSide( painter, v ) );
                    }

                    drawLabel( painter, minorTicks[i] );
                    if ( oppositeSide )
                    {
                        painter->restore();
                    }
                }
            }

            painter->restore();
        }
    }

protected:
    void drawBackbone( QPainter* ) const override {}

private:
    std::map<double, RiuBarChartTick> m_posTickTypeAndTexts;

    QString m_medLineBreak;
    QString m_majLineBreak;
    QString m_medSpacing;
    QString m_majSpacing;

    int m_labelFontPointSize;

    QColor m_textColor;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupedBarChartBuilder::RiuGroupedBarChartBuilder( bool sortGroupsByMaxValueInGroup )
    : m_isSortingByMaxValueInGroups( sortGroupsByMaxValueInGroup )
{
    m_labelPointSize =
        caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), caf::FontTools::RelativeSize::Medium );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::addBarEntry( const QString& majorTickText,
                                             const QString& midTickText,
                                             const QString& minTickText,
                                             const double   sortValue,
                                             const QString& legendText,
                                             const QString& barText,
                                             const double   value )
{
    m_sortedBarEntries.insert( BarEntry( majorTickText, midTickText, minTickText, sortValue, legendText, barText, value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::setBarColor( const QColor& color )
{
    m_useBarColor = true;
    m_barColor    = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::addBarChartToPlot( QwtPlot* plot, Qt::Orientation barOrientation, int maxBarCount )
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

    std::map<double, RiuBarChartScaleDraw::RiuBarChartTick> groupPositionedAxisTexts;
    std::map<double, RiuBarChartScaleDraw::RiuBarChartTick> positionedBarLabels;

    QList<double> majTickPositions;
    QList<double> midTickPositions;
    QList<double> minTickPositions;

    QList<double> majDividerPositions;
    QList<double> midDividerPositions;
    QList<double> minDividerPositions;

    // Filter the entries according to value
    std::multiset<BarEntry> filteredBarEntries;
    if ( maxBarCount >= 0 )
    {
        std::map<double, BarEntry> valueFilteredBarEntries;
        int                        mapSize = 0;

        for ( const BarEntry& barDef : m_sortedBarEntries )
        {
            if ( mapSize < maxBarCount )
            {
                if ( valueFilteredBarEntries.insert( std::make_pair( fabs( barDef.m_value ), barDef ) ).second )
                {
                    mapSize++;
                }
            }
            else if ( fabs( barDef.m_value ) > valueFilteredBarEntries.begin()->first )
            {
                if ( valueFilteredBarEntries.insert( std::make_pair( fabs( barDef.m_value ), barDef ) ).second )
                {
                    valueFilteredBarEntries.erase( valueFilteredBarEntries.begin() );
                }
            }
        }

        for ( auto valEntryPair : valueFilteredBarEntries )
        {
            filteredBarEntries.insert( valEntryPair.second );
        }
    }
    else
    {
        // No filtering
        filteredBarEntries = m_sortedBarEntries;
    }

    // Establish the max value within each group

    std::multiset<BarEntry> filteredSortedBarEntries;
    if ( m_isSortingByMaxValueInGroups )
    {
        std::map<QString, std::map<QString, double>> maxValuesPerMidGroup;
        std::map<QString, double>                    maxValuesPerMajGroup;

        for ( const BarEntry& barDef : filteredBarEntries )
        {
            if ( !barDef.m_majTickText.isEmpty() )
            {
                auto it_IsInsertedPair = maxValuesPerMajGroup.insert( std::make_pair( barDef.m_majTickText, barDef.m_sortValue ) );
                if ( !it_IsInsertedPair.second )
                    it_IsInsertedPair.first->second = std::max( it_IsInsertedPair.first->second, barDef.m_sortValue );
            }

            if ( !barDef.m_midTickText.isEmpty() )
            {
                auto it_IsInsertedPair =
                    maxValuesPerMidGroup[barDef.m_majTickText].insert( std::make_pair( barDef.m_midTickText, barDef.m_sortValue ) );
                if ( !it_IsInsertedPair.second )
                    it_IsInsertedPair.first->second = std::max( it_IsInsertedPair.first->second, barDef.m_sortValue );
            }
        }

        for ( BarEntry barDef : filteredBarEntries )
        {
            {
                auto it = maxValuesPerMajGroup.find( barDef.m_majTickText );
                if ( it != maxValuesPerMajGroup.end() )
                {
                    barDef.m_majorSortValue = it->second;
                }
            }

            {
                auto mapIt = maxValuesPerMidGroup.find( barDef.m_majTickText );
                if ( mapIt != maxValuesPerMidGroup.end() )
                {
                    auto it = mapIt->second.find( barDef.m_midTickText );
                    if ( it != mapIt->second.end() )
                    {
                        barDef.m_midSortValue = it->second;
                    }
                }
            }

            filteredSortedBarEntries.insert( barDef );
        }
    }
    else
    {
        // No sorting by max group member
        filteredSortedBarEntries = filteredBarEntries;
    }

    // clang-format off
    auto addGroupTickText = [&]( double groupStartPos, QString tickText, QList<double>& groupTickPosList )
    {
        if( tickText.isEmpty() ) return;

        double tickPos = midPoint( groupStartPos, currentBarPosition );

        QwtScaleDiv::TickType ttyp = (&groupTickPosList == &majTickPositions ) ? QwtScaleDiv::MajorTick
                                                                               : ( &groupTickPosList == &midTickPositions ) ? QwtScaleDiv::MediumTick
                                                                                                                            : QwtScaleDiv::MinorTick;

        // Make sure we do not get ticks of different level exactly at the same spot, 
        // so that the drawing is able to distinguish

        if( ttyp == QwtScaleDiv::MinorTick ) tickPos += 2e-4;
        if( ttyp == QwtScaleDiv::MediumTick ) tickPos += 1e-4;

        groupPositionedAxisTexts[tickPos] = RiuBarChartScaleDraw::RiuBarChartTick(ttyp, tickText, true);

        groupTickPosList.push_back( tickPos );
    };

    auto addGroupDivider = [&]( double position, QList<double>& groupDividerPosList)
    {
        QwtScaleDiv::TickType ttyp =
            (&groupDividerPosList == &majDividerPositions) ? QwtScaleDiv::MajorTick
                                                           : (&groupDividerPosList == &midDividerPositions) ? QwtScaleDiv::MediumTick
                                                                                                            : QwtScaleDiv::MinorTick;

        // Make sure we do not get ticks of different level exactly at the same spot, 
        // so that the drawing is able to distinguish
        double spacing = majGroupSpacing;

        if( ttyp == QwtScaleDiv::MediumTick ){ spacing = midGroupSpacing; }
        if( ttyp == QwtScaleDiv::MinorTick ) { spacing = minGroupSpacing; }

        groupDividerPosList.push_back(currentBarPosition - 0.5 - 0.5*spacing);
    };

    // clang-format on

    // Loop over entries, calculate tick positions and bar positions as we go

    for ( const BarEntry& barDef : filteredSortedBarEntries )
    {
        bool hasAnyMajTics         = !majTickTexts.empty();
        auto majInsertResult       = majTickTexts.insert( barDef.m_majTickText );
        bool isStartingNewMajGroup = majInsertResult.second;
        bool isFinishingMajGroup   = isStartingNewMajGroup && hasAnyMajTics;

        if ( isFinishingMajGroup )
        {
            addGroupTickText( currentMajGroupStartPos, previousMajText, majTickPositions );
            addGroupTickText( currentMidGroupStartPos, previousMidText, midTickPositions );
            addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPositions );

            currentBarPosition += majGroupSpacing;

            addGroupDivider( currentBarPosition, majDividerPositions );
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
            addGroupTickText( currentMidGroupStartPos, previousMidText, midTickPositions );
            addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPositions );

            currentBarPosition += midGroupSpacing;

            addGroupDivider( currentBarPosition, midDividerPositions );
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
            addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPositions );

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

        barPoints->push_back( { currentBarPosition, barDef.m_value } );
        if ( !barDef.m_barText.isEmpty() )
        {
            positionedBarLabels[currentBarPosition] =
                RiuBarChartScaleDraw::RiuBarChartTick( QwtScaleDiv::MinorTick, barDef.m_barText, barDef.m_value < 0.0 );
        }

        // Increment the bar position for the next bar
        currentBarPosition += 1.0;
    }

    // Add group tick texts for the last groups
    {
        if ( !previousMajText.isEmpty() )
        {
            addGroupTickText( currentMajGroupStartPos, previousMajText, majTickPositions );
        }

        if ( !previousMidText.isEmpty() )
        {
            addGroupTickText( currentMidGroupStartPos, previousMidText, midTickPositions );
        }

        if ( !previousMinText.isEmpty() )
        {
            addGroupTickText( currentMinGroupStartPos, previousMinText, minTickPositions );
        }
    }

    // Create QwtBarCharts for each of the legend groups

    int idx = 0;
    for ( const auto& legendToBarPointsPair : legendToBarPointsMap )
    {
        QColor color = RiaColorTables::categoryPaletteColors().cycledQColor( idx );

        if ( m_useBarColor ) color = m_barColor;

        addQwtBarChart( plot, legendToBarPointsPair.second, legendToBarPointsPair.first, color, barOrientation );
        idx++;
    }

    // Set up the axis to contain group texts and tick marks
    {
        QwtAxis::Position axis      = QwtAxis::XBottom;
        QwtAxis::Position valueAxis = QwtAxis::YLeft;

        if ( barOrientation == Qt::Horizontal )
        {
            axis      = QwtAxis::YLeft;
            valueAxis = QwtAxis::XBottom;
        }

        QwtScaleDiv groupAxisScaleDiv( 0, currentBarPosition );
        {
            if ( !majTickPositions.empty() ) groupAxisScaleDiv.setTicks( QwtScaleDiv::MajorTick, majTickPositions );
            if ( !midTickPositions.empty() ) groupAxisScaleDiv.setTicks( QwtScaleDiv::MediumTick, midTickPositions );
            if ( !minTickPositions.empty() ) groupAxisScaleDiv.setTicks( QwtScaleDiv::MinorTick, minTickPositions );

            if ( barOrientation == Qt::Horizontal )
            {
                groupAxisScaleDiv.invert();
            }
        }

        QColor textColor = RiuGuiTheme::getColorByVariableName( "textColor" );

        RiuBarChartScaleDraw* scaleDrawer = new RiuBarChartScaleDraw( groupPositionedAxisTexts, m_labelPointSize, textColor );

        plot->setAxisScaleDraw( axis, scaleDrawer );
        plot->setAxisScaleDiv( axis, groupAxisScaleDiv );

        // Set up the value axis

        plot->setAxisAutoScale( valueAxis, true );
        plot->setAxisScaleDraw( valueAxis, new QwtScaleDraw() );
    }

    // Setup grids
    {
        QwtPlotGrid* plotGrid  = nullptr;
        QwtPlotGrid* groupGrid = nullptr;

        // Get or create the two grids used in the plot
        {
            QwtPlotItemList gridList = plot->itemList( QwtPlotItem::Rtti_PlotGrid );
            for ( QwtPlotItem* plItem : gridList )
            {
                QwtPlotGrid* someGrid = dynamic_cast<QwtPlotGrid*>( plItem );
                if ( someGrid )
                {
                    if ( someGrid->title() == QString( "GroupGrid" ) )
                    {
                        groupGrid = someGrid;
                    }
                    else
                    {
                        plotGrid = someGrid;
                    }
                }
            }

            if ( !groupGrid )
            {
                groupGrid = new QwtPlotGrid;
                groupGrid->setTitle( QString( "GroupGrid" ) );
                groupGrid->attach( plot );
                QPen gridPen( Qt::SolidLine );
                gridPen.setColor( Qt::lightGray ); // QColor( 240, 240, 240 ) );
                groupGrid->setPen( gridPen );
                groupGrid->enableYMin( true );
                groupGrid->enableXMin( true );
            }
        }

        if ( groupGrid )
        {
            QwtScaleDiv gridDividerScaleDiv( 0, currentBarPosition );
            if ( !majDividerPositions.empty() )
            {
                gridDividerScaleDiv.setTicks( QwtScaleDiv::MajorTick, majDividerPositions );
            }

            if ( !midDividerPositions.empty() )
            {
                gridDividerScaleDiv.setTicks( QwtScaleDiv::MediumTick, midDividerPositions );
            }

            if ( !minDividerPositions.empty() )
            {
                gridDividerScaleDiv.setTicks( QwtScaleDiv::MinorTick, minDividerPositions );
            }

            if ( barOrientation == Qt::Horizontal )
            {
                gridDividerScaleDiv.invert();
                groupGrid->setYDiv( gridDividerScaleDiv );
                groupGrid->enableX( false );
                groupGrid->enableY( true );
            }
            else
            {
                groupGrid->setXDiv( gridDividerScaleDiv );
                groupGrid->enableX( true );
                groupGrid->enableY( false );
            }

            groupGrid->setItemInterest( QwtPlotItem::ScaleInterest, false );
        }

        if ( plotGrid )
        {
            if ( barOrientation == Qt::Horizontal )
            {
                plotGrid->enableX( true );
                plotGrid->enableY( false );
            }
            else
            {
                plotGrid->enableX( false );
                plotGrid->enableY( true );
            }
        }
    }

    // Add texts on the bars inside the plot
    {
        QwtScaleDraw::Alignment alignment      = QwtScaleDraw::TopScale;
        double                  labelRotation  = -90.0;
        Qt::Alignment           labelAlignment = Qt::AlignVCenter | Qt::AlignRight;
        if ( barOrientation == Qt::Horizontal )
        {
            alignment     = QwtScaleDraw::RightScale;
            labelRotation = 0.0;
        }

        QwtScaleDiv barTextScaleDiv( 0, currentBarPosition );
        {
            QList<double> onBarTickPositions;

            for ( const auto& doubleStuffPair : positionedBarLabels )
            {
                onBarTickPositions.push_back( doubleStuffPair.first );
            }

            barTextScaleDiv.setTicks( QwtScaleDiv::MinorTick, onBarTickPositions );
            if ( barOrientation == Qt::Horizontal )
            {
                barTextScaleDiv.invert();
            }
        }

        QColor textColor = RiuGuiTheme::getColorByVariableName( "textColor" );

        RiuBarChartScaleDraw* barTextScaleDrawer = new RiuBarChartScaleDraw( positionedBarLabels, m_labelPointSize, textColor );
        barTextScaleDrawer->setAlignment( alignment );
        barTextScaleDrawer->setLabelRotation( labelRotation );
        barTextScaleDrawer->setLabelAlignment( labelAlignment );

        QwtPlotScaleItem* barTextScale = new QwtPlotScaleItem( alignment, 0.0 );
        barTextScale->setScaleDraw( barTextScaleDrawer );
        barTextScale->setScaleDiv( barTextScaleDiv );
        barTextScale->attach( plot );
        barTextScale->setZ( 1000 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::setLabelFontSize( int labelPointSize )
{
    m_labelPointSize = labelPointSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuGroupedBarChartBuilder::plotContentAsText() const
{
    QString text;

    QTextStream              stream( &text );
    QString                  fieldSeparator = "\t";
    RifCsvDataTableFormatter formatter( stream, fieldSeparator );
    formatter.setUseQuotes( false );

    bool hasTextForMajor  = false;
    bool hasTextForMid    = false;
    bool hasTextForMinor  = false;
    bool hasTextForLegend = false;
    bool hasTextForBar    = false;

    for ( const BarEntry& barDef : m_sortedBarEntries )
    {
        if ( !barDef.m_majTickText.isEmpty() ) hasTextForMajor = true;
        if ( !barDef.m_midTickText.isEmpty() ) hasTextForMid = true;
        if ( !barDef.m_minTickText.isEmpty() ) hasTextForMinor = true;
        if ( !barDef.m_legendText.isEmpty() ) hasTextForLegend = true;
        if ( !barDef.m_barText.isEmpty() && barDef.m_barText != barDef.m_legendText ) hasTextForBar = true;
    }

    std::vector<RifTextDataTableColumn> header;
    if ( hasTextForMajor ) header.emplace_back( RifTextDataTableColumn( "Major" ) );
    if ( hasTextForMid ) header.emplace_back( RifTextDataTableColumn( "Mid" ) );
    if ( hasTextForMinor ) header.emplace_back( RifTextDataTableColumn( "Minor" ) );
    if ( hasTextForLegend ) header.emplace_back( RifTextDataTableColumn( "Legend" ) );
    if ( hasTextForBar ) header.emplace_back( RifTextDataTableColumn( "Bar" ) );

    header.emplace_back( RifTextDataTableColumn( "Value", RifTextDataTableDoubleFormat::RIF_FLOAT ) );

    formatter.header( header );

    for ( const BarEntry& barDef : m_sortedBarEntries )
    {
        if ( hasTextForMajor ) formatter.add( barDef.m_majTickText );
        if ( hasTextForMid ) formatter.add( barDef.m_midTickText );
        if ( hasTextForMinor ) formatter.add( barDef.m_minTickText );
        if ( hasTextForLegend ) formatter.add( barDef.m_legendText );
        if ( hasTextForBar ) formatter.add( barDef.m_barText );

        formatter.add( barDef.m_value );

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupedBarChartBuilder::addQwtBarChart( QwtPlot*                plot,
                                                const QVector<QPointF>& posAndValue,
                                                const QString&          legendText,
                                                const QColor&           barColor,
                                                Qt::Orientation         orientation )
{
    QPalette palette;
    palette.setColor( QPalette::Window, barColor );
    palette.setColor( QPalette::Dark, barColor );
    palette.setColor( QPalette::Text, Qt::black );

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
    barChart->setOrientation( orientation );
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
RiuGroupedBarChartBuilder::BarEntry::BarEntry( const QString& majorTickText,
                                               const QString& midTickText,
                                               const QString& minTickText,
                                               const double   sortValue,
                                               const QString& legendText,
                                               const QString& barText,
                                               const double   value )
    : m_majTickText( majorTickText )
    , m_midTickText( midTickText )
    , m_minTickText( minTickText )
    , m_sortValue( sortValue )
    , m_legendText( legendText )
    , m_barText( barText )
    , m_value( value )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGroupedBarChartBuilder::BarEntry::operator<( const BarEntry& other ) const
{
    if ( m_majorSortValue != other.m_majorSortValue ) return m_majorSortValue > other.m_majorSortValue;
    if ( m_majTickText != other.m_majTickText ) return m_majTickText < other.m_majTickText;
    if ( m_midSortValue != other.m_midSortValue ) return m_midSortValue > other.m_midSortValue;
    if ( m_midTickText != other.m_midTickText ) return m_midTickText < other.m_midTickText;
    if ( m_minTickText != other.m_minTickText ) return m_minTickText < other.m_minTickText;

    if ( m_sortValue != other.m_sortValue )
    {
        return m_sortValue > other.m_sortValue;
    }

    if ( m_legendText != other.m_legendText ) return m_legendText < other.m_legendText;

    return false;
}
