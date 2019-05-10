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

#include "RiaLogging.h"

using grpc::CompletionQueue;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncWriter;
using grpc::ServerWriter;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

class RiaGrpcServiceInterface;

//==================================================================================================
//
// Base class for all gRPC-callbacks
//
//==================================================================================================
class RiaGrpcServerCallMethod
{
public:
    enum CallState
    {
        CREATE,
        PROCESS,
        FINISH
    };

public:
    inline RiaGrpcServerCallMethod();

    virtual ~RiaGrpcServerCallMethod() {}
    virtual const char*              name() const                                          = 0;
    virtual RiaGrpcServerCallMethod* createNewFromThis() const = 0;
    virtual void                     createRequest(ServerCompletionQueue* completionQueue) = 0;
    virtual void                     processRequest()                                      = 0;
    virtual void                     finishRequest() {}

    inline CallState callState() const;
    inline const Status& status() const;

protected:
    inline void setCallState(CallState state);

protected:
    CallState  m_state;
    Status     m_status;
};

//==================================================================================================
//
// Templated gRPC-callback
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcServerAbstractCallData : public RiaGrpcServerCallMethod
{
public:
    RiaGrpcServerAbstractCallData(ServiceT* service);

    const char*      name() const override;
    const RequestT&  request() const;
    ReplyT&          reply();

protected:
    ServiceT*       m_service;
    ServerContext   m_context;
    RequestT        m_request;
    ReplyT          m_reply;
};

//==================================================================================================
//
// Templated gRPC-callback for non-streaming services
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcServerCallData : public RiaGrpcServerAbstractCallData<ServiceT, RequestT, ReplyT>
{
public:
	typedef ServerAsyncResponseWriter<ReplyT>                                          ResponseWriterT;
    typedef std::function<Status(ServiceT&, ServerContext*, const RequestT*, ReplyT*)> MethodImpl;
    typedef std::function<void(ServiceT&, ServerContext*, RequestT*, ResponseWriterT*,
        CompletionQueue*, ServerCompletionQueue*, void*)>                              MethodRequest;

    RiaGrpcServerCallData(ServiceT*              service,
                          MethodImpl             methodImpl,
                          MethodRequest          methodRequest);

    RiaGrpcServerCallMethod* createNewFromThis() const override;
    void                     createRequest(ServerCompletionQueue* completionQueue) override;
    void                     processRequest() override;

private:
    ResponseWriterT                   m_responder;
    MethodImpl                        m_methodImpl;
    MethodRequest                     m_methodRequest;
};

//==================================================================================================
//
// Templated *streaming* gRPC-callback calling service implementation callbacks
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcServerStreamingCallData : public RiaGrpcServerAbstractCallData<ServiceT, RequestT, ReplyT>
{
public:
    typedef ServerAsyncWriter<ReplyT> ResponseWriterT;

    typedef std::function<
        Status(ServiceT&, ServerContext*, const RequestT*, ReplyT*, size_t*)> MethodImpl;
    typedef std::function<
        void(ServiceT&, ServerContext*, RequestT*, ResponseWriterT*,
            CompletionQueue*, ServerCompletionQueue*, void*)>               MethodRequest;

    RiaGrpcServerStreamingCallData(ServiceT* service, MethodImpl methodImpl, MethodRequest methodRequest);

    RiaGrpcServerCallMethod* createNewFromThis() const override;
    void                     createRequest(ServerCompletionQueue* completionQueue) override;
    void                     processRequest() override;

private:
    ResponseWriterT                   m_responder;
    MethodImpl                        m_methodImpl;
    MethodRequest                     m_methodRequest;
    size_t                            m_dataCount; // This is used to keep track of progress. Only one item is sent for each invocation.
};

#include "RiaGrpcServerCallData.inl"
