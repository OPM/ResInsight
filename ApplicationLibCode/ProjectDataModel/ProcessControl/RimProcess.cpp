/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "RimProcess.h"

#include "RiaLogging.h"
#include "RimProcessMonitor.h"

#include "cafPdmFieldCapability.h"

#include <QApplication>
#include <QProcess>
#include <QProcessEnvironment>

CAF_PDM_SOURCE_INIT( RimProcess, "RimProcess" );

int RimProcess::m_nextProcessId = 1;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcess::RimProcess( bool logStdOutErr /*true*/ )
    : m_enableLogging( logStdOutErr )
{
    int defId = m_nextProcessId++;
    m_monitor = new RimProcessMonitor( defId, logStdOutErr );

    CAF_PDM_InitObject( "ResInsight Process", ":/Erase.png" );

    CAF_PDM_InitFieldNoDefault( &m_command, "Command", "Command" );
    m_command.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_workDir, "WorkDir", QString(), "Working Directory" );
    m_command.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_id, "ID", defId, "ID" );
    m_id.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcess::~RimProcess()
{
    delete m_monitor;
}

caf::PdmFieldHandle* RimProcess::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::addParameter( QString paramStr )
{
    m_arguments << paramStr.trimmed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::addParameters( QStringList parameterList )
{
    for ( int i = 0; i < parameterList.size(); i++ )
    {
        addParameter( parameterList[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::setCommand( QString cmdStr )
{
    m_command = cmdStr.trimmed();

    QString shell = optionalCommandInterpreter();
    if ( shell.isEmpty() ) return;

    QString preParam = optionalPreParameters();
    if ( !preParam.isEmpty() ) m_arguments.append( preParam );

    m_arguments.append( cmdStr.trimmed() );
    m_command = shell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::setDescription( QString desc )
{
    m_description = desc;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProcess::command() const
{
    return m_command;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimProcess::parameters() const
{
    return m_arguments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimProcess::stdOut() const
{
    if ( m_monitor ) return m_monitor->stdOut();
    return QStringList();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimProcess::stdErr() const
{
    if ( m_monitor ) return m_monitor->stdErr();
    return QStringList();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimProcess::ID() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProcess::execute( bool enableStdOut, bool enableStdErr )
{
    if ( !m_monitor ) return false;

    QProcess* proc = new QProcess();
    QString   cmd  = commandLine();

    if ( m_enableLogging ) RiaLogging::info( QString( "Start process %1: %2" ).arg( m_id ).arg( cmd ) );

    m_monitor->clearStdOutErr();

    QObject::connect( proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), m_monitor, SLOT( finished( int, QProcess::ExitStatus ) ) );
    if ( enableStdOut ) QObject::connect( proc, SIGNAL( readyReadStandardOutput() ), m_monitor, SLOT( readyReadStandardOutput() ) );
    if ( enableStdErr ) QObject::connect( proc, SIGNAL( readyReadStandardError() ), m_monitor, SLOT( readyReadStandardError() ) );
    QObject::connect( proc, SIGNAL( started() ), m_monitor, SLOT( started() ) );

    bool retval = false;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for ( auto& [key, val] : m_environmentVariables )
    {
        env.insert( key, val );
    }
    proc->setProcessEnvironment( env );
    if ( !m_workDir().isEmpty() )
    {
        proc->setWorkingDirectory( m_workDir );
    }

    proc->start( m_command, m_arguments );
    auto error = proc->errorString();
    if ( proc->waitForStarted( -1 ) )
    {
        while ( !proc->waitForFinished( 500 ) )
        {
            QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
        }
        retval = ( proc->exitCode() == 0 );
    }
    else
    {
        RiaLogging::error( QString( "Failed to start process %1. %2." ).arg( m_id ).arg( error ) );
    }

    proc->deleteLater();

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProcess::optionalCommandInterpreter() const
{
    if ( m_command.value().isNull() ) return "";

    if ( isWindowsBatchFile() )
    {
        return "cmd.exe";
    }
    if ( m_command.value().endsWith( ".sh", Qt::CaseInsensitive ) )
    {
        return "bash";
    }
    if ( m_command.value().endsWith( ".csh", Qt::CaseInsensitive ) )
    {
        return "csh";
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProcess::optionalPreParameters() const
{
    if ( m_command.value().isNull() ) return "";

    if ( isWindowsBatchFile() )
    {
        return "/c";
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProcess::isWindowsBatchFile() const
{
    return ( m_command.value().endsWith( ".cmd", Qt::CaseInsensitive ) || m_command.value().endsWith( ".bat", Qt::CaseInsensitive ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProcess::commandLine() const
{
    QString cmdline = handleSpaces( m_command );

    for ( int i = 0; i < m_arguments.size(); i++ )
    {
        cmdline += " ";
        cmdline += handleSpaces( m_arguments[i] );
    }

    return cmdline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProcess::handleSpaces( QString arg ) const
{
    if ( arg.contains( " " ) && !arg.startsWith( "\"" ) )
    {
        return QString( "\"" + arg + "\"" );
    }
    return arg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::addEnvironmentVariable( QString name, QString value )
{
    m_environmentVariables.push_back( std::make_pair( name, value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::setWorkingDirectory( QString workDir )
{
    m_workDir = workDir;
}
