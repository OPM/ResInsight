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

#include "RiaApplication.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"

#include <QPaintEvent>
#include <QPainter>
#include <QTextDocument>

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
    QRect        titleRect = fontMetrics.boundingRect( QRect( 0, 0, 200, 200 ), Qt::AlignLeft, m_title );

    int preferredContentHeight = titleRect.height() + labelCount() * layout.lineSpacing + 0.5 * layout.lineSpacing;
    int preferredHeight        = preferredContentHeight + layout.margins.top() + layout.margins.bottom();

    int titleWidth = titleRect.width() + layout.margins.left() + layout.margins.right();

    int maxTickTextWidth = 0;
    for ( int i = 0; i < labelCount(); ++i )
    {
        QString valueString = label( i );
        int     textWidth   = fontMetrics.boundingRect( valueString ).width();
        maxTickTextWidth    = std::max( maxTickTextWidth, textWidth );
    }

    int preferredWidth = layout.tickEndX + layout.margins.left() + layout.margins.right() + layout.tickTextLeadSpace +
                         maxTickTextWidth;

    preferredWidth = std::max( preferredWidth, titleWidth );
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
    QRect        titleRect = fontMetrics.boundingRect( QRect( 0, 0, 200, 200 ), Qt::AlignLeft, m_title );

    int preferredContentHeight = titleRect.height() + 2 * layout.lineSpacing + 0.5 * layout.lineSpacing;
    int preferredHeight        = preferredContentHeight + layout.margins.top() + layout.margins.bottom();
    int titleWidth             = titleRect.width() + layout.margins.left() + layout.margins.right();

    int firstTextWidth   = fontMetrics.boundingRect( label( 0 ) ).width();
    int lastTextWidth    = fontMetrics.boundingRect( label( labelCount() - 1 ) ).width();
    int maxTickTextWidth = std::max( firstTextWidth, lastTextWidth );

    int preferredWidth = layout.tickEndX + layout.margins.left() + layout.margins.right() + layout.tickTextLeadSpace +
                         maxTickTextWidth;

    preferredWidth = std::max( preferredWidth, titleWidth );
    preferredWidth = std::min( preferredWidth, 400 );

    return QSize( preferredWidth, preferredHeight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuAbstractLegendFrame::renderTo( QPainter* painter, const QRect& targetRect )
{
    QFont font = this->font();
    font.setPixelSize(
        caf::FontTools::pointSizeToPixelSize( RiaApplication::instance()->preferences()->defaultPlotFontSize() ) );
    this->setFont( font );

    LayoutInfo layout( QSize( targetRect.width(), targetRect.height() ) );
    layoutInfo( &layout );

    painter->save();
    painter->setFont( this->font() );
    painter->translate( targetRect.topLeft() );
    QPoint titlePos( layout.margins.left(), layout.margins.top() );
    {
        painter->save();
        painter->translate( QPoint( layout.margins.left(), layout.margins.top() ) );
        QTextDocument td;
        td.setDocumentMargin( 0.0 );
        td.setDefaultFont( this->font() );
        td.setPlainText( m_title );
        td.drawContents( painter );
        painter->restore();
    }

    std::vector<std::pair<QRect, QString>> visibleTickLabels = visibleLabels( layout );
    for ( auto tickLabel : visibleTickLabels )
    {
        painter->save();
        painter->translate( tickLabel.first.topLeft() );
        QTextDocument td;
        td.setDocumentMargin( 0.0 );
        td.setDefaultFont( this->font() );
        td.setPlainText( tickLabel.second );
        td.drawContents( painter );
        painter->restore();
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
std::vector<std::pair<QRect, QString>> RiuAbstractLegendFrame::visibleLabels( const LayoutInfo& layout ) const
{
    std::vector<std::pair<QRect, QString>> visibleTickLabels;

    QRect lastRect;

    int numLabels = labelCount();
    for ( int i = 0; i < numLabels; i++ )
    {
        QRect rect = labelRect( layout, i );

        if ( !lastRect.isEmpty() && rect.intersects( lastRect ) ) continue;

        QString valueString = label( i );
        lastRect            = rect;
        visibleTickLabels.push_back( {rect, valueString} );
    }
    return visibleTickLabels;
}
