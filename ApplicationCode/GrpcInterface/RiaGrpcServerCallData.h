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
    enum CallStatus
    {
        CREATE,
        PROCESS,
        FINISH
    };

public:
    RiaGrpcServerCallMethod()
        : m_status(CREATE)
    {
    }

    virtual ~RiaGrpcServerCallMethod() {}
    virtual const char*              name() const                                          = 0;
    virtual RiaGrpcServerCallMethod* createNewFromThis() const = 0;
    virtual void                     createRequest(ServerCompletionQueue* completionQueue) = 0;
    virtual Status                   processRequest()                                      = 0;
    virtual void                     finishRequest() {}

    CallStatus& callStatus()
    {
        return m_status;
    }

private:
    CallStatus  m_status;
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

    const char* name() const override;
    const RequestT&  request() const;

protected:
    ServiceT*       m_service;
    ServerContext   m_context;
    RequestT        m_request;

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
    Status                   processRequest() override;

    ReplyT&                  reply();

private:
    ReplyT                            m_reply;
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
    typedef ServerWriter<ReplyT>      ServerWriterT;

    typedef std::function<
        Status(ServiceT&, ServerContext*, const RequestT*, ServerWriterT*)> MethodImpl;
    typedef std::function<
        void(ServiceT&, ServerContext*, RequestT*, ResponseWriterT*,
            CompletionQueue*, ServerCompletionQueue*, void*)>               MethodRequest;

    RiaGrpcServerStreamingCallData(ServiceT* service, MethodImpl methodImpl, MethodRequest methodRequest);

    RiaGrpcServerCallMethod* createNewFromThis() const override;
    void                     createRequest(ServerCompletionQueue* completionQueue) override;
    Status                   processRequest() override;
    void                     finishRequest() override;

private:
    ServerWriterT                     m_serverWriter;
    ResponseWriterT                   m_responder;
    MethodImpl                        m_methodImpl;
    MethodRequest                     m_methodRequest;
};

#include "RiaGrpcServerCallData.inl"
