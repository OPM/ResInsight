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
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

using grpc::CompletionQueue;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncReader;
using grpc::ServerAsyncWriter;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

class RiaGrpcServiceInterface;

//==================================================================================================
//
// Base class for all gRPC-callbacks
//
//==================================================================================================
class RiaAbstractGrpcCallback
{
public:
    enum CallState
    {
        CREATE_HANDLER,
        INIT_REQUEST_STARTED,
        INIT_REQUEST_COMPLETED,
        PROCESS_REQUEST,
        FINISH_REQUEST
    };

public:
    inline RiaAbstractGrpcCallback();

    virtual ~RiaAbstractGrpcCallback() {}
    virtual QString                  name() const                                                 = 0;
    virtual RiaAbstractGrpcCallback* createNewFromThis() const                                    = 0;
    virtual void                     createRequestHandler(ServerCompletionQueue* completionQueue) = 0;
    virtual void                     onInitRequestStarted() {}
    virtual void                     onInitRequestCompleted()                                     = 0;
    virtual void                     onProcessRequest()                                           = 0;
    virtual void                     onFinishRequest() {}

    inline CallState     callState() const;
    inline const Status& status() const;

protected:
    inline void setCallState(CallState state);

protected:
    CallState m_state;
    Status    m_status;
};

//==================================================================================================
//
// Templated gRPC-callback base class
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcRequestCallback : public RiaAbstractGrpcCallback
{
public:
    RiaGrpcRequestCallback(ServiceT* service);

    QString         name() const override;
    const RequestT& request() const;
    ReplyT&         reply();

protected:
    virtual QString methodType() const = 0;

protected:
    ServiceT*     m_service;
    RequestT      m_request;
    ReplyT        m_reply;
};

//==================================================================================================
//
// Templated gRPC-callback for non-streaming services
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcCallback : public RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>
{
public:
    typedef ServerAsyncResponseWriter<ReplyT>                                          ResponseWriterT;
    typedef std::function<Status(ServiceT&, ServerContext*, const RequestT*, ReplyT*)> MethodImplT;
    typedef std::function<
        void(ServiceT&, ServerContext*, RequestT*, ResponseWriterT*, CompletionQueue*, ServerCompletionQueue*, void*)>
        MethodRequestT;

    RiaGrpcCallback(ServiceT* service, MethodImplT methodImpl, MethodRequestT methodRequest);

    RiaAbstractGrpcCallback* createNewFromThis() const override;
    void                     createRequestHandler(ServerCompletionQueue* completionQueue) override;
    void                     onInitRequestCompleted() override;
    void                     onProcessRequest() override;

protected:
    virtual QString methodType() const;

private:
    ServerContext   m_context;
    ResponseWriterT m_responder;
    MethodImplT     m_methodImpl;
    MethodRequestT  m_methodRequest;
};

//==================================================================================================
//
// Templated server *streaming* gRPC-callback calling service implementation callbacks
//
// The streaming callback needs a state handler for setting up and maintaining order.
//
// A fully functional state handler needs to implement the following methods:
// 1. Default Constructor
// 2. grpc::Status init(const grpc::Message* request)
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
class RiaGrpcStreamCallback : public RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>
{
public:
    typedef ServerAsyncWriter<ReplyT>                                                                  ResponseWriterT;
    typedef std::function<Status(ServiceT&, ServerContext*, const RequestT*, ReplyT*, StateHandlerT*)> MethodImplT;
    typedef std::function<
        void(ServiceT&, ServerContext*, RequestT*, ResponseWriterT*, CompletionQueue*, ServerCompletionQueue*, void*)>
        MethodRequestT;

    RiaGrpcStreamCallback(ServiceT* service, MethodImplT methodImpl, MethodRequestT methodRequest, StateHandlerT* stateHandler);

    RiaAbstractGrpcCallback* createNewFromThis() const override;
    void                     createRequestHandler(ServerCompletionQueue* completionQueue) override;
    void                     onInitRequestCompleted() override;
    void                     onProcessRequest() override;

protected:
    virtual QString methodType() const;

private:
    ServerContext   m_context;
    ResponseWriterT m_responder;
    MethodImplT     m_methodImpl;
    MethodRequestT  m_methodRequest;
    std::unique_ptr<StateHandlerT> m_stateHandler;
};

//==================================================================================================
//
// Templated client *streaming* gRPC-callback calling service implementation callbacks
//
// The streaming callback needs a state handler for setting up and maintaining order.
//
// A fully functional state handler needs to implement the following methods:
// 1. Default Constructor
// 2. grpc::Status init(const grpc::Message* request)
// 3. void finish() any updates required after completion
//
//==================================================================================================
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
class RiaGrpcClientStreamCallback : public RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>
{
public:
    typedef ServerAsyncReader<ReplyT, RequestT>                                                        RequestReaderT;
    typedef std::function<Status(ServiceT&, ServerContext*, const RequestT*, ReplyT*, StateHandlerT*)> MethodImplT;
    typedef std::function<
        void(ServiceT&, ServerContext*, RequestReaderT*, CompletionQueue*, ServerCompletionQueue*, void*)>
        MethodRequestT;

    RiaGrpcClientStreamCallback(ServiceT* service, MethodImplT methodImpl, MethodRequestT methodRequest, StateHandlerT* stateHandler);

    RiaAbstractGrpcCallback* createNewFromThis() const override;
    void                     createRequestHandler(ServerCompletionQueue* completionQueue) override;
    void                     onInitRequestStarted() override;
    void                     onInitRequestCompleted() override;
    void                     onProcessRequest() override;
    void                     onFinishRequest() override;

protected:
    virtual QString methodType() const;

private:
    ServerContext   m_context;
    RequestReaderT  m_reader;
    MethodImplT     m_methodImpl;
    MethodRequestT  m_methodRequest;
    std::unique_ptr<StateHandlerT> m_stateHandler;
};

#include "RiaGrpcCallbacks.inl"
