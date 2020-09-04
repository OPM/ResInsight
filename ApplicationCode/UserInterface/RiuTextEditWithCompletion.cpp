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

#include "RiuTextEditWithCompletion.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QKeyEvent>
#include <QModelIndex>
#include <QScrollBar>
#include <QtDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TextEditWithCompletion::TextEditWithCompletion( QWidget* parent /*= nullptr */ )
    : QTextEdit( parent )
{
    m_completer = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TextEditWithCompletion::~TextEditWithCompletion()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TextEditWithCompletion::setCompleter( QCompleter* completer )
{
    if ( m_completer ) m_completer->disconnect( this );

    m_completer = completer;

    if ( !m_completer ) return;

    m_completer->setWidget( this );
    m_completer->setCompletionMode( QCompleter::PopupCompletion );
    m_completer->setCaseSensitivity( Qt::CaseInsensitive );
    QObject::connect( m_completer,
                      QOverload<const QString&>::of( &QCompleter::activated ),
                      this,
                      &TextEditWithCompletion::insertCompletion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QCompleter* TextEditWithCompletion::completer()
{
    return m_completer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TextEditWithCompletion::keyPressEvent( QKeyEvent* e )
{
    if ( m_completer && m_completer->popup()->isVisible() )
    {
        // The following keys are forwarded by the completer to the widget
        switch ( e->key() )
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
            default:
                break;
        }
    }

    const bool isShortcut = ( e->modifiers().testFlag( Qt::ControlModifier ) && e->key() == Qt::Key_E ); // CTRL+E
    if ( !m_completer || !isShortcut ) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent( e );

    const bool ctrlOrShift = e->modifiers().testFlag( Qt::ControlModifier ) ||
                             e->modifiers().testFlag( Qt::ShiftModifier );
    if ( !m_completer || ( ctrlOrShift && e->text().isEmpty() ) ) return;

    static QString eow( "~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=" ); // end of word
    const bool     hasModifier      = ( e->modifiers() != Qt::NoModifier ) && !ctrlOrShift;
    QString        completionPrefix = textUnderCursor();

    if ( !isShortcut &&
         ( hasModifier || e->text().isEmpty() || completionPrefix.length() < 3 || eow.contains( e->text().right( 1 ) ) ) )
    {
        m_completer->popup()->hide();
        return;
    }

    if ( completionPrefix != m_completer->completionPrefix() )
    {
        m_completer->setCompletionPrefix( completionPrefix );
        m_completer->popup()->setCurrentIndex( m_completer->completionModel()->index( 0, 0 ) );
    }
    QRect cr = cursorRect();
    cr.setWidth( m_completer->popup()->sizeHintForColumn( 0 ) +
                 m_completer->popup()->verticalScrollBar()->sizeHint().width() );
    m_completer->complete( cr ); // popup it up!
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TextEditWithCompletion::focusInEvent( QFocusEvent* e )
{
    if ( m_completer ) m_completer->setWidget( this );
    QTextEdit::focusInEvent( e );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TextEditWithCompletion::insertCompletion( const QString& completion )
{
    if ( m_completer->widget() != this ) return;
    QTextCursor tc    = textCursor();
    int         extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition( QTextCursor::Left );
    tc.movePosition( QTextCursor::EndOfWord );
    tc.insertText( completion.right( extra ) );
    setTextCursor( tc );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString TextEditWithCompletion::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select( QTextCursor::WordUnderCursor );
    return tc.selectedText();
}
