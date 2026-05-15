/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiuWorkflowRunDialog.h"

#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

RiuWorkflowRunDialog::RiuWorkflowRunDialog( const QString& workflowName, QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle( QString( "Run Workflow: %1" ).arg( workflowName ) );
    resize( 720, 480 );
    setModal( false );

    m_log = new QPlainTextEdit( this );
    m_log->setReadOnly( true );
    m_log->setFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );

    m_cancelButton = new QPushButton( "Cancel", this );
    m_closeButton  = new QPushButton( "Close", this );
    m_closeButton->setEnabled( false );

    auto* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget( m_cancelButton );
    buttons->addWidget( m_closeButton );

    auto* layout = new QVBoxLayout( this );
    layout->addWidget( m_log );
    layout->addLayout( buttons );

    connect( m_cancelButton, &QPushButton::clicked, this, &RiuWorkflowRunDialog::onCancelClicked );
    connect( m_closeButton, &QPushButton::clicked, this, &QDialog::accept );

    connect( &m_process, &QProcess::readyReadStandardOutput, this, &RiuWorkflowRunDialog::onReadyReadStdout );
    connect( &m_process, &QProcess::readyReadStandardError, this, &RiuWorkflowRunDialog::onReadyReadStderr );
    connect( &m_process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &RiuWorkflowRunDialog::onProcessFinished );
}

RiuWorkflowRunDialog::~RiuWorkflowRunDialog()
{
    if ( m_process.state() != QProcess::NotRunning )
    {
        m_process.kill();
        m_process.waitForFinished( 2000 );
    }
}

void RiuWorkflowRunDialog::start( const QString& program, const QStringList& arguments, const QProcessEnvironment& env )
{
    appendLog( QString( "$ %1 %2" ).arg( program, arguments.join( ' ' ) ) );
    m_process.setProcessEnvironment( env );
    m_process.start( program, arguments );
}

void RiuWorkflowRunDialog::onReadyReadStdout()
{
    appendLog( QString::fromUtf8( m_process.readAllStandardOutput() ) );
}

void RiuWorkflowRunDialog::onReadyReadStderr()
{
    appendLog( QString::fromUtf8( m_process.readAllStandardError() ) );
}

void RiuWorkflowRunDialog::onProcessFinished( int exitCode, QProcess::ExitStatus status )
{
    QString tail = ( status == QProcess::NormalExit ) ? QString( "[exited with code %1]" ).arg( exitCode ) : QString( "[crashed]" );
    appendLog( "\n" + tail );
    m_cancelButton->setEnabled( false );
    m_closeButton->setEnabled( true );
}

void RiuWorkflowRunDialog::onCancelClicked()
{
    if ( m_process.state() != QProcess::NotRunning )
    {
        m_process.terminate();
        if ( !m_process.waitForFinished( 2000 ) ) m_process.kill();
    }
}

void RiuWorkflowRunDialog::appendLog( const QString& text )
{
    if ( text.isEmpty() ) return;
    m_log->moveCursor( QTextCursor::End );
    m_log->insertPlainText( text );
    m_log->moveCursor( QTextCursor::End );
}
