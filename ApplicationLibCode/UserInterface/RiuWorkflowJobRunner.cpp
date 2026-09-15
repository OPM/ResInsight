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

#include "RiuWorkflowJobRunner.h"

RiuWorkflowJobRunner::RiuWorkflowJobRunner( const QString& label, QObject* parent )
    : QObject( parent )
    , m_label( label )
{
    connect( &m_process, &QProcess::readyReadStandardOutput, this, &RiuWorkflowJobRunner::onReadyReadStdout );
    connect( &m_process, &QProcess::readyReadStandardError, this, &RiuWorkflowJobRunner::onReadyReadStderr );
    connect( &m_process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &RiuWorkflowJobRunner::onProcessFinished );
}

void RiuWorkflowJobRunner::start( const QString& program, const QStringList& arguments, const QProcessEnvironment& env )
{
    RiaLogging::info( QString( "Running %1" ).arg( m_label ).toStdString() );
    RiaLogging::info( QString( "[%1] $ %2 %3" ).arg( m_label, program, arguments.join( ' ' ) ).toStdString() );
    m_process.setProcessEnvironment( env );
    m_process.start( program, arguments );
}

void RiuWorkflowJobRunner::cancel()
{
    if ( m_process.state() == QProcess::NotRunning ) return;
    m_process.terminate();
    if ( !m_process.waitForFinished( 2000 ) ) m_process.kill();
}

bool RiuWorkflowJobRunner::isRunning() const
{
    return m_process.state() != QProcess::NotRunning;
}

void RiuWorkflowJobRunner::onReadyReadStdout()
{
    m_stdoutBuf.append( m_process.readAllStandardOutput() );
    drainLines( m_stdoutBuf, RILogLevel::RI_LL_INFO );
}

void RiuWorkflowJobRunner::onReadyReadStderr()
{
    m_stderrBuf.append( m_process.readAllStandardError() );
    drainLines( m_stderrBuf, RILogLevel::RI_LL_WARNING );
}

void RiuWorkflowJobRunner::onProcessFinished( int exitCode, QProcess::ExitStatus status )
{
    m_stdoutBuf.append( m_process.readAllStandardOutput() );
    m_stderrBuf.append( m_process.readAllStandardError() );

    if ( !m_stdoutBuf.endsWith( '\n' ) ) m_stdoutBuf.append( '\n' );
    if ( !m_stderrBuf.endsWith( '\n' ) ) m_stderrBuf.append( '\n' );
    drainLines( m_stdoutBuf, RILogLevel::RI_LL_INFO );
    drainLines( m_stderrBuf, RILogLevel::RI_LL_WARNING );

    if ( status == QProcess::NormalExit && exitCode == 0 )
    {
        RiaLogging::info( QString( "Finished %1 (exit 0)" ).arg( m_label ).toStdString() );
    }
    else if ( status == QProcess::NormalExit )
    {
        RiaLogging::error( QString( "Finished %1 (exit %2)" ).arg( m_label ).arg( exitCode ).toStdString() );
    }
    else
    {
        RiaLogging::error( QString( "%1 crashed" ).arg( m_label ).toStdString() );
    }

    deleteLater();
}

void RiuWorkflowJobRunner::drainLines( QByteArray& buffer, RILogLevel level )
{
    int nl;
    while ( ( nl = buffer.indexOf( '\n' ) ) >= 0 )
    {
        QString line = QString::fromUtf8( buffer.left( nl ) );
        buffer.remove( 0, nl + 1 );
        if ( line.endsWith( '\r' ) ) line.chop( 1 );
        if ( line.isEmpty() ) continue;

        const std::string msg = QString( "[%1] %2" ).arg( m_label, line ).toStdString();
        if ( level == RILogLevel::RI_LL_WARNING )
            RiaLogging::warning( msg );
        else
            RiaLogging::info( msg );
    }
}
