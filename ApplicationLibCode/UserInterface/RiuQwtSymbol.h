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
        LabelBelowSymbol,
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
        SYMBOL_DOWN_TRIANGLE,
        SYMBOL_CROSS,
        SYMBOL_XCROSS,
        SYMBOL_LEFT_ALIGNED_TRIANGLE, // Aligned so pin point is at lower right corner
        SYMBOL_RIGHT_ALIGNED_TRIANGLE, // Aligned so pin point is at lower left corner
        SYMBOL_LEFT_ANGLED_TRIANGLE,
        SYMBOL_RIGHT_ANGLED_TRIANGLE,
        SYMBOL_UP_TRIANGLE,
        SYMBOL_STAR1,
        SYMBOL_STAR2,
        SYMBOL_HEXAGON,
        SYMBOL_LEFT_TRIANGLE,
        SYMBOL_RIGHT_TRIANGLE
    };

    RiuQwtSymbol( PointSymbolEnum riuStyle,
                  const QString&  label,
                  LabelPosition   labelPosition   = LabelAboveSymbol,
                  int             labelFontSizePt = 8 );
    void    renderSymbols( QPainter* painter, const QPointF* points, int numPoints ) const override;
    void    renderSymbolLabel( QPainter* painter, const QPointF& position, const QString& label ) const;
    QString globalLabel() const;

    void setGlobalLabel( const QString& label );

    void setLabelPosition( LabelPosition labelPosition );

    static PointSymbolEnum cycledSymbolStyle( int indexLevel1, int indexLevel2 );
    static PointSymbolEnum cycledSymbolStyle( int indexLevel );

    static bool isFilledSymbol( PointSymbolEnum symbol );

private:
    QRect labelBoundingRect( const QPainter* painter, const QRect& symbolRect, const QString& label ) const;

private:
    QString       m_globalLabel;
    int           m_labelFontSizePx;
    LabelPosition m_labelPosition;
};
