/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RiuQssSyntaxHighlighter.h"

#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QssSyntaxHighligter::QssSyntaxHighligter( QTextDocument* parent )
    : QSyntaxHighlighter( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QssSyntaxHighligter::highlightBlock( const QString& text )
{
    QTextCharFormat variableFormat;
    variableFormat.setFontWeight( QFont::Bold );
    variableFormat.setForeground( QColor( "#e20a48" ) );

    QTextCharFormat commentFormat;
    commentFormat.setFontWeight( QFont::Normal );
    commentFormat.setForeground( QColor( "#4e9041" ) );

    QTextCharFormat classFormat;
    classFormat.setFontWeight( QFont::Bold );
    classFormat.setForeground( QColor( "#fc8f00" ) );

    QTextCharFormat handleFormat;
    handleFormat.setFontWeight( QFont::Bold );
    handleFormat.setForeground( QColor( "#e20a48" ) );

    QTextCharFormat modifierFormat;
    modifierFormat.setFontWeight( QFont::Normal );
    modifierFormat.setForeground( QColor( "#e20a48" ) );

    // Block states:
    // -1 - none
    // 1 - inside class
    // 2 - inside function
    // 3 - inside multiline comment

    QRegularExpression classStartExpression    = QRegularExpression( QString( "\\{" ) );
    QRegularExpression classEndExpression      = QRegularExpression( QString( "\\}" ) );
    QRegularExpression functionStartExpression = QRegularExpression( QString( "\\(" ) );
    QRegularExpression functionEndExpression   = QRegularExpression( QString( "\\)" ) );
    QRegularExpression commentStartExpression  = QRegularExpression( QString( "/\\*" ) );
    QRegularExpression commentEndExpression    = QRegularExpression( QString( "\\*/" ) );

    setCurrentBlockState( -1 );

    auto parseClass = [this, text]( int startIndex, int length ) {
        QTextCharFormat propertyFormat;
        propertyFormat.setFontWeight( QFont::Bold );

        QTextCharFormat valueFormat;
        valueFormat.setFontWeight( QFont::Normal );
        valueFormat.setForeground( QColor( "#0d98ce" ) );

        QTextCharFormat functionFormat;
        functionFormat.setFontWeight( QFont::Bold );
        functionFormat.setForeground( QColor( "#dd1bc1" ) );

        QRegularExpression expression =
            QRegularExpression( "\\s*([a-zA-Z0-9_-]+)[ \\t]*:\\s*([a-zA-Z0-9_-]+\\()?([\\$#\\[\\]:\\.,a-zA-Z0-9-_ "
                                "]+)(\\))?[ \\t]*;" );
        QRegularExpressionMatchIterator j = expression.globalMatch( text );
        while ( j.hasNext() )
        {
            QRegularExpressionMatch matchClass = j.next();
            setFormat( startIndex + matchClass.capturedStart( 1 ), matchClass.capturedLength( 1 ), propertyFormat );
            setFormat( startIndex + matchClass.capturedStart( 2 ), matchClass.capturedLength( 2 ), functionFormat );
            setFormat( startIndex + matchClass.capturedStart( 3 ), matchClass.capturedLength( 3 ), valueFormat );
            setFormat( startIndex + matchClass.capturedStart( 4 ), matchClass.capturedLength( 4 ), functionFormat );
        }
    };

    if ( previousBlockState() == -1 )
    {
        QRegularExpression expression = QRegularExpression( "([a-zA-z0-9_-]+)(::[a-zA-Z0-9_-]+)?(:[a-zA-Z0-9_-]+)?" );
        QRegularExpressionMatchIterator i = expression.globalMatch( text );
        while ( i.hasNext() )
        {
            QRegularExpressionMatch matchClass = i.next();
            setFormat( matchClass.capturedStart( 1 ), matchClass.capturedLength( 1 ), classFormat );
            setFormat( matchClass.capturedStart( 2 ), matchClass.capturedLength( 2 ), handleFormat );
            setFormat( matchClass.capturedStart( 3 ), matchClass.capturedLength( 3 ), modifierFormat );
        }

        int startIndex = text.indexOf( classStartExpression );
        while ( startIndex >= 0 )
        {
            QRegularExpressionMatch match       = classEndExpression.match( text, startIndex );
            int                     endIndex    = match.capturedStart();
            int                     classLength = 0;
            if ( endIndex == -1 )
            {
                setCurrentBlockState( 1 );
                classLength = text.length() - startIndex;
            }
            else
            {
                classLength = endIndex - startIndex + match.capturedLength();
            }
            parseClass( startIndex, classLength );
            startIndex = text.indexOf( classStartExpression, startIndex + classLength );
        }
    }
    if ( previousBlockState() == 1 )
    {
        int startIndex = 0;
        while ( startIndex >= 0 )
        {
            QRegularExpressionMatch match       = classEndExpression.match( text, startIndex );
            int                     endIndex    = match.capturedStart();
            int                     classLength = 0;
            if ( endIndex == -1 )
            {
                setCurrentBlockState( 1 );
                classLength = text.length() - startIndex;
            }
            else
            {
                classLength = endIndex - startIndex + match.capturedLength();
            }
            parseClass( startIndex, classLength );
            startIndex = text.indexOf( classStartExpression, startIndex + classLength );
        }
    }

    QRegularExpression              expression( "\\$[a-zA-z0-9_]+" );
    QRegularExpressionMatchIterator i = expression.globalMatch( text );
    while ( i.hasNext() )
    {
        QRegularExpressionMatch match = i.next();
        setFormat( match.capturedStart(), match.capturedLength(), variableFormat );
    }

    expression = QRegularExpression( "\\/\\/(.*)" );
    i          = expression.globalMatch( text );
    while ( i.hasNext() )
    {
        QRegularExpressionMatch match = i.next();
        setFormat( match.capturedStart(), match.capturedLength(), commentFormat );
    }

    int startIndex = 0;
    if ( previousBlockState() != 2 ) startIndex = text.indexOf( commentStartExpression );
    while ( startIndex >= 0 )
    {
        QRegularExpressionMatch match         = commentEndExpression.match( text, startIndex );
        int                     endIndex      = match.capturedStart();
        int                     commentLength = 0;
        if ( endIndex == -1 )
        {
            setCurrentBlockState( 2 );
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat( startIndex, commentLength, commentFormat );
        startIndex = text.indexOf( commentStartExpression, startIndex + commentLength );
    }
}
