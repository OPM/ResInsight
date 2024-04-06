/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiuMessagePanel.h"

#include "RiaRegressionTestRunner.h"

#include "RiuGuiTheme.h"

#include "DockWidget.h"

#include "cafStyleSheetTools.h"

#include <QMenu>
#include <QPlainTextEdit>
#include <QThread>
#include <QVBoxLayout>

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMessagePanel::RiuMessagePanel( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( caf::StyleSheetTools::smallContentMargin(),
                                caf::StyleSheetTools::smallContentMargin(),
                                caf::StyleSheetTools::smallContentMargin(),
                                caf::StyleSheetTools::smallContentMargin() );

    m_textEdit = new QPlainTextEdit;
    m_textEdit->setReadOnly( true );
    m_textEdit->setLineWrapMode( QPlainTextEdit::NoWrap );
    m_textEdit->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_textEdit, SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( slotShowContextMenu( const QPoint& ) ) );

    layout->addWidget( m_textEdit );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanel::addMessage( RILogLevel messageLevel, const QString& msg )
{
    QColor clr = RiuGuiTheme::getColorByVariableName( "textColor" );
    if ( messageLevel == RILogLevel::RI_LL_ERROR )
        clr = Qt::red;
    else if ( messageLevel == RILogLevel::RI_LL_WARNING )
        clr = QColor( 220, 100, 10 );
    else if ( messageLevel == RILogLevel::RI_LL_DEBUG )
        clr = QColor( 100, 100, 200 );

    QTextCharFormat form = m_textEdit->currentCharFormat();
    form.setForeground( clr );
    form.setFontWeight( messageLevel == RILogLevel::RI_LL_ERROR ? QFont::DemiBold : QFont::Normal );
    form.setFontItalic( messageLevel == RILogLevel::RI_LL_DEBUG );
    m_textEdit->setCurrentCharFormat( form );
    m_textEdit->appendPlainText( msg );

    m_textEdit->moveCursor( QTextCursor::End );
    m_textEdit->ensureCursorVisible();

    if ( !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        if ( messageLevel == RILogLevel::RI_LL_ERROR || messageLevel == RILogLevel::RI_LL_WARNING )
        {
            ads::CDockWidget* parentDockWidget = dynamic_cast<ads::CDockWidget*>( parent() );
            if ( parentDockWidget && !parentDockWidget->isVisible() )
            {
                parentDockWidget->toggleViewAction()->trigger();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMessagePanel::sizeHint() const
{
    return QSize( 20, 20 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanel::slotShowContextMenu( const QPoint& pos )
{
    QMenu menu;

    // Reworked from implementation in  QTextControl::createStandardContextMenu()
    {
        QAction* a = menu.addAction( "&Copy", m_textEdit, SLOT( copy() ), QKeySequence::Copy );
        a->setEnabled( m_textEdit->textCursor().hasSelection() );
    }
    {
        menu.addSeparator();
        QAction* a = menu.addAction( "Select All", m_textEdit, SLOT( selectAll() ), QKeySequence::SelectAll );
        a->setEnabled( !m_textEdit->document()->isEmpty() );
    }

    menu.addSeparator();
    menu.addAction( "Clear All &Messages", this, SLOT( slotClearMessages() ) );

    menu.exec( m_textEdit->mapToGlobal( pos ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanel::slotClearMessages()
{
    m_textEdit->clear();

    RiaLogging::info( "Message window cleared." );
}

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMessagePanelLogger::RiuMessagePanelLogger()
    : m_logLevel( int( RILogLevel::RI_LL_WARNING ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::addMessagePanel( RiuMessagePanel* messagePanel )
{
    // get rid of any unused entries (qpointers are null)
    m_messagePanels.erase( std::remove( m_messagePanels.begin(), m_messagePanels.end(), nullptr ), m_messagePanels.end() );

    m_messagePanels.push_back( messagePanel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMessagePanelLogger::level() const
{
    return m_logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::setLevel( int logLevel )
{
    m_logLevel = logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::error( const char* message )
{
    writeToMessagePanel( RILogLevel::RI_LL_ERROR, message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::warning( const char* message )
{
    writeToMessagePanel( RILogLevel::RI_LL_WARNING, message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::info( const char* message )
{
    writeToMessagePanel( RILogLevel::RI_LL_INFO, message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::debug( const char* message )
{
    writeToMessagePanel( RILogLevel::RI_LL_DEBUG, message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::writeToMessagePanel( RILogLevel messageLevel, const char* message )
{
    if ( int( messageLevel ) > m_logLevel )
    {
        return;
    }

    for ( auto& panel : m_messagePanels )
    {
        if ( panel )
        {
            // Make sure we only output messages for the GUI-thread.
            // We can loose some messages, but we avoid updating UI from a different thread that will cause asserts and
            // potential crashes
            if ( panel->thread() == QThread::currentThread() )
            {
                panel->addMessage( messageLevel, message );
            }
        }
    }
}
