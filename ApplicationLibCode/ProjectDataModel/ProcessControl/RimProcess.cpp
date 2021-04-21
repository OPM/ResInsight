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

#include "cafPdmFieldCapability.h"

#include <QProcess>

CAF_PDM_SOURCE_INIT( RimProcess, "RimProcess" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcess::RimProcess()
{
    CAF_PDM_InitObject( "ResInsight Process", ":/Erase.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_command, "Command", "Command", "", "", "" );
    m_command.uiCapability()->setUiReadOnly( true );

    // CAF_PDM_InitFieldNoDefault( &m_arguments, "Arguments", "Arguments", "", "", "" );
    // m_arguments.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description", "", "", "" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_id, "ID", -1, "ID", "", "", "" );
    m_id.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcess::~RimProcess()
{
}

caf::PdmFieldHandle* RimProcess::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                        QString                    uiConfigName,
                                        caf::PdmUiEditorAttribute* attribute )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
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
void RimProcess::setParameters( QStringList parameterList )
{
    m_arguments.clear();
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
    m_command = cmdStr;
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
int RimProcess::ID() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::setID( int id )
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProcess::execute()
{
    QProcess* myProcess = new QProcess();
    QString   cmd       = commandLine();
    myProcess->start( cmd );
    if ( myProcess->waitForStarted( -1 ) )
    {
        myProcess->waitForFinished( -1 );
        return QString( "Done!" );
    }
    return QString( "Failed to start!" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProcess::needsCommandInterpreter() const
{
#ifdef WIN32
    if ( m_command.value().isNull() ) return false;
    return m_command.value().endsWith( ".cmd", Qt::CaseInsensitive ) ||
           m_command.value().endsWith( ".bat", Qt::CaseInsensitive );
#endif
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProcess::commandLine() const
{
    QString cmdline;

    if ( needsCommandInterpreter() )
    {
        cmdline += "cmd.exe /c ";
    }

    cmdline += handleSpaces( m_command );

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
