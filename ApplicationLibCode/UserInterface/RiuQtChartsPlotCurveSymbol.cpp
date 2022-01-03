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

#include "RiuQtChartsPlotCurveSymbol.h"

#include "RiaColorTools.h"
#include "RiaFontCache.h"
#include "RiaLogging.h"

#include "RiuPlotCurveSymbol.h"
#include "cvfAssert.h"

#include <QPainter>
#include <QPainterPath>
#include <QScatterSeries>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotCurveSymbol::RiuQtChartsPlotCurveSymbol( PointSymbolEnum riuStyle,
                                                        const QString&  label,
                                                        LabelPosition   labelPosition,
                                                        int             labelFontSizePt )
    : RiuPlotCurveSymbol( riuStyle, label, labelPosition, labelFontSizePt )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurveSymbol::setSize( int width, int height )
{
    m_size = std::min( width, height );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurveSymbol::setColor( const QColor& color )
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurveSymbol::setPen( const QPen& pen )
{
    m_pen = pen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurveSymbol::applyToScatterSeries( QtCharts::QScatterSeries* series ) const
{
    if ( m_style == PointSymbolEnum::SYMBOL_NONE )
    {
        series->hide();
        return;
    }

    series->show();

    if ( m_style == PointSymbolEnum::SYMBOL_RECT )
    {
        setImageBrush( series, createRectImage() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_ELLIPSE )
    {
        setImageBrush( series, createEllipseImage() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_CROSS )
    {
        setImageBrush( series, createCrossImage() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_XCROSS )
    {
        setImageBrush( series, createXCrossImage() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_DIAMOND )
    {
        setImageBrush( series, createDiamondImage() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_HEXAGON )
    {
        setImageBrush( series, createHexagonImage() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_STAR1 )
    {
        setImageBrush( series, createStar1Image() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_STAR1 )
    {
        setImageBrush( series, createStar1Image() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_STAR2 )
    {
        setImageBrush( series, createStar2Image() );
    }
    else if ( m_style == PointSymbolEnum::SYMBOL_TRIANGLE || m_style == PointSymbolEnum::SYMBOL_UP_TRIANGLE ||
              m_style == PointSymbolEnum::SYMBOL_DOWN_TRIANGLE || m_style == PointSymbolEnum::SYMBOL_LEFT_TRIANGLE ||
              m_style == PointSymbolEnum::SYMBOL_RIGHT_TRIANGLE )
    {
        setImageBrush( series, createTriangleImage( m_style ) );
    }
    else
    {
        RiaLogging::warning( "Missing symbol style." );
    }

    series->setMarkerSize( m_size );
    series->setColor( m_color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurveSymbol::setImageBrush( QtCharts::QScatterSeries* series, const QImage& image ) const
{
    series->setMarkerShape( QtCharts::QScatterSeries::MarkerShapeRectangle );
    series->setBrush( image );
    series->setPen( QColor( Qt::transparent ) );
}

//--------------------------------------------------------------------------------------------------
/// Adapted from QwtSymbol::qwtDrawTriangleSymbols
//--------------------------------------------------------------------------------------------------
QImage RiuQtChartsPlotCurveSymbol::createTriangleImage( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle ) const
{
    QSize size( m_size, m_size );

    QImage star( size.width(), size.height(), QImage::Format_ARGB32 );
    star.fill( Qt::transparent );

    QPainter painter( &star );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

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

    if ( symbolStyle == PointSymbolEnum::SYMBOL_LEFT_TRIANGLE )
    {
        trianglePoints[0].rx() = x2;
        trianglePoints[0].ry() = y1;

        trianglePoints[1].rx() = x1;
        trianglePoints[1].ry() = y;

        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y2;
    }

    if ( symbolStyle == PointSymbolEnum::SYMBOL_RIGHT_TRIANGLE )
    {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y1;

        trianglePoints[1].rx() = x2;
        trianglePoints[1].ry() = y;

        trianglePoints[2].rx() = x1;
        trianglePoints[2].ry() = y2;
    }

    if ( symbolStyle == PointSymbolEnum::SYMBOL_UP_TRIANGLE || symbolStyle == PointSymbolEnum::SYMBOL_TRIANGLE )
    {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y2;

        trianglePoints[1].rx() = x;
        trianglePoints[1].ry() = y1;

        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y2;
    }

    if ( symbolStyle == PointSymbolEnum::SYMBOL_DOWN_TRIANGLE )
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
QImage RiuQtChartsPlotCurveSymbol::createCrossImage() const
{
    const QSize size( m_size, m_size );

    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

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
QImage RiuQtChartsPlotCurveSymbol::createXCrossImage() const
{
    const QSize size( m_size, m_size );

    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

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
QImage RiuQtChartsPlotCurveSymbol::createRectImage() const
{
    const QSize size( m_size, m_size );

    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

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
QImage RiuQtChartsPlotCurveSymbol::createEllipseImage() const
{
    const QSize size( m_size, m_size );

    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

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
QImage RiuQtChartsPlotCurveSymbol::createStar1Image() const
{
    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

    QRectF r( 0, 0, m_size, m_size );

    const double sqrt1_2 = 1.0 / sqrt( 2 );

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
QImage RiuQtChartsPlotCurveSymbol::createStar2Image() const
{
    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

    QRectF r( 0, 0, m_size, m_size );

    const double cos30 = 0.866025; // cos(30°)

    const double dy = 0.25 * m_size;
    const double dx = 0.5 * m_size * cos30 / 3.0;

    QPolygonF star( 12 );
    QPointF*  starPoints = star.data();

    double x = m_size * 0.5;
    double y = m_size * 0.5;

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
QImage RiuQtChartsPlotCurveSymbol::createHexagonImage() const
{
    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

    QRectF r( 0, 0, m_size, m_size );

    const double cos30 = 0.866025; // cos(30°)
    const double dx    = 0.5 * ( m_size - cos30 );

    const double dy = 0.25 * m_size;

    QPolygonF hexaPolygon( 6 );
    QPointF*  hexaPoints = hexaPolygon.data();

    double x = m_size * 0.5;
    double y = m_size * 0.5;

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
QImage RiuQtChartsPlotCurveSymbol::createDiamondImage() const
{
    QImage image( m_size, m_size, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing );

    QPen pen( m_pen );
    pen.setWidth( 1.0 );
    pen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( pen );
    painter.setBrush( m_color );

    QSize size( m_size, m_size );

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
