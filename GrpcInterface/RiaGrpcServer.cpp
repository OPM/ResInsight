/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////

#include "RiaGrpcServer.h"

#include "RiaDefines.h"
#include "RiaLogging.h"

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcCaseService.h"
#include "RiaGrpcServiceInterface.h"

#include "RigCaseCellResultsData.h"
#include "RigMainGrid.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "cafAssert.h"
#include "cafProgressInfo.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <QTcpServer>

using grpc::CompletionQueue;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

//==================================================================================================
//
// The GRPC server implementation
//
//==================================================================================================
class RiaGrpcServerImpl
{
public:
    RiaGrpcServerImpl( int portNumber );
    ~RiaGrpcServerImpl();
    int    portNumber() const;
    bool   isRunning() const;
    void   run();
    void   runInThread();
    void   initialize();
    size_t processAllQueuedRequests();
    void   quit();

private:
    void waitForNextRequest();
    void process( RiaGrpcCallbackInterface* method );

private:
    int                                                 m_portNumber;
    std::unique_ptr<grpc::ServerCompletionQueue>        m_completionQueue;
    std::unique_ptr<grpc::Server>                       m_server;
    std::list<std::shared_ptr<RiaGrpcServiceInterface>> m_services;
    std::list<RiaGrpcCallbackInterface*>                m_unprocessedRequests;
    std::mutex                                          m_requestMutex;
    std::thread                                         m_thread;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServerImpl::RiaGrpcServerImpl( int portNumber )
    : m_portNumber( portNumber )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServerImpl::~RiaGrpcServerImpl()
{
    quit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaGrpcServerImpl::portNumber() const
{
    return m_portNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGrpcServerImpl::isRunning() const
{
    return m_server != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::run()
{
    initialize();
    while ( true )
    {
        waitForNextRequest();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::runInThread()
{
    initialize();
    m_thread = std::thread( &RiaGrpcServerImpl::waitForNextRequest, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::initialize()
{
    CAF_ASSERT( m_portNumber > 0 && m_portNumber <= (int)std::numeric_limits<quint16>::max() );

    QString serverAddress = QString( "localhost:%1" ).arg( m_portNumber );

    ServerBuilder builder;
    builder.AddListeningPort( serverAddress.toStdString(), grpc::InsecureServerCredentials() );

    for ( auto key : RiaGrpcServiceFactory::instance()->allKeys() )
    {
        std::shared_ptr<RiaGrpcServiceInterface> service( RiaGrpcServiceFactory::instance()->create( key ) );
        builder.RegisterService( dynamic_cast<grpc::Service*>( service.get() ) );
        m_services.push_back( service );
    }

    m_completionQueue = builder.AddCompletionQueue();
    m_server          = builder.BuildAndStart();

    CVF_ASSERT( m_server );
    RiaLogging::info( QString( "Server listening on %1" ).arg( serverAddress ) );

    // Spawn new CallData instances to serve new clients.
    for ( auto service : m_services )
    {
        for ( auto callback : service->createCallbacks() )
        {
            process( callback );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaGrpcServerImpl::processAllQueuedRequests()
{
    std::list<RiaGrpcCallbackInterface*> waitingRequests;
    {
        // Block only while transferring the unprocessed requests to a local function list
        std::lock_guard<std::mutex> requestLock( m_requestMutex );
        waitingRequests.swap( m_unprocessedRequests );
    }
    size_t count = waitingRequests.size();

    // Now free to receive new requests from client while processing the current ones.
    while ( !waitingRequests.empty() )
    {
        RiaGrpcCallbackInterface* method = waitingRequests.front();
        waitingRequests.pop_front();
        process( method );
    }
    return count;
}

//--------------------------------------------------------------------------------------------------
/// Gracefully shut down the GRPC server.
/// BE VERY CAREFUL ABOUT CHANGING THE ORDER IN THIS METHOD. IT IS IMPORTANT!
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::quit()
{
    if ( m_server )
    {
        RiaLogging::info( "Shutting down gRPC server" );
        // Clear unhandled requests
        while ( !m_unprocessedRequests.empty() )
        {
            RiaGrpcCallbackInterface* method = m_unprocessedRequests.front();
            m_unprocessedRequests.pop_front();
            delete method;
        }

        // Shutdown server and queue
        m_server->Shutdown();
        m_completionQueue->Shutdown();

        // Wait for thread to join after handling the shutdown call
        m_thread.join();

        // Must destroy server before services
        m_server.reset();
        m_completionQueue.reset();

        // Finally clear services
        m_services.clear();
    }
}

//--------------------------------------------------------------------------------------------------
/// Block and wait for requests from the client from the command queue.
/// The requests are pushed onto the Unprocessed Request queue which are handled in processRequests
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::waitForNextRequest()
{
    void* tag;
    bool  ok = false;

    while ( m_completionQueue->Next( &tag, &ok ) )
    {
        std::lock_guard<std::mutex> requestLock( m_requestMutex );
        RiaGrpcCallbackInterface*   method = static_cast<RiaGrpcCallbackInterface*>( tag );
        if ( !ok )
        {
            method->setNextCallState( RiaGrpcCallbackInterface::FINISH_REQUEST );
        }
        m_unprocessedRequests.push_back( method );
    }
}

//--------------------------------------------------------------------------------------------------
/// The handling of calls pushed onto the command queue. We only get one queued callback per client request.
/// The gRPC calls triggered in the callback will see each callback pushed back onto the command queue.
/// The call state will then determine what the callback should do next.
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::process( RiaGrpcCallbackInterface* method )
{
    if ( method->callState() == RiaGrpcCallbackInterface::CREATE_HANDLER )
    {
        method->createRequestHandler( m_completionQueue.get() );
    }
    else if ( method->callState() == RiaGrpcCallbackInterface::INIT_REQUEST_STARTED )
    {
        method->onInitRequestStarted();
    }
    else if ( method->callState() == RiaGrpcCallbackInterface::INIT_REQUEST_COMPLETED )
    {
        method->onInitRequestCompleted();
    }
    else if ( method->callState() == RiaGrpcCallbackInterface::PROCESS_REQUEST )
    {
        method->onProcessRequest();
    }
    else
    {
        method->onFinishRequest();
        process( method->createNewFromThis() );
        delete method;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServer::RiaGrpcServer( int portNumber )
{
    m_serverImpl = new RiaGrpcServerImpl( portNumber );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServer::~RiaGrpcServer()
{
    delete m_serverImpl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaGrpcServer::portNumber() const
{
    if ( m_serverImpl ) return m_serverImpl->portNumber();

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGrpcServer::isRunning() const
{
    if ( m_serverImpl ) return m_serverImpl->isRunning();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::run()
{
    CVF_ASSERT( m_serverImpl );

    m_serverImpl->run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::runInThread()
{
    CVF_ASSERT( m_serverImpl );

    m_serverImpl->runInThread();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGrpcServer::s_receivedExitRequest = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::initialize()
{
    CVF_ASSERT( m_serverImpl );

    m_serverImpl->initialize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaGrpcServer::processAllQueuedRequests()
{
    CVF_ASSERT( m_serverImpl );

    return m_serverImpl->processAllQueuedRequests();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::quit()
{
    if ( m_serverImpl ) m_serverImpl->quit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaGrpcServer::findAvailablePortNumber( int defaultPortNumber )
{
    int startPort = 50051;

    if ( defaultPortNumber > 0 && defaultPortNumber < (int)std::numeric_limits<quint16>::max() )
    {
        startPort = defaultPortNumber;
    }

    int endPort = std::min( startPort + 100, (int)std::numeric_limits<quint16>::max() );

    QTcpServer serverTest;
    quint16    port = static_cast<quint16>( startPort );
    for ( ; port <= static_cast<quint16>( endPort ); ++port )
    {
        if ( serverTest.listen( QHostAddress::LocalHost, port ) )
        {
            return static_cast<int>( port );
        }
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::setReceivedExitRequest()
{
    RiaLogging::info( "Received Exit Request" );
    s_receivedExitRequest = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGrpcServer::receivedExitRequest()
{
    return s_receivedExitRequest;
}
