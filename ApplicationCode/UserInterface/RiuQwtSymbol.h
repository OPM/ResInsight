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

#include "qwt_symbol.h"
#include <QString>

//--------------------------------------------------------------------------------------------------
/// This class overrides renderSymbols to draw symbols and labels.
/// The label is only visible in the legend, while it is clipped in the plot.
/// Therefore the method RiuQwtPlotCurve::drawSymbols also draw labels to have labels
/// in the plot as well.
//--------------------------------------------------------------------------------------------------
class RiuQwtSymbol : public QwtSymbol
{
public:
    enum LabelPosition
    {
        LabelAboveSymbol,
        LabelLeftOfSymbol,
        LabelRightOfSymbol
    };
    enum PointSymbolEnum
    {
        SYMBOL_NONE,
        SYMBOL_ELLIPSE,
        SYMBOL_RECT,
        SYMBOL_DIAMOND,
        SYMBOL_TRIANGLE,
        SYMBOL_CROSS,
        SYMBOL_XCROSS,
        SYMBOL_DOWN_TRIANGLE,
        SYMBOL_LEFT_TRIANGLE,
        SYMBOL_RIGHT_TRIANGLE,
        SYMBOL_LEFT_ANGLED_TRIANGLE,
        SYMBOL_RIGHT_ANGLED_TRIANGLE
    };

    RiuQwtSymbol(PointSymbolEnum riuStyle, const QString& label, LabelPosition labelPosition = LabelAboveSymbol);

    virtual void renderSymbols(QPainter *painter, const QPointF *points, int numPoints) const override;
    void         renderSymbolLabel(QPainter *painter, const QPointF& position) const;
    QString label() const { return m_label; }

    void    setLabelPosition(LabelPosition labelPosition);
    virtual QRect boundingRect() const override;

private:
    QRect   labelBoundingRect(const QRect& symbolRect) const;

private:
    QString       m_label;
    LabelPosition m_labelPosition;
};


