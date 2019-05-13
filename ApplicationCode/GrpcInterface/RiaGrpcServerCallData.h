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

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <QString>

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
        CREATE_HANDLER,
        INIT_REQUEST,
        PROCESS_REQUEST,
        FINISH_REQUEST
    };

public:
    inline RiaGrpcServerCallMethod();

    virtual ~RiaGrpcServerCallMethod() {}
    virtual QString                  name() const                                                 = 0;
    virtual RiaGrpcServerCallMethod* createNewFromThis() const                                    = 0;
    virtual void                     createRequestHandler(ServerCompletionQueue* completionQueue) = 0;
    virtual void                     initRequest()                                                = 0;
    virtual void                     processRequest()                                             = 0;
    virtual void                     finishRequest() {}

    inline CallState     callState() const;
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

    QString          name() const override;
    const RequestT&  request() const;
    ReplyT&          reply();

protected:
    virtual QString methodType() const = 0;

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
    void                     createRequestHandler(ServerCompletionQueue* completionQueue) override;
    void                     initRequest() override;
    void                     processRequest() override;

protected:
    virtual QString          methodType() const;

private:
    ResponseWriterT          m_responder;
    MethodImpl               m_methodImpl;
    MethodRequest            m_methodRequest;
};


//==================================================================================================
//
// Templated *streaming* gRPC-callback calling service implementation callbacks
//
// The streaming callback needs a state handler for setting up and maintaining order.
//
// A fully functional stream handler needs to implement the following methods:
// 1. Default Constructor
// 2. grpc::Status init(const grpc::Message* request)
// 3. grpc::status assignReply(grpc::Message* reply)
//
//==================================================================================================

//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
class RiaGrpcServerStreamingCallData : public RiaGrpcServerAbstractCallData<ServiceT, RequestT, ReplyT>
{
public:
    typedef ServerAsyncWriter<ReplyT>                      ResponseWriterT;

    typedef std::function<
        Status(ServiceT&, ServerContext*, const RequestT*, ReplyT*, StateHandlerT*)> MethodImpl;
    typedef std::function<
        void(ServiceT&, ServerContext*, RequestT*, ResponseWriterT*,
            CompletionQueue*, ServerCompletionQueue*, void*)>                       MethodRequest;


    RiaGrpcServerStreamingCallData(ServiceT* service, MethodImpl methodImpl, MethodRequest methodRequest, StateHandlerT* stateHandler);

    RiaGrpcServerCallMethod* createNewFromThis() const override;
    void                     createRequestHandler(ServerCompletionQueue* completionQueue) override;
    void                     initRequest() override;
    void                     processRequest() override;
protected:
    virtual QString          methodType() const;

private:
    ResponseWriterT                         m_responder;
    MethodImpl                              m_methodImpl;
    MethodRequest                           m_methodRequest;
    size_t                                  m_dataCount; // This is used to keep track of progress. Only one item is sent for each invocation.
    std::unique_ptr<StateHandlerT>          m_stateHandler;
};

#include "RiaGrpcServerCallData.inl"
