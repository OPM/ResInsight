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
#include "RiaGrpcConsoleApplication.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QProcessEnvironment RiaGrpcConsoleApplication::pythonProcessEnvironment() const
{
    return grpcProcessEnvironment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcConsoleApplication::doIdleProcessing()
{
    int processCount = processRequests();
    if ( processCount == -1 )
    {
        closeProject();
        QCoreApplication::quit();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcConsoleApplication::RiaGrpcConsoleApplication( int& argc, char** argv )
    : RiaConsoleApplication( argc, argv )
{
    m_idleTimer = new QTimer( this );
    connect( m_idleTimer, SIGNAL( timeout() ), this, SLOT( doIdleProcessing() ) );
    m_idleTimer->start( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcConsoleApplication::~RiaGrpcConsoleApplication()
{
    delete m_idleTimer;
}
