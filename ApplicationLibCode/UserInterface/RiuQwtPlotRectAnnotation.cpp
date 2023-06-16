/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RiuQwtPlotRectAnnotation.h"

#include "RiuGuiTheme.h"
#include "RiuQwtPlotCurveDefines.h"

#include "qwt_painter.h"
#include "qwt_plot_marker.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"
#include "qwt_text.h"

#include <QPainter>
#include <QRect>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotRectAnnotation::RiuQwtPlotRectAnnotation()
    : QwtPlotItem( QwtText( "PlotRectAnnotation" ) )
{
    setItemAttribute( QwtPlotItem::AutoScale, false );
    setItemAttribute( QwtPlotItem::Legend, false );

    QColor c( Qt::darkGray );
    c.setAlpha( 50 );
    m_brush = QBrush( c );

    m_pen = QPen();
    m_pen.setColor( Qt::black );
    m_pen.setWidth( 2 );

    setZ( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_PLOT_RECT_ANNOTATION ) );

    m_textLabel = std::make_unique<QwtPlotMarker>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::setPen( const QPen& pen )
{
    m_pen = pen;
    itemChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QPen& RiuQwtPlotRectAnnotation::pen() const
{
    return m_pen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::setBrush( const QBrush& brush )
{
    m_brush = brush;
    itemChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QBrush& RiuQwtPlotRectAnnotation::brush() const
{
    return m_brush;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::setInterval( double minX, double maxX, double minY, double maxY )
{
    m_intervalX.setMinValue( minX );
    m_intervalX.setMaxValue( maxX );
    m_intervalY.setMinValue( minY );
    m_intervalY.setMaxValue( maxY );

    itemChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::draw( QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect ) const
{
    if ( !m_intervalX.isValid() || !m_intervalY.isValid() ) return;

    QPen pen = m_pen;
    pen.setCapStyle( Qt::FlatCap );

    double x1 = xMap.transform( m_intervalX.minValue() );
    double x2 = xMap.transform( m_intervalX.maxValue() );

    double y1 = yMap.transform( m_intervalY.minValue() );
    double y2 = yMap.transform( m_intervalY.maxValue() );

    const bool doAlign = QwtPainter::roundingAlignment( painter );
    if ( doAlign )
    {
        x1 = qRound( x1 );
        x2 = qRound( x2 );

        y1 = qRound( y1 );
        y2 = qRound( y2 );
    }

    QRectF r( x1, y1, x2 - x1, y2 - y1 );
    r = r.normalized();

    if ( m_brush.style() != Qt::NoBrush && x1 != x2 && y1 != y2 )
    {
        QwtPainter::fillRect( painter, r, m_brush );
    }

    if ( m_pen.style() != Qt::NoPen )
    {
        painter->setPen( m_pen );

        QwtPainter::drawRect( painter, r );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRectF RiuQwtPlotRectAnnotation::boundingRect() const
{
    QRectF br = QwtPlotItem::boundingRect();

    if ( m_intervalX.isValid() && m_intervalY.isValid() )
    {
        br.setTop( m_intervalY.minValue() );
        br.setBottom( m_intervalY.maxValue() );
        br.setLeft( m_intervalX.minValue() );
        br.setRight( m_intervalX.maxValue() );
    }

    return br;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotRectAnnotation::rtti() const
{
    return 5001;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::setText( const QString& text )
{
    QwtText label( text );
    auto    textColor = RiuGuiTheme::getColorByVariableName( "textColor" );
    label.setColor( textColor );
    label.setRenderFlags( Qt::AlignLeft );

    m_textLabel->setLabel( label );
    m_textLabel->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );

    m_textLabel->setLineStyle( QwtPlotMarker::NoLine );
    m_textLabel->setSymbol( new QwtSymbol( QwtSymbol::NoSymbol ) );

    m_textLabel->setValue( m_intervalX.minValue(), m_intervalY.maxValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::attach( QwtPlot* plot )
{
    QwtPlotItem::attach( plot );
    m_textLabel->attach( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::detach()
{
    QwtPlotItem::detach();
    m_textLabel->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotRectAnnotation::setVisible( bool isVisible )
{
    QwtPlotItem::setVisible( isVisible );
    m_textLabel->setVisible( isVisible );
}
