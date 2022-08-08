/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RiuPlotCurveSymbolImageCreator.h"

#include "cafFontTools.h"
#include "cvfAssert.h"

#include <QPainter>
#include <QRect>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createSymbolImage( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle,
                                                          const QSize&                        size,
                                                          const QPen&                         pen,
                                                          const QColor&                       color )
{
    if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_NONE )
    {
        return QImage();
    }

    if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_RECT )
    {
        return createRectImage( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_ELLIPSE )
    {
        return createEllipseImage( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_CROSS )
    {
        return createCrossImage( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_XCROSS )
    {
        return createXCrossImage( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_DIAMOND )
    {
        return createDiamondImage( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_HEXAGON )
    {
        return createHexagonImage( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_STAR1 )
    {
        return createStar1Image( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_STAR1 )
    {
        return createStar1Image( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_STAR2 )
    {
        return createStar2Image( size, pen, color );
    }
    else if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_TRIANGLE ||
              symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_UP_TRIANGLE ||
              symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_DOWN_TRIANGLE ||
              symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_LEFT_TRIANGLE ||
              symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_RIGHT_TRIANGLE )
    {
        return createTriangleImage( symbolStyle, size, pen, color );
    }

    return QImage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createSymbolImage( RiuPlotCurveSymbol::RiuPlotCurveSymbol::PointSymbolEnum symbolStyle,
                                                          const QSize&                                            size,
                                                          const QColor& color )
{
    QPen defaultPen;
    return createSymbolImage( symbolStyle, size, defaultPen, color );
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawTriangleSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createTriangleImage( RiuPlotCurveSymbol::RiuPlotCurveSymbol::PointSymbolEnum symbolStyle,
                                                            const QSize&  size,
                                                            const QPen&   m_pen,
                                                            const QColor& color )
{
    QImage star( size, QImage::Format_ARGB32 );
    star.fill( Qt::transparent );

    QPainter painter( &star );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    double sw2 = 0.5 * size.width();
    double sh2 = 0.5 * size.height();

    QPolygonF triangle( 3 );
    QPointF*  trianglePoints = triangle.data();

    double x = size.width() * 0.5;
    double y = size.width() * 0.5;

    const double x1 = x - sw2;
    const double x2 = x1 + size.width();
    const double y1 = y - sh2;
    const double y2 = y1 + size.height();

    if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_LEFT_TRIANGLE )
    {
        trianglePoints[0].rx() = x2;
        trianglePoints[0].ry() = y1;

        trianglePoints[1].rx() = x1;
        trianglePoints[1].ry() = y;

        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y2;
    }

    if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_RIGHT_TRIANGLE )
    {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y1;

        trianglePoints[1].rx() = x2;
        trianglePoints[1].ry() = y;

        trianglePoints[2].rx() = x1;
        trianglePoints[2].ry() = y2;
    }

    if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_UP_TRIANGLE ||
         symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_TRIANGLE )
    {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y2;

        trianglePoints[1].rx() = x;
        trianglePoints[1].ry() = y1;

        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y2;
    }

    if ( symbolStyle == RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_DOWN_TRIANGLE )
    {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y1;

        trianglePoints[1].rx() = x;
        trianglePoints[1].ry() = y2;

        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y1;
    }

    painter.drawPolygon( triangle );

    return star;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawLineSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createCrossImage( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    const double sw  = size.width();
    const double sh  = size.height();
    const double sw2 = 0.5 * size.width();
    const double sh2 = 0.5 * size.height();

    double midX = size.width() * 0.5;
    double midY = size.height() * 0.5;

    {
        const double x = midX - sw2;
        const double y = midY;

        painter.drawLine( x, y, x + sw, y );
    }
    {
        const double y = midY - sh2;
        const double x = midX;

        painter.drawLine( x, y, x, y + sh );
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawXCrossSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createXCrossImage( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    const double sw  = size.width();
    const double sh  = size.height();
    const double sw2 = 0.5 * size.width();
    const double sh2 = 0.5 * size.height();

    double midX = size.width() * 0.5;
    double midY = size.height() * 0.5;

    const double x1 = midX - sw2;
    const double x2 = x1 + sw;
    const double y1 = midY - sh2;
    const double y2 = y1 + sh;

    painter.drawLine( x1, y1, x2, y2 );
    painter.drawLine( x1, y2, x2, y1 );

    return image;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawRectSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createRectImage( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    const double sw  = size.width();
    const double sh  = size.height();
    const double sw2 = 0.5 * size.width();
    const double sh2 = 0.5 * size.height();

    // Mid point
    const double x = sw * 0.5;
    const double y = sw * 0.5;

    const QRectF r( x - sw2, y - sh2, sw, sh );
    painter.drawRect( r );

    return image;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawEllipseSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createEllipseImage( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    const double sw  = size.width();
    const double sh  = size.height();
    const double sw2 = 0.5 * size.width();
    const double sh2 = 0.5 * size.height();

    // Mid point
    const double x = sw * 0.5;
    const double y = sw * 0.5;

    const QRectF r( x - sw2, y - sh2, sw, sh );
    painter.drawEllipse( r );

    return image;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawStar1Symbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createStar1Image( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    QRectF r( 0, 0, size.width(), size.height() );

    const double sqrt1_2 = 1.0 / std::sqrt( 2 );

    const QPointF c  = r.center();
    const double  d1 = r.width() / 2.0 * ( 1.0 - sqrt1_2 );

    painter.drawLine( r.left() + d1, r.top() + d1, r.right() - d1, r.bottom() - d1 );
    painter.drawLine( r.left() + d1, r.bottom() - d1, r.right() - d1, r.top() + d1 );
    painter.drawLine( c.x(), r.top(), c.x(), r.bottom() );
    painter.drawLine( r.left(), c.y(), r.right(), c.y() );

    return image;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawStar2Symbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createStar2Image( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    QRectF r( 0, 0, size.width(), size.height() );

    const double cos30 = 0.866025; // cos(30°)

    const double dy = 0.25 * size.height();
    const double dx = 0.5 * size.width() * cos30 / 3.0;

    QPolygonF star( 12 );
    QPointF*  starPoints = star.data();

    double x = size.width() * 0.5;
    double y = size.height() * 0.5;

    double x1 = x - 3 * dx;
    double y1 = y - 2 * dy;

    const double x2 = x1 + 1 * dx;
    const double x3 = x1 + 2 * dx;
    const double x4 = x1 + 3 * dx;
    const double x5 = x1 + 4 * dx;
    const double x6 = x1 + 5 * dx;
    const double x7 = x1 + 6 * dx;

    const double y2 = y1 + 1 * dy;
    const double y3 = y1 + 2 * dy;
    const double y4 = y1 + 3 * dy;
    const double y5 = y1 + 4 * dy;

    starPoints[0].rx() = x4;
    starPoints[0].ry() = y1;

    starPoints[1].rx() = x5;
    starPoints[1].ry() = y2;

    starPoints[2].rx() = x7;
    starPoints[2].ry() = y2;

    starPoints[3].rx() = x6;
    starPoints[3].ry() = y3;

    starPoints[4].rx() = x7;
    starPoints[4].ry() = y4;

    starPoints[5].rx() = x5;
    starPoints[5].ry() = y4;

    starPoints[6].rx() = x4;
    starPoints[6].ry() = y5;

    starPoints[7].rx() = x3;
    starPoints[7].ry() = y4;

    starPoints[8].rx() = x1;
    starPoints[8].ry() = y4;

    starPoints[9].rx() = x2;
    starPoints[9].ry() = y3;

    starPoints[10].rx() = x1;
    starPoints[10].ry() = y2;

    starPoints[11].rx() = x3;
    starPoints[11].ry() = y2;

    painter.drawPolygon( star );

    return image;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawHexagonSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createHexagonImage( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    QRectF r( 0, 0, size.width(), size.height() );

    const double cos30 = 0.866025; // cos(30°)
    const double dx    = 0.5 * ( size.height() - cos30 );

    const double dy = 0.25 * size.height();

    QPolygonF hexaPolygon( 6 );
    QPointF*  hexaPoints = hexaPolygon.data();

    double x = size.width() * 0.5;
    double y = size.height() * 0.5;

    double x1 = x - dx;
    double y1 = y - 2 * dy;

    const double x2 = x1 + 1 * dx;
    const double x3 = x1 + 2 * dx;

    const double y2 = y1 + 1 * dy;
    const double y3 = y1 + 3 * dy;
    const double y4 = y1 + 4 * dy;

    hexaPoints[0].rx() = x2;
    hexaPoints[0].ry() = y1;

    hexaPoints[1].rx() = x3;
    hexaPoints[1].ry() = y2;

    hexaPoints[2].rx() = x3;
    hexaPoints[2].ry() = y3;

    hexaPoints[3].rx() = x2;
    hexaPoints[3].ry() = y4;

    hexaPoints[4].rx() = x1;
    hexaPoints[4].ry() = y3;

    hexaPoints[5].rx() = x1;
    hexaPoints[5].ry() = y2;

    painter.drawPolygon( hexaPolygon );
    return image;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawDiamondSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuPlotCurveSymbolImageCreator::createDiamondImage( const QSize& size, const QPen& m_pen, const QColor& color )
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( color );

    double       x  = size.width() * 0.5;
    double       y  = size.height() * 0.5;
    const double x1 = x - 0.5 * size.width();
    const double y1 = y - 0.5 * size.height();
    const double x2 = x1 + size.width();
    const double y2 = y1 + size.height();

    QPolygonF polygon;
    polygon += QPointF( x, y1 );
    polygon += QPointF( x2, y );
    polygon += QPointF( x, y2 );
    polygon += QPointF( x1, y );

    painter.drawPolygon( polygon );

    return image;
}
