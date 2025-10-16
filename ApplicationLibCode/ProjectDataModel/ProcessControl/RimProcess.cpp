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
RimProcess::RimProcess( bool logStdOutErr /*true*/, RimProcessMonitor* monitor )
    : m_enableLogging( logStdOutErr )
    , m_qProcess( nullptr )
{
    int defId = m_nextProcessId++;
    if ( monitor == nullptr )
        m_monitor = new RimProcessMonitor( defId, logStdOutErr );
    else
    {
        m_monitor = monitor;
        m_monitor->setProcessId( defId );
    }

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
    if ( m_monitor != nullptr )
    {
        delete m_monitor;
        m_monitor = nullptr;
    }
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
bool RimProcess::start( bool enableStdOut, bool enableStdErr )
{
    if ( !m_monitor ) return false;
    if ( m_qProcess != nullptr ) return false;

    m_qProcess  = new QProcess();
    QString cmd = commandLine();

    if ( m_enableLogging ) RiaLogging::info( QString( "Start process %1: %2" ).arg( m_id ).arg( cmd ) );

    m_monitor->clearStdOutErr();

    QObject::connect( m_qProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ), m_monitor, SLOT( finished( int, QProcess::ExitStatus ) ) );
    if ( enableStdOut ) QObject::connect( m_qProcess, SIGNAL( readyReadStandardOutput() ), m_monitor, SLOT( readyReadStandardOutput() ) );
    if ( enableStdErr ) QObject::connect( m_qProcess, SIGNAL( readyReadStandardError() ), m_monitor, SLOT( readyReadStandardError() ) );
    QObject::connect( m_qProcess, SIGNAL( started() ), m_monitor, SLOT( started() ) );

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for ( auto& [key, val] : m_environmentVariables )
    {
        env.insert( key, val );
    }
    m_qProcess->setProcessEnvironment( env );
    if ( !m_workDir().isEmpty() )
    {
        m_qProcess->setWorkingDirectory( m_workDir );
    }

    m_qProcess->start( m_command, m_arguments );
    auto error = m_qProcess->errorString();
    if ( !m_qProcess->waitForStarted( -1 ) )
    {
        RiaLogging::error( QString( "Failed to start process %1. %2." ).arg( m_id ).arg( error ) );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::cleanUpAfterRun()
{
    if ( m_qProcess != nullptr )
    {
        m_qProcess->deleteLater();
        m_qProcess = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::terminate()
{
    if ( m_qProcess != nullptr )
    {
        if ( m_qProcess->state() == QProcess::Running )
        {
            m_qProcess->kill();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProcess::execute( bool enableStdOut, bool enableStdErr )
{
    if ( !start( enableStdOut, enableStdErr ) ) return false;

    while ( !m_qProcess->waitForFinished( 250 ) )
    {
        QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    }

    bool retval = ( m_qProcess->exitCode() == 0 );

    cleanUpAfterRun();

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
