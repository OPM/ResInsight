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

#include "RiaApplication.h"
#include "RiaDefines.h"

#include "RiaGrpcServerCallData.h"
#include "RiaGrpcServiceInterface.h"
#include "RiaGrpcGridInfoService.h"

#include "RigCaseCellResultsData.h"
#include "RigMainGrid.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

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
    ~RiaGrpcServerImpl();
    void run();
    void runInThread();
    void initialize();
    void processOneRequest();
    void quit();

private:
    void waitForNextRequest();
    void process(RiaGrpcServerCallMethod* method);

private:
    std::unique_ptr<grpc::ServerCompletionQueue>        m_completionQueue;
    std::unique_ptr<grpc::Server>                       m_server;
    std::list<std::shared_ptr<RiaGrpcServiceInterface>> m_services;
    std::list<RiaGrpcServerCallMethod*>                 m_unprocessedRequests;
    std::mutex                                          m_requestMutex;
    std::thread                                         m_thread;
};

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
void RiaGrpcServerImpl::run()
{
    initialize();
    while (true)
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
    m_thread = std::thread(&RiaGrpcServerImpl::waitForNextRequest, this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::initialize()
{
    quint16 port = 50051u;
    {
        QTcpServer serverTest;
        while (!serverTest.listen(QHostAddress::LocalHost, port))
        {
            port++;
        }
    }

    QString serverAddress = QString("localhost:%1").arg(port);

    ServerBuilder builder;
    builder.AddListeningPort(serverAddress.toStdString(), grpc::InsecureServerCredentials());

    for (auto key : RiaGrpcServiceFactory::instance()->allKeys())
    {
        std::shared_ptr<RiaGrpcServiceInterface> service(RiaGrpcServiceFactory::instance()->create(key));
        builder.RegisterService(dynamic_cast<grpc::Service*>(service.get()));
        m_services.push_back(service);
    }

    m_completionQueue = builder.AddCompletionQueue();
    m_server = builder.BuildAndStart();

    CVF_ASSERT(m_server);
    RiaLogging::info(QString("Server listening on %1").arg(serverAddress));
    
    // Spawn new CallData instances to serve new clients.
    for (auto service : m_services)
    {
        for (auto callback : service->createCallbacks())
        {
            process(callback);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::processOneRequest()
{
    std::lock_guard<std::mutex> requestLock(m_requestMutex);
    if (!m_unprocessedRequests.empty())
    {
        RiaGrpcServerCallMethod* method = m_unprocessedRequests.front();
        m_unprocessedRequests.pop_front();
        process(method);
    }
}

//--------------------------------------------------------------------------------------------------
/// Gracefully shut down the GRPC server. The internal order is important.
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::quit()
{
    if (m_server)
    {
        // Clear unhandled requests
        while (!m_unprocessedRequests.empty())
        {
            RiaGrpcServerCallMethod* method = m_unprocessedRequests.front();
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
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::waitForNextRequest()
{
    void* tag;
    bool  ok = false;

    while (m_completionQueue->Next(&tag, &ok))
    {
        RiaGrpcServerCallMethod* method = static_cast<RiaGrpcServerCallMethod*>(tag);
        if (ok)
        {
            std::lock_guard<std::mutex> requestLock(m_requestMutex);
            m_unprocessedRequests.push_back(method);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServerImpl::process(RiaGrpcServerCallMethod* method)
{
    if (method->callState() == RiaGrpcServerCallMethod::CREATE_HANDLER)
    {
        RiaLogging::debug(QString("Initialising request handler for: %1").arg(method->name()));
        method->createRequestHandler(m_completionQueue.get());
    }
    else if (method->callState() == RiaGrpcServerCallMethod::INIT_REQUEST)
    {
        // Perform initialization and immediately process the first request
        // The initialization is necessary for streaming services.
        RiaLogging::info(QString("Starting handling: %1").arg(method->name()));
        method->initRequest();
        method->processRequest();
    }
    else if (method->callState() == RiaGrpcServerCallMethod::PROCESS_REQUEST)
    {
        method->processRequest();       
    }
    else
    {
        RiaLogging::info(QString("Finished handling: %1").arg(method->name()));
        method->finishRequest();
        process(method->createNewFromThis());
        delete method;
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServer::RiaGrpcServer()
{
    m_serverImpl = new RiaGrpcServerImpl;
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
void RiaGrpcServer::run()
{
    m_serverImpl->run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::runInThread()
{
    m_serverImpl->runInThread();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::initialize()
{
    m_serverImpl->initialize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::processOneRequest()
{
    m_serverImpl->processOneRequest();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::quit()
{
    m_serverImpl->quit();
}
