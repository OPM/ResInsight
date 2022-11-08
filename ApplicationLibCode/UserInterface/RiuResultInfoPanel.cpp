/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiuResultInfoPanel.h"

#include <QTextEdit>
#include <QVBoxLayout>

//==================================================================================================
///
/// \class RiuResultInfoPanel
/// \ingroup ResInsight
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuResultInfoPanel::RiuResultInfoPanel( QWidget* parent )
    : QWidget( parent )
{
    m_textEdit = new QTextEdit( this );
    m_textEdit->setReadOnly( true );
    m_textEdit->setLineWrapMode( QTextEdit::NoWrap );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget( m_textEdit );

    // Use a nonexisting font family to trigger the use of QFont::Monospace
    // https://forum.qt.io/topic/35999/solved-qplaintextedit-how-to-change-the-font-to-be-monospaced/7
    QFont font( "does not exist" );
    font.setStyleHint( QFont::Monospace );
    m_textEdit->setFont( font );

    layout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( layout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultInfoPanel::setInfo( const QString& info )
{
    QString tmp( info );

    convertStringToHTML( &tmp );

    m_textEdit->setText( info );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultInfoPanel::convertStringToHTML( QString* str )
{
    str->replace( "\n", "<br>" );
    str->replace( " ", "&nbsp;" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuResultInfoPanel::sizeHint() const
{
    // As small as possible for now
    return QSize( 20, 20 );
}
