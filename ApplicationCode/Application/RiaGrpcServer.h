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
#pragma once

#include "RiaLogging.h"

#include <QString>

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#ifdef WIN32
// GRPC does a lot of tricks that give warnings on MSVC but works fine
#pragma warning(push)
#pragma warning(disable : 4251 4702 4005)
#endif

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <mutex>

#include "ResInsightGrid.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using ResInsight::Grid;
using ResInsight::Case;
using ResInsight::Int32Message;
using ResInsight::Vec3i;
using ResInsight::EclipseResultRequest;
using ResInsight::EclipseResultAddress;
using ResInsight::DoubleResult;

class RimEclipseCase;

class RiaGrpcServerCallMethod
{
public:
    enum CallStatus
    {
        CREATE,
        PROCESS,
        FINISH
    };

public:
    RiaGrpcServerCallMethod(const std::string& methodName)
        : m_methodName(methodName)
        , m_status(CREATE)
    {
    }

    virtual ~RiaGrpcServerCallMethod() {}

    const std::string& callMethodName() const
    {
        return m_methodName;
    }
    CallStatus& callStatus()
    {
        return m_status;
    }

private:
    std::string m_methodName;
    CallStatus  m_status;
};

template<typename RequestT, typename ReplyT>
class RiaGrpcServerCallData : public RiaGrpcServerCallMethod
{
public:
    RiaGrpcServerCallData(const std::string& methodName)
        : RiaGrpcServerCallMethod(methodName)
        , m_responder(&m_context)
    {
        RiaLogging::info(QString("Listening for %1").arg(QString::fromStdString(methodName)));
    }

    ServerContext& context()
    {
        return m_context;
    }
    RequestT& request()
    {
        return m_request;
    }
    ReplyT& reply()
    {
        return m_reply;
    }

    ServerAsyncResponseWriter<ReplyT>& responder()
    {
        return m_responder;
    }

private:
    ServerContext                     m_context;
    RequestT                          m_request;
    ReplyT                            m_reply;
    ServerAsyncResponseWriter<ReplyT> m_responder;
};

class RiaGridServiceImpl final : public ResInsight::Grid::AsyncService
{
public:
    RimEclipseCase* getCase(int caseId) const;

    Status dimensions(ServerContext* context, const Case* request, Vec3i* reply) override;
    Status results(ServerContext* context, const EclipseResultRequest* request, DoubleResult* result) override;
    Status numberOfTimeSteps(ServerContext* context, const Case* request, Int32Message* reply) override;
};

class RiaGrpcServer
{
public:
    ~RiaGrpcServer();
    void run();
    void runInThread();
    void initialize();
private:
    void blockForNextRequest();
    void blockForNextRequestAsync();
    void process(RiaGrpcServerCallMethod* method);

private:
    std::unique_ptr<ServerCompletionQueue> m_commandQueue;
    std::unique_ptr<Server>                m_server;
    RiaGridServiceImpl                     m_service;
    std::list<RiaGrpcServerCallMethod*>    m_callMethods;
    std::thread                            m_thread;
};


#ifdef WIN32
#pragma warning(pop)
#endif
