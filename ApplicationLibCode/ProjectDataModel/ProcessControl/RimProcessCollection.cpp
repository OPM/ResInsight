/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimProcessCollection.h"

#include "RiaLogging.h"

#include "RimProcess.h"
#include "RimProcessThread.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimProcessCollection, "ProcessCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcessCollection::RimProcessCollection()
    : m_nextProcessID( 1 )
    , m_nThreadsRunning( 0 )
    , m_bRunEnabled( false )
{
    CAF_PDM_InitObject( "Processes", ":/Folder.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_processes, "Processes", "Processes", "", "", "" );
    m_processes.uiCapability()->setUiTreeHidden( true );
    m_processes.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_maxParallellProcesses, "MaxParallellProcesses", 1, "Max Processes in Parallell", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcessCollection::~RimProcessCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessCollection::addProcess( RimProcess* process )
{
    process->setID( m_nextProcessID++ );
    m_processes.push_back( process );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimProcess*> RimProcessCollection::processes() const
{
    return m_processes.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                           std::vector<caf::PdmObjectHandle*>& referringObjects )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessCollection::run()
{
    if ( m_bRunEnabled ) return;

    if ( m_processes.size() == 0 ) return;

    // RimProcessThread* newThread = new RimProcessThread()

    m_bRunEnabled = true;
}
