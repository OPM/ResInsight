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
#include "RiuPlotCurveSymbolImageCreator.h"

#include "cvfAssert.h"

#include <QPainter>
#include <QPainterPath>
#include <QScatterSeries>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotCurveSymbol::RiuQtChartsPlotCurveSymbol( PointSymbolEnum riuStyle, const QString& label, LabelPosition labelPosition, int labelFontSizePt )
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
void RiuQtChartsPlotCurveSymbol::setPixmap( const QPixmap& pixmap )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurveSymbol::applyToScatterSeries( QScatterSeries* series ) const
{
    if ( m_style == PointSymbolEnum::SYMBOL_NONE )
    {
        series->hide();
        return;
    }

    series->show();

    QImage image = RiuPlotCurveSymbolImageCreator::createSymbolImage( m_style, QSize( m_size, m_size ), m_pen, m_color );
    if ( !image.isNull() )
    {
        setImageBrush( series, image );
    }

    series->setMarkerSize( m_size );
    series->setColor( m_color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotCurveSymbol::setImageBrush( QScatterSeries* series, const QImage& image ) const
{
    series->setMarkerShape( QScatterSeries::MarkerShapeRectangle );
    series->setBrush( image );
    series->setPen( QColor( Qt::transparent ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRect RiuQtChartsPlotCurveSymbol::boundingRect() const
{
    return QRect( 0, 0, m_size, m_size );
}
