//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafUiProcess.h"
#include <QTimer>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiProcess::UiProcess( QObject* parent )
    : QProcess( parent )
{
    connect( this, SIGNAL( started() ), SLOT( slotProcStarted() ) );
    connect( this, SIGNAL( error( QProcess::ProcessError ) ), SLOT( slotProcError( QProcess::ProcessError ) ) );
    connect( this, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( slotProcFinished( int, QProcess::ExitStatus ) ) );
    connect( this, SIGNAL( stateChanged( QProcess::ProcessState ) ), SLOT( slotProcStateChanged( QProcess::ProcessState ) ) );
    connect( this, SIGNAL( readyReadStandardError() ), SLOT( slotUpdateStatusMessage() ) );
    connect( this, SIGNAL( readyReadStandardOutput() ), SLOT( slotUpdateStatusMessage() ) );

    // Use a one sec timer to make sure the status message is updated at least every second
    QTimer* timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( slotUpdateStatusMessage() ) );
    timer->start( 1000 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiProcess::doEmitStatusMsg( const QString& msg, int statusMsgType )
{
    QString simpleMsg    = msg;
    QString formattedMsg = simpleMsg;

    if ( statusMsgType == PROCESS_STATE_RUNNING )
        formattedMsg = QString( "<font color='green'>%1</font>" ).arg( simpleMsg );
    else if ( statusMsgType == PROCESS_STATE_ERROR )
        formattedMsg = QString( "<font color='red'>%1</font>" ).arg( simpleMsg );

    emit signalStatusMsg( simpleMsg, statusMsgType );
    emit signalFormattedStatusMsg( formattedMsg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiProcess::slotProcStarted()
{
    m_timer.start();

    doEmitStatusMsg( "Started", PROCESS_STATE_NORMAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiProcess::slotProcError( QProcess::ProcessError error )
{
    QString msg = "UNKNOWN";

    switch ( error )
    {
        case QProcess::FailedToStart:
            msg = "Failed to start";
            break;
        case QProcess::Crashed:
            msg = "Crashed";
            break;
        case QProcess::Timedout:
            msg = "Timed out";
            break;
        case QProcess::WriteError:
            msg = "Write error";
            break;
        case QProcess::ReadError:
            msg = "Read error";
            break;
        case QProcess::UnknownError:
            msg = "Unknown error";
            break;
    }

    doEmitStatusMsg( msg, PROCESS_STATE_ERROR );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiProcess::slotProcFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    if ( exitStatus == QProcess::CrashExit )
    {
        doEmitStatusMsg( "Crashed or aborted", PROCESS_STATE_ERROR );
    }

    else if ( exitStatus == QProcess::NormalExit )
    {
        if ( exitCode == 0 )
        {
            doEmitStatusMsg( "Finished OK", PROCESS_STATE_NORMAL );
        }
        else
        {
            QString msg = QString( "Error exit (code %1)" ).arg( exitCode );
            doEmitStatusMsg( msg, PROCESS_STATE_ERROR );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiProcess::slotProcStateChanged( QProcess::ProcessState newState )
{
    int     statusMsgType = PROCESS_STATE_ERROR;
    QString msg           = "UNKNOWN";

    switch ( newState )
    {
        case QProcess::NotRunning:
            msg           = "Not running";
            statusMsgType = PROCESS_STATE_NORMAL;
            break;
        case QProcess::Starting:
            msg           = "Starting...";
            statusMsgType = PROCESS_STATE_NORMAL;
            break;
        case QProcess::Running:
            msg           = "Running";
            statusMsgType = PROCESS_STATE_RUNNING;
            break;
    }

    doEmitStatusMsg( msg, statusMsgType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiProcess::slotUpdateStatusMessage()
{
    if ( state() == QProcess::Running )
    {
        // Use this as a sign that the process is alive and kicking
        // Emit a message with the current run time to signify progress
        double timeRunning = m_timer.elapsed() / 1000.0;

        QString msg = QString( "Running (%1 s)" ).arg( timeRunning, 0, 'f', 1 );

        doEmitStatusMsg( msg, PROCESS_STATE_RUNNING );
    }
}

} // namespace caf
