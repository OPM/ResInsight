/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiuScalarMapperLegendFrame.h"

#include "cvfString.h"
#include "cvfqtUtils.h"

#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuScalarMapperLegendFrame::RiuScalarMapperLegendFrame( QWidget*           parent,
                                                        const QString&     title,
                                                        cvf::ScalarMapper* scalarMapper )
    : RiuAbstractLegendFrame( parent, title )
    , m_scalarMapper( scalarMapper )
    , m_tickNumberPrecision( 4 )
    , m_numberFormat( RimRegularLegendConfig::AUTO )
{
    if ( m_scalarMapper.notNull() )
    {
        m_scalarMapper->majorTickValues( &m_tickValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuScalarMapperLegendFrame::~RiuScalarMapperLegendFrame() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuScalarMapperLegendFrame::setTickPrecision( int precision )
{
    m_tickNumberPrecision = precision;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuScalarMapperLegendFrame::setTickFormat( NumberFormat format )
{
    m_numberFormat = format;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuScalarMapperLegendFrame::layoutInfo( LayoutInfo* layout ) const
{
    QFontMetrics fontMetrics( this->font() );
    QStringList  titleLines = m_title.split( "\n", QString::SkipEmptyParts );

    layout->charHeight        = fontMetrics.height();
    layout->charAscent        = fontMetrics.ascent();
    layout->lineSpacing       = ( fontMetrics.lineSpacing() * 3 ) / 2;
    layout->margins           = QMargins( 8, 8, 8, 8 );
    layout->tickTextLeadSpace = 5;

    int colorBarWidth  = 25;
    int colorBarHeight = layout->overallLegendSize.height() - layout->margins.top() - layout->margins.bottom() -
                         titleLines.size() * layout->lineSpacing;

    int colorBarStartY = layout->margins.top() + titleLines.size() * layout->lineSpacing;

    layout->colorBarRect = QRect( layout->margins.left(), colorBarStartY, colorBarWidth, colorBarHeight );

    layout->tickStartX = layout->margins.left();
    layout->tickMidX   = layout->margins.left() + layout->colorBarRect.width();
    layout->tickEndX   = layout->tickMidX + 5;

    // Build array containing the pixel positions of all the ticks
    int numTicks = (int)m_tickValues.size();
    layout->tickYPixelPos.reserve( numTicks );

    int i;
    for ( i = 0; i < numTicks; i++ )
    {
        double t = 0.0;
        if ( m_scalarMapper.notNull() )
        {
            t = cvf::Math::clamp( m_scalarMapper->normalizedValue( m_tickValues[i] ), 0.0, 1.1 );
        }

        if ( i == 0 )
        {
            layout->tickYPixelPos.push_back( 0.0 );
        }
        else if ( i == numTicks - 1 )
        {
            layout->tickYPixelPos.push_back( layout->colorBarRect.height() );
        }
        else
        {
            layout->tickYPixelPos.push_back( t * layout->colorBarRect.height() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuScalarMapperLegendFrame::label( int index ) const
{
    double  tickValue = m_tickValues[index];
    QString valueString;
    switch ( m_numberFormat )
    {
        case RimRegularLegendConfig::FIXED:
            valueString = QString::number( tickValue, 'f', m_tickNumberPrecision );
            break;
        case RimRegularLegendConfig::SCIENTIFIC:
            valueString = QString::number( tickValue, 'e', m_tickNumberPrecision );
            break;
        default:
            valueString = QString::number( tickValue );
            break;
    }
    return valueString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuScalarMapperLegendFrame::labelCount() const
{
    return (int)m_tickValues.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuScalarMapperLegendFrame::rectCount() const
{
    return (int)m_tickValues.size() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuScalarMapperLegendFrame::renderRect( QPainter* painter, const LayoutInfo& layout, int rectIndex ) const
{
    int           rectIndexFromBottom = rectCount() - rectIndex - 1;
    cvf::Color3ub startColor          = m_scalarMapper->mapToColor( m_tickValues[rectIndexFromBottom] );
    cvf::Color3ub endColor            = m_scalarMapper->mapToColor( m_tickValues[rectIndexFromBottom + 1] );
    QColor        startQColor( startColor.r(), startColor.g(), startColor.b() );
    QColor        endQColor( endColor.r(), endColor.g(), endColor.b() );

    QRectF gradientRect( QPointF( layout.tickStartX,
                                  layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom] + 1 ),
                         QPointF( layout.tickStartX,
                                  layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom + 1] + 1 ) );

    QLinearGradient gradient( gradientRect.topLeft(), gradientRect.bottomRight() );
    gradient.setCoordinateMode( QGradient::LogicalMode );
    gradient.setColorAt( 0.0, startQColor );
    gradient.setColorAt( 1.0, endQColor );

    QRectF rect( QPointF( layout.tickStartX,
                          layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom] + 1 ),
                 QPointF( layout.tickMidX,
                          layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom + 1] + 1 ) );

    painter->fillRect( rect, QBrush( gradient ) );
    painter->drawLine( QPointF( layout.tickStartX,
                                layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom] + 1 ),
                       QPointF( layout.tickEndX,
                                layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom] + 1 ) );
    painter->drawLine( QPointF( layout.tickStartX,
                                layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom + 1] + 1 ),
                       QPointF( layout.tickEndX,
                                layout.colorBarRect.bottom() - layout.tickYPixelPos[rectIndexFromBottom + 1] + 1 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuScalarMapperLegendFrame::labelPixelPosY( const LayoutInfo& layout, int index ) const
{
    int indexFromBottom = labelCount() - index - 1;
    int offset          = layout.charAscent - static_cast<int>( std::ceil( layout.charHeight / 2.0 ) );
    return layout.colorBarRect.bottom() - layout.tickYPixelPos[indexFromBottom] + offset;
}
