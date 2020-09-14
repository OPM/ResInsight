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
#include <grpcpp/server_context.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/sync_stream.h>

using grpc::CompletionQueue;
using grpc::ServerAsyncReader;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncWriter;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

class RiaGrpcServiceInterface;

//==================================================================================================
//
// Base class for all gRPC-callbacks
//
//==================================================================================================
class RiaGrpcCallbackInterface
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
    inline RiaGrpcCallbackInterface();

    virtual ~RiaGrpcCallbackInterface() {}
    virtual QString                   name() const                                                   = 0;
    virtual RiaGrpcCallbackInterface* createNewFromThis() const                                      = 0;
    virtual void                      createRequestHandler( ServerCompletionQueue* completionQueue ) = 0;
    virtual void                      onInitRequestStarted() {}
    virtual void                      onInitRequestCompleted() {}
    virtual void                      onProcessRequest() = 0;
    virtual void                      onFinishRequest() {}

    inline CallState     callState() const;
    inline const Status& status() const;
    inline void          setNextCallState( CallState state );

protected:
    CallState m_state;
    Status    m_status;
};

//==================================================================================================
//
// Templated gRPC-callback base class
//
//==================================================================================================
template <typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcServiceCallback : public RiaGrpcCallbackInterface
{
public:
    RiaGrpcServiceCallback( ServiceT* service );

    QString         name() const override;
    const RequestT& request() const;
    ReplyT&         reply();

protected:
    virtual QString methodType() const = 0;

protected:
    ServiceT* m_service;
    RequestT  m_request;
    ReplyT    m_reply;
};

//==================================================================================================
//
// Templated gRPC-callback for non-streaming services
//
//==================================================================================================
template <typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcUnaryCallback : public RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>
{
public:
    using ResponseWriterT = ServerAsyncResponseWriter<ReplyT>;
    using MethodImplT     = std::function<Status( ServiceT&, ServerContext*, const RequestT*, ReplyT* )>;
    using MethodRequestT  = std::function<
        void( ServiceT&, ServerContext*, RequestT*, ResponseWriterT*, CompletionQueue*, ServerCompletionQueue*, void* )>;

    RiaGrpcUnaryCallback( ServiceT* service, MethodImplT methodImpl, MethodRequestT methodRequest );

    RiaGrpcCallbackInterface* createNewFromThis() const override;
    void                      createRequestHandler( ServerCompletionQueue* completionQueue ) override;
    void                      onProcessRequest() override;

protected:
    virtual QString methodType() const override;

private:
    ServerContext   m_context;
    ResponseWriterT m_responder;
    MethodImplT     m_methodImpl;
    MethodRequestT  m_methodRequest;
};

//==================================================================================================
//
// Templated server->client *streaming* gRPC-callback calling service implementation callbacks
//
// The streaming callback needs a state handler for setting up and maintaining order.
//
// A fully functional state handler for server->client streaming needs to implement the following methods:
// 1. Default Constructor
// 2. grpc::Status init(const grpc::Message* request)
//
//==================================================================================================
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
class RiaGrpcServerToClientStreamCallback : public RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>
{
public:
    using ResponseWriterT = ServerAsyncWriter<ReplyT>;
    using MethodImplT    = std::function<Status( ServiceT&, ServerContext*, const RequestT*, ReplyT*, StateHandlerT* )>;
    using MethodRequestT = std::function<
        void( ServiceT&, ServerContext*, RequestT*, ResponseWriterT*, CompletionQueue*, ServerCompletionQueue*, void* )>;

    RiaGrpcServerToClientStreamCallback( ServiceT*      service,
                                         MethodImplT    methodImpl,
                                         MethodRequestT methodRequest,
                                         StateHandlerT* stateHandler );

    RiaGrpcCallbackInterface* createNewFromThis() const override;
    void                      createRequestHandler( ServerCompletionQueue* completionQueue ) override;
    void                      onInitRequestCompleted() override;
    void                      onProcessRequest() override;

protected:
    virtual QString methodType() const override;

private:
    ServerContext                  m_context;
    ResponseWriterT                m_responder;
    MethodImplT                    m_methodImpl;
    MethodRequestT                 m_methodRequest;
    std::unique_ptr<StateHandlerT> m_stateHandler;
};

//==================================================================================================
//
// Templated client->server *streaming* gRPC-callback calling service implementation callbacks
//
// The streaming callback needs a state handler for setting up and maintaining order.
//
// A fully functional state handler for client->server streaming needs to implement the following methods:
// 1. Default Constructor
// 2. grpc::Status init(const grpc::Message* request)
// 3. void finish() any updates required after completion
//
//==================================================================================================
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
class RiaGrpcClientToServerStreamCallback : public RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>
{
public:
    using RequestReaderT = ServerAsyncReader<ReplyT, RequestT>;
    using MethodImplT    = std::function<Status( ServiceT&, ServerContext*, const RequestT*, ReplyT*, StateHandlerT* )>;
    using MethodRequestT =
        std::function<void( ServiceT&, ServerContext*, RequestReaderT*, CompletionQueue*, ServerCompletionQueue*, void* )>;

    RiaGrpcClientToServerStreamCallback( ServiceT*      service,
                                         MethodImplT    methodImpl,
                                         MethodRequestT methodRequest,
                                         StateHandlerT* stateHandler );

    RiaGrpcCallbackInterface* createNewFromThis() const override;
    void                      createRequestHandler( ServerCompletionQueue* completionQueue ) override;
    void                      onInitRequestStarted() override;
    void                      onInitRequestCompleted() override;
    void                      onProcessRequest() override;
    void                      onFinishRequest() override;

protected:
    virtual QString methodType() const override;

private:
    ServerContext                  m_context;
    RequestReaderT                 m_reader;
    MethodImplT                    m_methodImpl;
    MethodRequestT                 m_methodRequest;
    std::unique_ptr<StateHandlerT> m_stateHandler;
};

#include "RiaGrpcCallbacks.inl"
