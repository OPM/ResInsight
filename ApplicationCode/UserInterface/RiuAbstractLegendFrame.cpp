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
#include "RiuAbstractLegendFrame.h"

#include <QPaintEvent>
#include <QPainter>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuAbstractLegendFrame::RiuAbstractLegendFrame( QWidget* parent, const QString& title )
    : RiuAbstractOverlayContentFrame( parent )
    , m_title( title )
{
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuAbstractLegendFrame::sizeHint() const
{
    LayoutInfo layout( QSize( 200, 200 ) ); // Use default size
    layoutInfo( &layout );

    QFontMetrics fontMetrics( this->font() );
    QStringList  titleLines = m_title.split( "\n", QString::SkipEmptyParts );

    int preferredContentHeight = titleLines.size() * layout.lineSpacing + labelCount() * layout.lineSpacing;
    int preferredHeight        = preferredContentHeight + layout.margins.top() + layout.margins.bottom();

    int maxTickTextWidth = 0;
    for ( int i = 0; i < labelCount(); ++i )
    {
        QString valueString = label( i );
        int     textWidth   = fontMetrics.boundingRect( valueString ).width();
        maxTickTextWidth    = std::max( maxTickTextWidth, textWidth );
    }

    int preferredWidth = layout.tickEndX + layout.margins.left() + layout.margins.right() + layout.tickTextLeadSpace +
                         maxTickTextWidth;

    preferredWidth = std::max( preferredWidth, fontMetrics.boundingRect( m_title ).width() );
    preferredWidth = std::min( preferredWidth, 400 );

    return QSize( preferredWidth, preferredHeight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuAbstractLegendFrame::minimumSizeHint() const
{
    LayoutInfo layout( QSize( 200, 200 ) ); // Use default size
    layoutInfo( &layout );

    QFontMetrics fontMetrics( this->font() );
    QStringList  titleLines = m_title.split( "\n", QString::SkipEmptyParts );

    int preferredContentHeight = titleLines.size() * layout.lineSpacing + 2 * layout.lineSpacing;
    int preferredHeight        = preferredContentHeight + layout.margins.top() + layout.margins.bottom();

    int firstTextWidth   = fontMetrics.boundingRect( label( 0 ) ).width();
    int lastTextWidth    = fontMetrics.boundingRect( label( labelCount() - 1 ) ).width();
    int maxTickTextWidth = std::max( firstTextWidth, lastTextWidth );

    int preferredWidth = layout.tickEndX + layout.margins.left() + layout.margins.right() + layout.tickTextLeadSpace +
                         maxTickTextWidth;

    preferredWidth = std::max( preferredWidth, fontMetrics.boundingRect( m_title ).width() );
    preferredWidth = std::min( preferredWidth, 400 );

    return QSize( preferredWidth, preferredHeight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuAbstractLegendFrame::renderTo( QPainter* painter, const QRect& targetRect )
{
    LayoutInfo layout( QSize( targetRect.width(), targetRect.height() ) );
    layoutInfo( &layout );

    painter->save();
    painter->translate( targetRect.topLeft() );
    QPoint titlePos( layout.margins.left(), layout.margins.top() + layout.lineSpacing / 2 );
    painter->drawText( titlePos, m_title );

    QStringList titleLines = m_title.split( "\n", QString::SkipEmptyParts );

    std::vector<std::pair<QPoint, QString>> visibleTickLabels = visibleLabels( layout );
    for ( auto tickLabel : visibleTickLabels )
    {
        painter->drawText( tickLabel.first, tickLabel.second );
    }

    // Render color bar as one colored rectangle per color interval
    for ( int i = 0; i < rectCount(); ++i )
    {
        renderRect( painter, layout, i );
    }
    painter->drawRect( layout.colorBarRect );
    // painter->drawLine( QPointF( layout.tickMidX, layout.tickYPixelPos->get( i ) ),
    //         QPointF( layout.tickMidX, layout.tickYPixelPos->get( i + 1 ) ) );

    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuAbstractLegendFrame::paintEvent( QPaintEvent* e )
{
    QPainter painter( this );
    renderTo( &painter, e->rect() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QPoint, QString>> RiuAbstractLegendFrame::visibleLabels( const LayoutInfo& layout ) const
{
    const int textX = layout.tickEndX + layout.tickTextLeadSpace;

    const double overlapTolerance = 1.1 * layout.charHeight;
    int          lastVisibleTextY = 0;

    std::vector<std::pair<QPoint, QString>> visibleTickLabels;

    int numLabels = labelCount();
    for ( int i = 0; i < numLabels; i++ )
    {
        int textY = labelPixelPosY( layout, i );

        // Always draw first and last tick label. For all others, skip drawing if text ends up
        // on top of the previous label.
        if ( i != 0 && i != ( numLabels - 1 ) )
        {
            if ( std::fabs( static_cast<double>( textY - lastVisibleTextY ) ) < overlapTolerance )
            {
                continue;
            }
            // Make sure it does not overlap the last tick as well

            int lastTickY = layout.colorBarRect.bottom();

            if ( std::fabs( static_cast<double>( lastTickY - textY ) ) < overlapTolerance )
            {
                continue;
            }
        }

        QString valueString = label( numLabels - i - 1 );
        QPoint  pos( textX, textY );

        lastVisibleTextY = textY;
        visibleTickLabels.push_back( {pos, valueString} );
    }
    return visibleTickLabels;
}
