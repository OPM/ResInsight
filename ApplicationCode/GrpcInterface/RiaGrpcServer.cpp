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
#ifdef WIN32
// GRPC does a lot of tricks that give warnings on MSVC but works fine
#pragma warning(push)
#pragma warning(disable : 4251 4702 4005 4244)
#endif

#include "RiaGrpcServer.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RigCaseCellResultsData.h"
#include "RigMainGrid.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "grpc/impl/codegen/gpr_types.h"

#include <QTcpServer>

using grpc::CompletionQueue;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServer::~RiaGrpcServer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::run()
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
void RiaGrpcServer::runInThread()
{
    initialize();
    std::thread(&RiaGrpcServer::waitForNextRequest, this).detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::initialize()
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

    builder.RegisterService(&m_projectService);
    builder.RegisterService(&m_gridService);

    m_completionQueue = builder.AddCompletionQueue();
    m_server = builder.BuildAndStart();

    CVF_ASSERT(m_server);
    RiaLogging::info(QString("Server listening on %1").arg(serverAddress));
    // Spawn new CallData instances to serve new clients.
    for (auto callback : m_projectService.createCallbacks(m_completionQueue.get()))
    {
        process(callback);
    }
    for (auto callback : m_gridService.createCallbacks(m_completionQueue.get()))
    {
        process(callback);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::processOneRequest()
{
    std::lock_guard<std::mutex> requestLock(m_requestMutex);
    if (!m_receivedRequests.empty())
    {
        RiaGrpcServerCallMethod* method = m_receivedRequests.front();
        m_receivedRequests.pop_front();
        process(method);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::quit()
{
    m_server->Shutdown();
    m_completionQueue->Shutdown();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::waitForNextRequest()
{
    void* tag;
    bool  ok = false;

    while (m_completionQueue->Next(&tag, &ok))
    {
        RiaGrpcServerCallMethod* method = static_cast<RiaGrpcServerCallMethod*>(tag);
        if (!ok)
        {
            method->callStatus() = RiaGrpcServerCallMethod::FINISH;
            process(method);
        }
        std::lock_guard<std::mutex> requestLock(m_requestMutex);
        m_receivedRequests.push_back(method);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::process(RiaGrpcServerCallMethod* method)
{
    if (method->callStatus() == RiaGrpcServerCallMethod::CREATE)
    {
        method->callStatus() = RiaGrpcServerCallMethod::PROCESS;
        method->createRequest();
    }
    else if (method->callStatus() == RiaGrpcServerCallMethod::PROCESS)
    {
        method->callStatus() = RiaGrpcServerCallMethod::FINISH;
        method->processRequest();
        process(method->clone());
    }
    else
    {
        delete method;
    }
}

#ifdef WIN32
#pragma warning(pop)
#endif
