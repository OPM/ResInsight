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

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

using grpc::CompletionQueue;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

//==================================================================================================
//
// Base class for all gRPC-callbacks
//
//==================================================================================================
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
    virtual RiaGrpcServerCallMethod* clone() const    = 0;
    virtual void                     createRequest()  = 0;
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

//==================================================================================================
//
// Templated gRPC-callback calling service implementation callbacks
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcServerCallData : public RiaGrpcServerCallMethod
{
public:
    typedef ServerAsyncResponseWriter<ReplyT>                                          ResponseWriterT;
    typedef std::function<Status(ServiceT&, ServerContext*, const RequestT*, ReplyT*)> MethodImpl;
    typedef std::function<
        void(ServiceT&, ServerContext*, RequestT*, ResponseWriterT*, CompletionQueue*, ServerCompletionQueue*, void*)>
        RequestImpl;

    RiaGrpcServerCallData(ServiceT*              service,
                          ServerCompletionQueue* cq,
                          const std::string&     methodName,
                          MethodImpl             methodImpl,
                          RequestImpl            methodRequest);

    ServerContext& context();
    RequestT& request();
    ReplyT& reply();

    RiaGrpcServerCallMethod* clone() const override;

    void createRequest() override;

    Status processRequest() override;

    ServerAsyncResponseWriter<ReplyT>& responder();

private:
    ServiceT*                         m_service;
    ServerCompletionQueue*            m_completionQueue;
    ServerContext                     m_context;
    RequestT                          m_request;
    ReplyT                            m_reply;
    ServerAsyncResponseWriter<ReplyT> m_responder;
    MethodImpl                        m_methodImpl;
    RequestImpl                       m_methodRequest;
};

#include "RiaGrpcServerCallData.inl"
