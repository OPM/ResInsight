/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021    Equinor ASA
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

#include "RimProcessMonitor.h"

#include "RiaLogging.h"

#include <QProcess>
#include <QtCore/QtCore>

RimProcessMonitor::RimProcessMonitor( int processId )
    : QObject( nullptr )
    , m_processId( processId )
{
}

QString RimProcessMonitor::addPrefix( QString message )
{
    return QString( "Process %1: %2" ).arg( m_processId ).arg( message );
}

void RimProcessMonitor::error( QProcess::ProcessError error )
{
    QString errorStr;

    switch ( error )
    {
        case QProcess::FailedToStart:
            errorStr = "Failed to start";
            break;
        case QProcess::Crashed:
            errorStr = "Crashed";
            break;
        case QProcess::Timedout:
            errorStr = "Timed out";
            break;
        case QProcess::ReadError:
            errorStr = "Read error";
            break;
        case QProcess::WriteError:
            errorStr = "Write error";
            break;
        case QProcess::UnknownError:
        default:
            errorStr = "Unknown";
            break;
    }

    RiaLogging::error( addPrefix( errorStr ) );
}

void RimProcessMonitor::finished( int exitCode, QProcess::ExitStatus exitStatus )
{
    QString finishStr;
    switch ( exitStatus )
    {
        case QProcess::NormalExit:
            finishStr = QString( "Normal exit, code %1" ).arg( exitCode );
            break;
        case QProcess::CrashExit:
        default:
            finishStr = QString( "Crash exit, code %1" ).arg( exitCode );
            break;
    }

    RiaLogging::debug( addPrefix( finishStr ) );
}

void RimProcessMonitor::readyReadStandardError()
{
    QProcess* p = (QProcess*)sender();
    p->setReadChannel( QProcess::StandardError );
    while ( p->canReadLine() )
    {
        QString line = p->readLine();
        line         = line.trimmed();
        if ( line.size() == 0 ) continue;
        RiaLogging::error( addPrefix( line ) );
    }
}

void RimProcessMonitor::readyReadStandardOutput()
{
    QProcess* p = (QProcess*)sender();
    p->setReadChannel( QProcess::StandardOutput );
    while ( p->canReadLine() )
    {
        QString line = p->readLine();
        line         = line.trimmed();
        if ( line.size() == 0 ) continue;
        RiaLogging::info( addPrefix( line ) );
    }
}

void RimProcessMonitor::started()
{
    RiaLogging::debug( addPrefix( "Started" ) );
}
