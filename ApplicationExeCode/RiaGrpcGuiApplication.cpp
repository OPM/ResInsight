////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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
#include "RiaGrpcGuiApplication.h"

#include "RiaPreferences.h"

#include "cafProgressInfo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcGuiApplication::RiaGrpcGuiApplication( int& argc, char** argv )
    : RiaGuiApplication( argc, argv )
{
    m_idleTimer = new QTimer( this );
    connect( m_idleTimer, SIGNAL( timeout() ), this, SLOT( doIdleProcessing() ) );
    m_idleTimer->start( 5 );

    connect( this, SIGNAL( aboutToQuit() ), this, SLOT( onProgramExit() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcGuiApplication::~RiaGrpcGuiApplication()
{
    delete m_idleTimer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QProcessEnvironment RiaGrpcGuiApplication::pythonProcessEnvironment() const
{
    return grpcProcessEnvironment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcGuiApplication::doIdleProcessing()
{
    if ( !caf::ProgressInfoStatic::isRunning() )
    {
        int processCount = processRequests();
        if ( processCount == -1 )
        {
            closeProject();
            QCoreApplication::quit();
        }
        else
        {
            static int idleIterationCount = 0;
            int        iterationInterval  = 0;
            if ( processCount > 0 )
            {
                idleIterationCount = 0;
            }
            else
            {
                ++idleIterationCount;
                idleIterationCount = std::min( idleIterationCount, 500 );
                if ( idleIterationCount == 500 )
                {
                    iterationInterval = 5;
                }
            }
            if ( iterationInterval != m_idleTimer->interval() )
            {
                m_idleTimer->setInterval( iterationInterval );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcGuiApplication::onGuiPreferencesChanged()
{
    bool isGrpcRunning     = m_grpcServer != nullptr && m_grpcServer->isRunning();
    bool shouldItBeRunning = m_preferences->enableGrpcServer();
    if ( isGrpcRunning && !shouldItBeRunning )
    {
        m_grpcServer->quit();
    }
    else if ( !isGrpcRunning && shouldItBeRunning )
    {
        int portNumber = RiaGrpcServer::findAvailablePortNumber( m_preferences->defaultGrpcPortNumber() );
        m_grpcServer   = std::make_unique<RiaGrpcServer>( portNumber );
        m_grpcServer->runInThread();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcGuiApplication::onProgramExit()
{
    if ( m_grpcServer )
    {
        m_grpcServer->quit();
    }
}
