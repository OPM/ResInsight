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

#include "RiuQtChartsToolTip.h"

#include <QtCharts/QChart>
#include <QtGui/QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsToolTip::RiuQtChartsToolTip( QChart* chart, QAbstractSeries* series )
    : QGraphicsItem( chart )
    , m_chart( chart )
    , m_series( series )
    , m_radius( 10 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRectF RiuQtChartsToolTip::boundingRect() const
{
    QPointF anchor = mapFromParent( m_chart->mapToPosition( m_anchor, m_series ) );
    QRectF  rect   = m_rect.united( m_textRect );
    rect.setLeft( std::min( rect.left(), anchor.x() ) );
    rect.setRight( std::max( rect.right(), anchor.x() ) );
    rect.setTop( std::min( rect.top(), anchor.y() ) );
    rect.setBottom( std::max( rect.bottom(), anchor.y() ) );
    return rect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsToolTip::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED( option )
    Q_UNUSED( widget )
    QPainterPath path;

    QPointF anchor = mapFromParent( m_chart->mapToPosition( m_anchor, m_series ) );

    path.addEllipse( anchor, m_radius, m_radius );
    path = path.simplified();

    painter->setPen( QPen( Qt::black ) );

    painter->drawPath( path );
    painter->drawText( m_textRect, m_text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsToolTip::setSeries( QAbstractSeries* series )
{
    m_series = series;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsToolTip::setText( const QString& text )
{
    m_text = text;
    QFontMetrics metrics( m_font );
    m_textRect = metrics.boundingRect( QRect( 0, 0, 150, 150 ), Qt::AlignLeft, m_text );
    m_textRect.translate( m_radius, 0 );
    prepareGeometryChange();

    m_rect.setRect( -m_radius, -m_radius, m_radius, m_radius );
    m_rect.moveCenter( QPoint( m_radius, m_radius ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsToolTip::setAnchor( QPointF point )
{
    m_anchor = point;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsToolTip::updateGeometry()
{
    prepareGeometryChange();
    setPos( m_chart->mapToPosition( m_anchor, m_series ) );
}
