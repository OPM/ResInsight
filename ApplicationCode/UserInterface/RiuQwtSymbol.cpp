/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Equinor ASA
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

#include "RiuQwtSymbol.h"

#include "RiaFontCache.h"

#include "cvfAssert.h"

#include <QPainter>

//--------------------------------------------------------------------------------------------------
/// Internal class to support labels on symbols
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::RiuQwtSymbol( PointSymbolEnum riuStyle, const QString& label, LabelPosition labelPosition, int labelFontSizePt )
    : QwtSymbol( QwtSymbol::NoSymbol )
    , m_globalLabel( label )
    , m_labelPosition( labelPosition )
    , m_labelFontSizePx( caf::FontTools::pointSizeToPixelSize( labelFontSizePt ) )
{
    QwtSymbol::Style style = QwtSymbol::NoSymbol;

    switch ( riuStyle )
    {
        case SYMBOL_ELLIPSE:
            style = QwtSymbol::Ellipse;
            break;
        case SYMBOL_RECT:
            style = QwtSymbol::Rect;
            break;
        case SYMBOL_DIAMOND:
            style = QwtSymbol::Diamond;
            break;
        case SYMBOL_TRIANGLE:
            style = QwtSymbol::Triangle;
            break;
        case SYMBOL_CROSS:
            style = QwtSymbol::Cross;
            break;
        case SYMBOL_XCROSS:
            style = QwtSymbol::XCross;
            break;
        case SYMBOL_DOWN_TRIANGLE:
            style = QwtSymbol::DTriangle;
            break;
        case SYMBOL_LEFT_ALIGNED_TRIANGLE:
            style = QwtSymbol::Path;
            {
                QPainterPath path;
                path.moveTo( 0, 0 );
                path.lineTo( -10, 10 );
                path.lineTo( 0, 20 );
                path.lineTo( 0, 0 );
                setPath( path );
                setPinPoint( QPointF( 0, 0 ) );
            }
            break;
        case SYMBOL_RIGHT_ALIGNED_TRIANGLE:
            style = QwtSymbol::Path;
            {
                QPainterPath path;
                path.moveTo( 0, 0 );
                path.lineTo( 10, 10 );
                path.lineTo( 0, 20 );
                path.lineTo( 0, 0 );
                setPath( path );
                setPinPoint( QPointF( 0, 0 ) );
            }
            break;
        case SYMBOL_LEFT_ANGLED_TRIANGLE:
            style = QwtSymbol::Path;
            {
                QPainterPath path;
                path.moveTo( 0, 0 );
                path.lineTo( 0, 10 );
                path.lineTo( -10, 10 );
                path.lineTo( 0, 0 );
                setPath( path );
                setPinPoint( QPointF( 0, 10 ) );
            }
            break;
        case SYMBOL_RIGHT_ANGLED_TRIANGLE:
            style = QwtSymbol::Path;
            {
                QPainterPath path;
                path.moveTo( 0, 0 );
                path.lineTo( 0, 10 );
                path.lineTo( 10, 10 );
                path.lineTo( 0, 0 );
                setPath( path );
                setPinPoint( QPointF( 0, 10 ) );
            }
            break;
        case SYMBOL_UP_TRIANGLE:
            style = QwtSymbol::UTriangle;
            break;
        case SYMBOL_STAR1:
            style = QwtSymbol::Star1;
            break;
        case SYMBOL_STAR2:
            style = QwtSymbol::Star2;
            break;
        case SYMBOL_HEXAGON:
            style = QwtSymbol::Hexagon;
            break;
        case SYMBOL_LEFT_TRIANGLE:
            style = QwtSymbol::LTriangle;
            break;
        case SYMBOL_RIGHT_TRIANGLE:
            style = QwtSymbol::RTriangle;
        default:
            break;
    }
    setStyle( style );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::renderSymbols( QPainter* painter, const QPointF* points, int numPoints ) const
{
    QwtSymbol::renderSymbols( painter, points, numPoints );

    if ( !m_globalLabel.isEmpty() )
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            auto position = points[i];
            renderSymbolLabel( painter, position, m_globalLabel );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::renderSymbolLabel( QPainter* painter, const QPointF& position, const QString& label ) const
{
    painter->save();
    QFont font = painter->font();
    font.setPixelSize( m_labelFontSizePx );
    painter->setFont( font );

    QSize symbolSize = QwtSymbol::size();
    QRect symbolRect( position.x(), position.y(), symbolSize.width(), symbolSize.height() );
    QRect labelRect = labelBoundingRect( painter, symbolRect, label );
    painter->drawText( labelRect.topLeft(), label );
    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuQwtSymbol::globalLabel() const
{
    return m_globalLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::setGlobalLabel( const QString& label )
{
    m_globalLabel = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::setLabelPosition( LabelPosition labelPosition )
{
    m_labelPosition = labelPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum RiuQwtSymbol::cycledSymbolStyle( int indexLevel1, int indexLevel2 )
{
    std::vector<std::vector<PointSymbolEnum>> categorisedStyles = {
        {SYMBOL_ELLIPSE, SYMBOL_RECT, SYMBOL_DIAMOND},
        {SYMBOL_DOWN_TRIANGLE, SYMBOL_UP_TRIANGLE},
        {SYMBOL_LEFT_TRIANGLE, SYMBOL_RIGHT_TRIANGLE},
        {SYMBOL_CROSS, SYMBOL_XCROSS},
        {SYMBOL_STAR1, SYMBOL_STAR2},
    };

    int level1Category = indexLevel1 % int( categorisedStyles.size() );
    int level2Category = indexLevel2 % int( categorisedStyles[level1Category].size() );

    return categorisedStyles[level1Category][level2Category];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum RiuQwtSymbol::cycledSymbolStyle( int indexLevel )
{
    std::vector<PointSymbolEnum> contrastingSymbols = {SYMBOL_ELLIPSE,
                                                       SYMBOL_CROSS,
                                                       SYMBOL_RECT,
                                                       SYMBOL_DOWN_TRIANGLE,
                                                       SYMBOL_UP_TRIANGLE,
                                                       SYMBOL_LEFT_TRIANGLE,
                                                       SYMBOL_RIGHT_TRIANGLE,
                                                       SYMBOL_STAR2,
                                                       SYMBOL_DIAMOND,
                                                       SYMBOL_STAR1};

    return contrastingSymbols[indexLevel % (int)contrastingSymbols.size()];
}

//--------------------------------------------------------------------------------------------------
/// Is this a symbol with an interior and a border? If false, it is just lines.
//--------------------------------------------------------------------------------------------------
bool RiuQwtSymbol::isFilledSymbol( PointSymbolEnum symbol )
{
    return symbol != SYMBOL_NONE && symbol != SYMBOL_CROSS && symbol != SYMBOL_XCROSS && symbol != SYMBOL_STAR1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRect RiuQwtSymbol::labelBoundingRect( const QPainter* painter, const QRect& symbolRect, const QString& label ) const
{
    CVF_ASSERT( painter );

    QPoint symbolPosition = symbolRect.topLeft();

    int symbolWidth  = symbolRect.width();
    int symbolHeight = symbolRect.height();

    int labelWidth  = painter->fontMetrics().width( label );
    int labelHeight = painter->fontMetrics().height();

    QPoint labelPosition;
    if ( m_labelPosition == LabelAboveSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() - labelWidth / 2, symbolPosition.y() - 5 );
    }
    else if ( m_labelPosition == LabelBelowSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() - labelWidth / 2, symbolPosition.y() + symbolHeight + 5 );
    }
    else if ( m_labelPosition == LabelLeftOfSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() - labelWidth - symbolWidth, symbolPosition.y() );
    }
    else if ( m_labelPosition == LabelRightOfSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() + symbolWidth + 3, symbolPosition.y() );
    }
    return QRect( labelPosition.x(), labelPosition.y(), labelWidth, labelHeight );
}
