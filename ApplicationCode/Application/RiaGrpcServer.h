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
using grpc::CompletionQueue;
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


class RiaGridServiceImpl final : public ResInsight::Grid::AsyncService
{
public:
    RimEclipseCase* getCase(int caseId) const;

    Status dimensions(ServerContext* context, const Case* request, Vec3i* reply) override;
    Status results(ServerContext* context, const EclipseResultRequest* request, DoubleResult* result) override;
    Status numberOfTimeSteps(ServerContext* context, const Case* request, Int32Message* reply) override;
};

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
    virtual RiaGrpcServerCallMethod* clone() const = 0;
    virtual void                     createRequest() = 0;
    virtual Status                   processRequest() = 0;
    virtual void                     finishRequest() {}

    const std::string& methodName() const
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
    typedef ServerAsyncResponseWriter<ReplyT> ResponseWriterT;
    typedef std::function<Status(RiaGridServiceImpl&, ServerContext*, const RequestT*, ReplyT*)> MethodImpl;
    typedef std::function<void(RiaGridServiceImpl&, ServerContext*, RequestT*, ResponseWriterT*, CompletionQueue*, ServerCompletionQueue*, void*)> RequestImpl;

    RiaGrpcServerCallData(RiaGridServiceImpl* service, ServerCompletionQueue* cq, const std::string& methodName, MethodImpl methodImpl, RequestImpl methodRequest)
        : RiaGrpcServerCallMethod(methodName)
        , m_service(service)
        , m_completionQueue(cq)
        , m_responder(&m_context)
        , m_methodImpl(methodImpl)
        , m_methodRequest(methodRequest)

    {
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

    RiaGrpcServerCallMethod* clone() const override
    {
        return new RiaGrpcServerCallData<RequestT, ReplyT>(m_service, m_completionQueue, methodName(), m_methodImpl, m_methodRequest);
    }

    void createRequest() override
    {
        m_methodRequest(*m_service, &m_context, &m_request, &m_responder, m_completionQueue, m_completionQueue, this);
    }

    Status processRequest() override
    {
        Status status = m_methodImpl(*m_service, &m_context, &m_request, &m_reply);
        responder().Finish(m_reply, status, this);
        return status;
    }

    ServerAsyncResponseWriter<ReplyT>& responder()
    {
        return m_responder;
    }

private:
    RiaGridServiceImpl*               m_service;
    ServerCompletionQueue*            m_completionQueue;
    ServerContext                     m_context;
    RequestT                          m_request;
    ReplyT                            m_reply;
    ServerAsyncResponseWriter<ReplyT> m_responder;
    MethodImpl                        m_methodImpl;
    RequestImpl                       m_methodRequest;
};

class RiaGrpcServer
{
public:
    ~RiaGrpcServer();
    void run();
    void runInThread();
    void initialize();
    void processOneRequest();
private:
    void waitForNextRequest();
    void process(RiaGrpcServerCallMethod* method);

private:
    std::unique_ptr<ServerCompletionQueue> m_completionQueue;
    std::unique_ptr<Server>                m_server;
    RiaGridServiceImpl                     m_service;
    std::list<RiaGrpcServerCallMethod*>    m_receivedRequests;
    std::thread                            m_thread;    
    std::mutex                             m_requestMutex;
};


#ifdef WIN32
#pragma warning(pop)
#endif
