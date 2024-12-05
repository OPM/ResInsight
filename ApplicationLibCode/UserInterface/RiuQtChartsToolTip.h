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

#pragma once

#include <QtCharts/QChart>
#include <QtCharts/QChartGlobal>
#include <QtGui/QFont>
#include <QtWidgets/QGraphicsItem>

class RiuQtChartsToolTip : public QGraphicsItem
{
public:
    RiuQtChartsToolTip( QChart* parent, QAbstractSeries* series );

    void setSeries( QAbstractSeries* series );
    void setText( const QString& text );
    void setAnchor( QPointF point );
    void updateGeometry();

    QRectF boundingRect() const override;
    void   paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

private:
    QString          m_text;
    QRectF           m_textRect;
    QRectF           m_rect;
    QPointF          m_anchor;
    QFont            m_font;
    int              m_radius;
    QChart*          m_chart;
    QAbstractSeries* m_series;
};
