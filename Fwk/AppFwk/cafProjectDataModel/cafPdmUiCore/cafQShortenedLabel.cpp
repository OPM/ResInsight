//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#include "cafQShortenedLabel.h"

#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QResizeEvent>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QShortenedLabel::QShortenedLabel( QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/ )
    : QLabel( parent, f )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QShortenedLabel::setText( const QString& text )
{
    bool textHasChanged = m_fullLengthText != text;

    m_fullLengthText = text;
    setDisplayText( text );

    if ( textHasChanged )
    {
        adjustSize();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString QShortenedLabel::fullText() const
{
    return m_fullLengthText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize QShortenedLabel::minimumSizeHint() const
{
    const int minimumNumberOfCharacters = 10;

    QFontMetrics fontMetrics( QApplication::font() );
    QString      fullLabelText = fullText();
    QString      shortenedText = fullLabelText.left( minimumNumberOfCharacters );
    int          minimumWidth  = fontMetrics.horizontalAdvance( shortenedText );

    if ( !fullLabelText.isEmpty() )
    {
        int maxLineWidth      = 0;
        int maxFirstWordWidth = 0;

        QStringList labelLines = fullLabelText.split( "\n" );
        for ( QString line : labelLines )
        {
            int lineWidth     = fontMetrics.horizontalAdvance( line );
            maxLineWidth      = std::max( maxLineWidth, lineWidth );
            QStringList words = line.split( " " );
            if ( !words.empty() )
            {
                int wordWidth     = fontMetrics.horizontalAdvance( words.front() + "..." );
                maxFirstWordWidth = std::max( maxFirstWordWidth, wordWidth );
            }
        }
        int minimumTextWidth = maxFirstWordWidth;

        // Prefer the max line width if there's just one character between them
        if ( maxFirstWordWidth + fontMetrics.maxWidth() >= maxLineWidth ) minimumTextWidth = maxLineWidth;

        minimumWidth = std::max( minimumWidth, minimumTextWidth );
    }
    QSize minimumSize = QLabel::minimumSizeHint();
    minimumSize.setWidth( minimumWidth );
    return minimumSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize QShortenedLabel::sizeHint() const
{
    QFontMetrics fontMetrics( QApplication::font() );
    QString      labelText = fullText();

    QStringList labelLines   = labelText.split( "\n" );
    int         maxLineWidth = 0;
    for ( const QString& line : labelLines )
    {
        maxLineWidth = std::max( maxLineWidth, fontMetrics.horizontalAdvance( line ) );
    }

    // increase size hint with a few pixels to avoid linux gui issues
    return QSize( maxLineWidth + 3, QLabel::sizeHint().height() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QShortenedLabel::resizeEvent( QResizeEvent* event )
{
    QSize paintSize = event->size();
    resizeText( paintSize );
    QLabel::resizeEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QShortenedLabel::configureContextMenu( const QString& pythonParameterName )
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    auto createContextMenu = [pythonParameterName]( const QPoint& pos )
    {
        QMenu    menu;
        QAction* action = menu.addAction( "Copy Python Parameter Name" );
        action->setIcon( QIcon( ":/caf/duplicate.svg" ) );

        connect( action,
                 &QAction::triggered,
                 [pythonParameterName]()
                 {
                     if ( QClipboard* clipboard = QApplication::clipboard() )
                     {
                         clipboard->setText( pythonParameterName );
                     }
                 } );

        menu.exec( QCursor::pos() );
    };

    connect( this, &QLabel::customContextMenuRequested, this, createContextMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::QShortenedLabel::resizeText( QSize paintSize )
{
    QString      labelText = fullText();
    QFontMetrics fontMetrics( QApplication::font() );

    QStringList labelLines   = labelText.split( "\n" );
    int         maxLineWidth = 0;
    for ( const QString& line : labelLines )
    {
        maxLineWidth += fontMetrics.horizontalAdvance( line );
    }

    if ( maxLineWidth < paintSize.width() )
    {
        setDisplayText( labelText );
    }
    else
    {
        int limitWidth = std::max( minimumSizeHint().width(), paintSize.width() );

        QStringList elidedLines;
        for ( const QString& line : labelLines )
        {
            QString elidedLine = fontMetrics.elidedText( line, Qt::ElideRight, limitWidth );
            elidedLines.push_back( elidedLine );
        }
        setDisplayText( elidedLines.join( "\n" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QShortenedLabel::setDisplayText( const QString& displayText )
{
    QLabel::setText( displayText );
}
