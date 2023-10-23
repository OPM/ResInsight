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

#pragma once

#include "qwt_interval.h"
#include "qwt_plot_item.h"

#include <QBrush>
#include <QPen>

#include <memory>

class QRectF;
class QPainter;
class QwtPlotMarker;

//==================================================================================================
/// Rectangular plot item for annotation areas of a plot.
///
//==================================================================================================
class RiuQwtPlotRectAnnotation : public QwtPlotItem
{
public:
    RiuQwtPlotRectAnnotation();
    ~RiuQwtPlotRectAnnotation() override = default;

    void setInterval( double minX, double maxX, double minY, double maxY );

    void        setPen( const QPen& );
    const QPen& pen() const;

    void          setBrush( const QBrush& );
    const QBrush& brush() const;

    void setText( const QString& text );

    int    rtti() const override;
    void   draw( QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect ) const override;
    QRectF boundingRect() const override;

    void attach( QwtPlot* plot );
    void detach();
    void setVisible( bool isVisible ) override;

protected:
    QwtInterval m_intervalX;
    QwtInterval m_intervalY;
    QPen        m_pen;
    QBrush      m_brush;
    QString     m_text;

    std::unique_ptr<QwtPlotMarker> m_textLabel;
};
