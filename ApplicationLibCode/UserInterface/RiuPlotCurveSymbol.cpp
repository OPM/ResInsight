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

#include "RiuPlotCurveSymbol.h"

#include "cafFontTools.h"
#include "cvfAssert.h"

#include <QPainter>
#include <QRect>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::RiuPlotCurveSymbol( PointSymbolEnum riuStyle, const QString& label, LabelPosition labelPosition, int labelFontSize )
    : m_style( riuStyle )
    , m_globalLabel( label )
    , m_labelPosition( labelPosition )
    , m_labelFontSize( labelFontSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuPlotCurveSymbol::globalLabel() const
{
    return m_globalLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurveSymbol::setGlobalLabel( const QString& label )
{
    m_globalLabel = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurveSymbol::setLabelPosition( LabelPosition labelPosition )
{
    m_labelPosition = labelPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurveSymbol::setLabelFontSize( int labelFontSize )
{
    m_labelFontSize = labelFontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::PointSymbolEnum RiuPlotCurveSymbol::cycledSymbolStyle( int indexLevel1, int indexLevel2 )
{
    std::vector<std::vector<PointSymbolEnum>> categorisedStyles = {
        { SYMBOL_ELLIPSE, SYMBOL_RECT, SYMBOL_DIAMOND },
        { SYMBOL_DOWN_TRIANGLE, SYMBOL_UP_TRIANGLE },
        { SYMBOL_LEFT_TRIANGLE, SYMBOL_RIGHT_TRIANGLE },
        { SYMBOL_CROSS, SYMBOL_XCROSS },
        { SYMBOL_STAR1, SYMBOL_STAR2 },
    };

    int level1Category = indexLevel1 % int( categorisedStyles.size() );
    int level2Category = indexLevel2 % int( categorisedStyles[level1Category].size() );

    return categorisedStyles[level1Category][level2Category];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::PointSymbolEnum RiuPlotCurveSymbol::cycledSymbolStyle( int indexLevel )
{
    std::vector<PointSymbolEnum> contrastingSymbols = { SYMBOL_ELLIPSE,
                                                        SYMBOL_CROSS,
                                                        SYMBOL_RECT,
                                                        SYMBOL_DOWN_TRIANGLE,
                                                        SYMBOL_UP_TRIANGLE,
                                                        SYMBOL_LEFT_TRIANGLE,
                                                        SYMBOL_RIGHT_TRIANGLE,
                                                        SYMBOL_STAR2,
                                                        SYMBOL_DIAMOND,
                                                        SYMBOL_STAR1 };

    return contrastingSymbols[indexLevel % (int)contrastingSymbols.size()];
}

//--------------------------------------------------------------------------------------------------
/// Is this a symbol with an interior and a border? If false, it is just lines.
//--------------------------------------------------------------------------------------------------
bool RiuPlotCurveSymbol::isFilledSymbol( PointSymbolEnum symbol )
{
    return symbol != SYMBOL_NONE && symbol != SYMBOL_CROSS && symbol != SYMBOL_XCROSS && symbol != SYMBOL_STAR1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRect RiuPlotCurveSymbol::labelBoundingRect( const QPainter* painter, const QRect& symbolRect, const QString& label ) const
{
    CVF_ASSERT( painter );

    QPoint symbolPosition = symbolRect.topLeft();

    int symbolWidth  = symbolRect.width();
    int symbolHeight = symbolRect.height();

    int labelWidth  = painter->fontMetrics().horizontalAdvance( label );
    int labelHeight = painter->fontMetrics().height();

    QPoint labelPosition;
    if ( m_labelPosition == LabelAboveSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() - labelWidth / 2, symbolPosition.y() - 5 );
    }
    else if ( m_labelPosition == LabelBelowSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() - labelWidth / 2, symbolPosition.y() + symbolHeight + 5 );
    }
    else if ( m_labelPosition == LabelLeftOfSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() - labelWidth - symbolWidth, symbolPosition.y() );
    }
    else if ( m_labelPosition == LabelRightOfSymbol )
    {
        labelPosition = QPoint( symbolPosition.x() + symbolWidth + 3, symbolPosition.y() );
    }
    return QRect( labelPosition.x(), labelPosition.y(), labelWidth, labelHeight );
}
