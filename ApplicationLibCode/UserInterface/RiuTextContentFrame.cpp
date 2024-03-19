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

#include "RiuTextContentFrame.h"

#include "RiaTextStringTools.h"

#include "RiaFontCache.h"
#include "RiaPreferences.h"

#include "RiuGuiTheme.h"

#include <QPaintEvent>
#include <QPainter>
#include <QTextDocument>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuTextContentFrame::RiuTextContentFrame( QWidget* parent, const QString& title, const QString& text, int fontPixelSize )
    : RiuAbstractOverlayContentFrame( parent )
    , m_title( title )
    , m_text( text )
    , m_fontPixelSize( fontPixelSize )
{
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );

    updateFontSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuTextContentFrame::sizeHint() const
{
    LayoutInfo layout( QSize( 200, 100 ) ); // Use default size
    layoutInfo( &layout );

    QFontMetrics fontMetrics( font() );
    QRect        titleRect = fontMetrics.boundingRect( QRect( 0, 0, 2000, 200 ), Qt::AlignLeft | Qt::TextWordWrap, m_title );
    QRect        textRect  = fontMetrics.boundingRect( QRect( 0, 0, 2000, 200 ), Qt::AlignLeft | Qt::TextWordWrap, m_text );

    int preferredContentHeight = titleRect.height() + layout.lineSpacing + textRect.height();
    int preferredHeight        = preferredContentHeight + layout.margins.top() + layout.margins.bottom();

    int preferredWidth = std::max( titleRect.width(), textRect.width() ) + layout.margins.left() + layout.margins.right();

    preferredWidth = std::min( preferredWidth, 2000 );

    return QSize( preferredWidth, preferredHeight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuTextContentFrame::minimumSizeHint() const
{
    LayoutInfo layout( QSize( 200, 100 ) ); // Use default size
    layoutInfo( &layout );

    QFont titleFont = font();
    titleFont.setBold( true );
    QFontMetrics fontMetrics( titleFont );
    QRect        titleRect = fontMetrics.boundingRect( QRect( 0, 0, 2000, 200 ), Qt::AlignLeft | Qt::TextWordWrap, m_title );
    fontMetrics            = QFontMetrics( font() );
    QRect textRect         = fontMetrics.boundingRect( QRect( 0, 0, 2000, 200 ), Qt::AlignLeft | Qt::TextWordWrap, m_text );

    int preferredContentHeight = titleRect.height() + layout.lineSpacing + textRect.height();
    int preferredHeight        = preferredContentHeight + layout.margins.top() + layout.margins.bottom();
    int preferredWidth         = std::max( titleRect.width(), textRect.width() ) + layout.margins.left() + layout.margins.right();

    preferredWidth = std::min( preferredWidth, 2000 );

    return QSize( preferredWidth, preferredHeight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextContentFrame::renderTo( QPainter* painter, const QRect& targetRect )
{
    updateFontSize();

    QColor textColor = RiuGuiTheme::getColorByVariableName( "textColor" );

    LayoutInfo layout( QSize( targetRect.width(), targetRect.height() ) );
    layoutInfo( &layout );

    painter->save();

    painter->setFont( font() );
    painter->translate( targetRect.topLeft() );

    {
        painter->save();
        painter->translate( QPoint( layout.margins.left(), layout.margins.top() ) );
        painter->setPen( QPen( textColor ) );
        QTextDocument td;
        td.setDocumentMargin( 0.0 );
        td.setDefaultFont( font() );
        QString formattedTitle = m_title;
        td.setHtml(
            QString( "<body><font color='%1' ><b>%2</b></font></body>" ).arg( textColor.name() ).arg( formattedTitle.replace( "\n", "<br />" ) ) );
        td.drawContents( painter );
        painter->restore();
    }

    {
        painter->save();
        painter->translate( QPoint( layout.margins.left(), layout.margins.top() + layout.lineSpacing * 2 ) );
        painter->setPen( QPen( textColor ) );

        QTextDocument td;
        td.setDocumentMargin( 0.0 );
        td.setDefaultFont( font() );
        QString formattedTitle = m_text;
        td.setHtml(
            QString( "<body><font color='%1'>%2</font></body>" ).arg( textColor.name() ).arg( formattedTitle.replace( "\n", "<br />" ) ) );
        td.drawContents( painter );

        painter->restore();
    }

    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextContentFrame::paintEvent( QPaintEvent* e )
{
    QPainter painter( this );
    renderTo( &painter, e->rect() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextContentFrame::layoutInfo( LayoutInfo* layout ) const
{
    QFontMetrics fontMetrics( font() );
    QStringList  titleLines = RiaTextStringTools::splitSkipEmptyParts( m_text, "\n" );

    layout->charHeight        = fontMetrics.height();
    layout->charAscent        = fontMetrics.ascent();
    layout->lineSpacing       = fontMetrics.lineSpacing();
    layout->margins           = QMargins( 8, 8, 8, 8 );
    layout->tickTextLeadSpace = 5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextContentFrame::updateFontSize()
{
    if ( m_fontPixelSize != -1 )
    {
        QFont font = this->font();
        font.setPixelSize( m_fontPixelSize );
        setFont( font );
    }
    else
    {
        RiuAbstractOverlayContentFrame::updateFontSize();
    }
}
