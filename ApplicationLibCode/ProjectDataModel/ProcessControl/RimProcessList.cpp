/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026    Equinor ASA
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

#include "RimProcessList.h"

#include "RimProcess.h"
#include "RimProcessMonitor.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcessList::RimProcessList( std::shared_ptr<RimProcessMonitor> monitor )
    : m_monitor( monitor )
    , m_name( "<unknown>" )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcessList::~RimProcessList()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<RimProcess>> RimProcessList::createProcesses()
{
    std::vector<std::shared_ptr<RimProcess>> retList;

    for ( auto cmdline : m_commandList )
    {
        auto proc = std::make_shared<RimProcess>( true /*log output*/, m_monitor );
        auto cmd  = cmdline.takeFirst();
        proc->setCommand( cmd );
        proc->addParameters( cmdline );
        proc->setWslOptions( m_wslOptions );
        for ( auto& [var, val] : m_environment )
        {
            proc->addEnvironmentVariable( var, val );
        }
        proc->setWorkingDirectory( m_workingDirectory );
        proc->setDescription( m_name );

        retList.push_back( proc );
    }

    return retList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<QStringList>& RimProcessList::commands()
{
    return m_commandList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessList::setWslOptions( const RiaWslTools::WslOptions& wslOpt )
{
    m_wslOptions = wslOpt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessList::setName( QString name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RiaWslTools::WslOptions& RimProcessList::wslOptions() const
{
    return m_wslOptions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList& RimProcessList::stdOut() const
{
    if ( m_monitor ) return m_monitor->stdOut();
    return m_emptyList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList& RimProcessList::stdErr() const
{
    if ( m_monitor ) return m_monitor->stdErr();
    return m_emptyList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessList::addProcessCommand( QString command, QStringList arguments )
{
    QStringList cmdLine;
    cmdLine.append( command );
    cmdLine.append( arguments );

    m_commandList.push_back( cmdLine );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessList::addProcessCommand( QStringList arguments )
{
    m_commandList.push_back( arguments );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessList::addEnvironmentVariable( QString name, QString value )
{
    m_environment[name] = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessList::setWorkingDirectory( QString filePath )
{
    m_workingDirectory = filePath;
}
