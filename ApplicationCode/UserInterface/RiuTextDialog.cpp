/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
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

#include "RiuTextDialog.h"
#include "RiuTools.h"

#include "RiaQDateTimeTools.h"

#include "SummaryPlotCommands/RicAsciiExportSummaryPlotFeature.h"

#include "cafCmdFeature.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QMenu>
#include <QTabWidget>

#include <cvfAssert.h>

//--------------------------------------------------------------------------------------------------
///
///
///  RiuQPlainTextEdit
///
///
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::keyPressEvent( QKeyEvent* e )
{
    if ( e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier )
    {
        slotCopyContentToClipboard();
        e->setAccepted( true );
    }
    else
    {
        QPlainTextEdit::keyPressEvent( e );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotCopyContentToClipboard()
{
    QTextCursor cursor( this->textCursor() );

    QString textForClipboard;

    QString selText = cursor.selectedText();
    if ( !selText.isEmpty() )
    {
        QTextDocument doc;
        doc.setPlainText( selText );

        textForClipboard = doc.toPlainText();
    }

    if ( textForClipboard.isEmpty() )
    {
        textForClipboard = this->toPlainText();
    }

    if ( !textForClipboard.isEmpty() )
    {
        QClipboard* clipboard = QApplication::clipboard();
        if ( clipboard )
        {
            clipboard->setText( textForClipboard );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotSelectAll()
{
    this->selectAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotExportToFile()
{
    // Get dialog
    RiuTabbedTextDialog* dialog = nullptr;
    auto                 curr   = parent();
    while ( dialog == nullptr )
    {
        if ( !curr ) break;
        dialog = dynamic_cast<RiuTabbedTextDialog*>( curr );
        if ( dialog ) break;
        curr = curr->parent();
    }

    if ( dialog )
    {
        QString defaultDir = RicAsciiExportSummaryPlotFeature::defaultExportDir();
        auto fileName = RicAsciiExportSummaryPlotFeature::getFileNameFromUserDialog( dialog->description(), defaultDir );
        RicAsciiExportSummaryPlotFeature::exportTextToFile( fileName, this->toPlainText() );
    }
}

//--------------------------------------------------------------------------------------------------
///
///
/// RiuTextDialog
///
///
//--------------------------------------------------------------------------------------------------
RiuTextDialog::RiuTextDialog( QWidget* parent )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
{
    m_textEdit = new RiuQPlainTextEdit( this );
    m_textEdit->setReadOnly( true );
    m_textEdit->setLineWrapMode( QPlainTextEdit::NoWrap );

    QFont font( "Courier", 8 );
    m_textEdit->setFont( font );

    m_textEdit->setContextMenuPolicy( Qt::NoContextMenu );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget( m_textEdit );
    setLayout( layout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextDialog::setText( const QString& text )
{
    m_textEdit->setPlainText( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextDialog::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu menu;

    {
        QAction* actionToSetup = new QAction( this );

        actionToSetup->setText( "Copy" );
        actionToSetup->setIcon( QIcon( ":/Copy.svg" ) );
        caf::CmdFeature::applyShortcutWithHintToAction( actionToSetup, QKeySequence::Copy );

        connect( actionToSetup, SIGNAL( triggered() ), m_textEdit, SLOT( slotCopyContentToClipboard() ) );

        menu.addAction( actionToSetup );
    }

    {
        QAction* actionToSetup = new QAction( this );

        actionToSetup->setText( "Select All" );
        caf::CmdFeature::applyShortcutWithHintToAction( actionToSetup, QKeySequence::SelectAll );

        connect( actionToSetup, SIGNAL( triggered() ), m_textEdit, SLOT( slotSelectAll() ) );

        menu.addAction( actionToSetup );
    }

    menu.exec( event->globalPos() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuTabbedTextDialog::RiuTabbedTextDialog( RiuTabbedTextProvider* textProvider, QWidget* parent /*= nullptr*/ )
    : m_textProvider( textProvider )
    , QDialog( parent, RiuTools::defaultDialogFlags() )
{
    m_tabWidget = new QTabWidget( this );

    connect( m_tabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( slotTabChanged( int ) ) );

    CVF_ASSERT( m_textProvider->isValid() );
    this->setWindowTitle( m_textProvider->description() );

    for ( int tabIndex = 0; tabIndex < m_textProvider->tabCount(); ++tabIndex )
    {
        QString            tabTitle = m_textProvider->tabTitle( tabIndex );
        RiuQPlainTextEdit* textEdit = new RiuQPlainTextEdit();
        textEdit->setReadOnly( true );
        textEdit->setLineWrapMode( QPlainTextEdit::NoWrap );

        QFont font( "Courier", 8 );
        textEdit->setFont( font );
        textEdit->setContextMenuPolicy( Qt::NoContextMenu );

        auto fontWidth = QFontMetrics( font ).boundingRect( "m" ).width();
#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
        textEdit->setTabStopDistance( fontWidth * 4 );
#else
        textEdit->setTabStopWidth( fontWidth * 4 );
#endif

        m_tabWidget->addTab( textEdit, tabTitle );
    }
    m_tabTexts.resize( m_textProvider->tabCount() );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget( m_tabWidget );
    setLayout( layout );

    updateTabText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuTabbedTextDialog::description() const
{
    if ( m_textProvider && m_textProvider->isValid() )
    {
        return m_textProvider->description();
    }
    else
    {
        return "Data Invalid";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTabbedTextDialog::redrawText()
{
    auto textEdit  = currentTextEdit();
    auto currIndex = m_tabWidget->currentIndex();

    textEdit->setPlainText( "Populating Text View..." );
    textEdit->repaint();

    if ( currIndex < (int)m_tabTexts.size() )
    {
        if ( m_tabTexts[currIndex].isEmpty() )
        {
            updateTabText();
        }
        textEdit->setPlainText( m_tabTexts[currIndex] );
        textEdit->repaint();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQPlainTextEdit* RiuTabbedTextDialog::currentTextEdit() const
{
    return dynamic_cast<RiuQPlainTextEdit*>( m_tabWidget->currentWidget() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTabbedTextDialog::updateTabText()
{
    auto currIndex = m_tabWidget->currentIndex();
    if ( m_textProvider && m_textProvider->isValid() &&
         m_tabWidget->tabText( currIndex ) == m_textProvider->tabTitle( currIndex ) )
    {
        m_tabTexts[currIndex] = m_textProvider->tabText( currIndex );
    }
    else
    {
        m_tabTexts[currIndex] = "Data Source No Longer Valid";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTabbedTextDialog::slotTabChanged( int index )
{
    redrawText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTabbedTextDialog::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu              menu;
    RiuQPlainTextEdit* textEdit = dynamic_cast<RiuQPlainTextEdit*>( m_tabWidget->currentWidget() );

    {
        QAction* actionToSetup = new QAction( this );

        actionToSetup->setText( "Copy" );
        actionToSetup->setIcon( QIcon( ":/Copy.svg" ) );
        caf::CmdFeature::applyShortcutWithHintToAction( actionToSetup, QKeySequence::Copy );

        connect( actionToSetup, SIGNAL( triggered() ), textEdit, SLOT( slotCopyContentToClipboard() ) );

        menu.addAction( actionToSetup );
    }

    {
        QAction* actionToSetup = new QAction( this );

        actionToSetup->setText( "Select All" );
        caf::CmdFeature::applyShortcutWithHintToAction( actionToSetup, QKeySequence::SelectAll );

        connect( actionToSetup, SIGNAL( triggered() ), textEdit, SLOT( slotSelectAll() ) );

        menu.addAction( actionToSetup );
    }

    {
        QAction* actionToSetup = new QAction( this );

        actionToSetup->setText( "Export to File..." );

        connect( actionToSetup, SIGNAL( triggered() ), textEdit, SLOT( slotExportToFile() ) );

        menu.addAction( actionToSetup );
    }

    menu.exec( event->globalPos() );
}
