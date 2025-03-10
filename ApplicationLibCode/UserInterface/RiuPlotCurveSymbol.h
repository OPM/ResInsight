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

#include <QString>

class QColor;
class QPen;
class QPainter;
class QRect;
class QPixmap;

//--------------------------------------------------------------------------------------------------
/// Interface for plot curve symbol
//--------------------------------------------------------------------------------------------------
class RiuPlotCurveSymbol
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
        SYMBOL_RIGHT_TRIANGLE,
        SYMBOL_DOWN_ALIGNED_TRIANGLE
    };

    RiuPlotCurveSymbol( PointSymbolEnum riuStyle, const QString& label, LabelPosition labelPosition = LabelAboveSymbol, int labelFontSize = 8 );

    QString globalLabel() const;

    void setGlobalLabel( const QString& label );

    void setLabelPosition( LabelPosition labelPosition );

    void setLabelFontSize( int labelFontSize );

    virtual void setPixmap( const QPixmap& pixmap ) = 0;

    virtual void setSize( int width, int height ) = 0;

    virtual void setColor( const QColor& color ) = 0;

    virtual void setPen( const QPen& pen ) = 0;

    virtual QRect boundingRect() const = 0;

    static PointSymbolEnum cycledSymbolStyle( int indexLevel1, int indexLevel2 );
    static PointSymbolEnum cycledSymbolStyle( int indexLevel );

    static bool isFilledSymbol( PointSymbolEnum symbol );

    QRect labelBoundingRect( const QPainter* painter, const QRect& symbolRect, const QString& label ) const;

protected:
    PointSymbolEnum m_style;
    QString         m_globalLabel;
    int             m_labelFontSize;
    LabelPosition   m_labelPosition;
};
