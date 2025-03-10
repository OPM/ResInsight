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

#pragma once
#include "RiuPlotCurveSymbol.h"

#include "qwt_symbol.h"

#include <QString>

class QPainter;
class QPointF;
class QRect;

//--------------------------------------------------------------------------------------------------
/// This class overrides renderSymbols to draw symbols and labels.
/// The label is only visible in the legend, while it is clipped in the plot.
/// Therefore the method RiuQwtPlotCurve::drawSymbols also draw labels to have labels
/// in the plot as well.
//--------------------------------------------------------------------------------------------------
class RiuQwtSymbol : public RiuPlotCurveSymbol, public QwtSymbol
{
public:
    RiuQwtSymbol( RiuPlotCurveSymbol::PointSymbolEnum riuStyle,
                  const QString&                      label         = QString(),
                  LabelPosition                       labelPosition = RiuPlotCurveSymbol::LabelAboveSymbol,
                  int                                 labelFontSize = 8 );
    void renderSymbols( QPainter* painter, const QPointF* points, int numPoints ) const override;
    void renderSymbolLabel( QPainter* painter, const QPointF& position, const QString& label ) const;

    void setSize( int width, int height ) override;

    void setColor( const QColor& color ) override;

    void setPen( const QPen& pen ) override;

    void setPixmap( const QPixmap& pixmap ) override;

    QRect boundingRect() const override;
};
