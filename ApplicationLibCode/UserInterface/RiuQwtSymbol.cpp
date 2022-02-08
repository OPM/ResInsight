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

#include "RiaColorTools.h"
#include "RiaFontCache.h"

#include "RiuPlotCurveSymbol.h"
#include "cvfAssert.h"
#include "qwt_symbol.h"

#include <QPainter>
#include <QPainterPath>

//--------------------------------------------------------------------------------------------------
/// Internal class to support labels on symbols
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::RiuQwtSymbol( PointSymbolEnum riuStyle, const QString& label, LabelPosition labelPosition, int labelFontSizePt )
    : RiuPlotCurveSymbol( riuStyle, label, labelPosition, labelFontSizePt )
    , QwtSymbol( QwtSymbol::NoSymbol )
{
    QwtSymbol::Style style = QwtSymbol::NoSymbol;

    switch ( riuStyle )
    {
        case PointSymbolEnum::SYMBOL_ELLIPSE:
            style = QwtSymbol::Ellipse;
            break;
        case PointSymbolEnum::SYMBOL_RECT:
            style = QwtSymbol::Rect;
            break;
        case PointSymbolEnum::SYMBOL_DIAMOND:
            style = QwtSymbol::Diamond;
            break;
        case PointSymbolEnum::SYMBOL_TRIANGLE:
            style = QwtSymbol::Triangle;
            break;
        case PointSymbolEnum::SYMBOL_CROSS:
            style = QwtSymbol::Cross;
            break;
        case PointSymbolEnum::SYMBOL_XCROSS:
            style = QwtSymbol::XCross;
            break;
        case PointSymbolEnum::SYMBOL_DOWN_TRIANGLE:
            style = QwtSymbol::DTriangle;
            break;
        case PointSymbolEnum::SYMBOL_LEFT_ALIGNED_TRIANGLE:
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
        case PointSymbolEnum::SYMBOL_RIGHT_ALIGNED_TRIANGLE:
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
        case PointSymbolEnum::SYMBOL_LEFT_ANGLED_TRIANGLE:
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
        case PointSymbolEnum::SYMBOL_RIGHT_ANGLED_TRIANGLE:
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
        case PointSymbolEnum::SYMBOL_UP_TRIANGLE:
            style = QwtSymbol::UTriangle;
            break;
        case PointSymbolEnum::SYMBOL_STAR1:
            style = QwtSymbol::Star1;
            break;
        case PointSymbolEnum::SYMBOL_STAR2:
            style = QwtSymbol::Star2;
            break;
        case PointSymbolEnum::SYMBOL_HEXAGON:
            style = QwtSymbol::Hexagon;
            break;
        case PointSymbolEnum::SYMBOL_LEFT_TRIANGLE:
            style = QwtSymbol::LTriangle;
            break;
        case PointSymbolEnum::SYMBOL_RIGHT_TRIANGLE:
            style = QwtSymbol::RTriangle;
        default:
            break;
    }
    setStyle( style );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::setSize( int width, int height )
{
    QwtSymbol::setSize( width, height );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::setColor( const QColor& color )
{
    QwtSymbol::setColor( color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::setPen( const QPen& pen )
{
    QwtSymbol::setPen( pen );
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
    painter->setPen( RiaColorTools::textColor() );

    QSize symbolSize = QwtSymbol::size();
    QRect symbolRect( position.x(), position.y(), symbolSize.width(), symbolSize.height() );
    QRect labelRect = labelBoundingRect( painter, symbolRect, label );
    painter->drawText( labelRect.topLeft(), label );
    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtSymbol::setPixmap( const QPixmap& pixmap )
{
    QwtSymbol::setPixmap( pixmap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRect RiuQwtSymbol::boundingRect() const
{
    return QwtSymbol::boundingRect();
}
