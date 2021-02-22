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

#include "RiaGrpcApplicationInterface.h"
#include "RiaPreferences.h"

#include "cvfProgramOptions.h"

#include <QProcessEnvironment>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGrpcApplicationInterface::initializeGrpcServer( const cvf::ProgramOptions& progOpt )
{
    if ( !RiaPreferences::current()->enableGrpcServer() ) return false;

    int  defaultPortNumber = RiaPreferences::current()->defaultGrpcPortNumber();
    bool fixedPort         = false;
    if ( cvf::Option o = progOpt.option( "server" ) )
    {
        if ( o.valueCount() == 1 )
        {
            defaultPortNumber = o.value( 0 ).toInt( defaultPortNumber );
            fixedPort         = true;
        }
    }
    int portNumber = defaultPortNumber;
    if ( !fixedPort )
    {
        portNumber = RiaGrpcServer::findAvailablePortNumber( defaultPortNumber );
    }
    m_grpcServer = std::make_unique<RiaGrpcServer>( portNumber );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcApplicationInterface::launchGrpcServer()
{
    if ( m_grpcServer ) m_grpcServer->runInThread();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServer* RiaGrpcApplicationInterface::grpcServer() const
{
    return m_grpcServer.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QProcessEnvironment RiaGrpcApplicationInterface::grpcProcessEnvironment() const
{
    QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();

    if ( m_grpcServer )
    {
        penv.insert( "RESINSIGHT_GRPC_PORT", QString( "%1" ).arg( m_grpcServer->portNumber() ) );
        penv.insert( "RESINSIGHT_EXECUTABLE", QCoreApplication::applicationFilePath() );

        QStringList ripsLocations;
        QString     separator;
#ifdef WIN32
        ripsLocations << QCoreApplication::applicationDirPath() + "\\Python";
        separator = ";";

#else
        ripsLocations << QCoreApplication::applicationDirPath() + "/Python";
        separator = ":";
#endif
        penv.insert( "PYTHONPATH",
                     QString( "%1%2%3" ).arg( penv.value( "PYTHONPATH" ) ).arg( separator ).arg( ripsLocations.join( separator ) ) );
    }

    return penv;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaGrpcApplicationInterface::processRequests()
{
    if ( RiaGrpcServer::receivedExitRequest() )
    {
        if ( m_grpcServer ) m_grpcServer->quit();
        return -1;
    }

    if ( m_grpcServer )
    {
        size_t requestsProcessed = m_grpcServer->processAllQueuedRequests();
        return static_cast<int>( requestsProcessed );
    }

    return 0;
}
