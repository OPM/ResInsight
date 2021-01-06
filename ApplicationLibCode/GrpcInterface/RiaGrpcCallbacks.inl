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

inline RiaGrpcCallbackInterface::RiaGrpcCallbackInterface()
    : m_state( CREATE_HANDLER )
    , m_status( Status::OK )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcCallbackInterface::CallState RiaGrpcCallbackInterface::callState() const
{
    return m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Status& RiaGrpcCallbackInterface::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
inline void RiaGrpcCallbackInterface::setNextCallState( CallState state )
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>::RiaGrpcServiceCallback( ServiceT* service )
    : m_service( service )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
QString RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>::name() const
{
    QString fullName = QString( "%1:%2(%3, %4)" )
                           .arg( typeid( ServiceT ).name() )
                           .arg( methodType() )
                           .arg( typeid( RequestT ).name() )
                           .arg( typeid( ReplyT ).name() );
    return fullName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
const RequestT& RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>::request() const
{
    return m_request;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
ReplyT& RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>::reply()
{
    return m_reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcUnaryCallback<ServiceT, RequestT, ReplyT>::RiaGrpcUnaryCallback( ServiceT*      service,
                                                                        MethodImplT    methodImpl,
                                                                        MethodRequestT methodRequest )
    : RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>( service )
    , m_responder( &m_context )
    , m_methodImpl( methodImpl )
    , m_methodRequest( methodRequest )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcCallbackInterface* RiaGrpcUnaryCallback<ServiceT, RequestT, ReplyT>::createNewFromThis() const
{
    return new RiaGrpcUnaryCallback<ServiceT, RequestT, ReplyT>( this->m_service, this->m_methodImpl, this->m_methodRequest );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcUnaryCallback<ServiceT, RequestT, ReplyT>::createRequestHandler( ServerCompletionQueue* completionQueue )
{
    // The Request-method is where the service gets registered to respond to a given request.
    m_methodRequest( *this->m_service, &m_context, &this->m_request, &m_responder, completionQueue, completionQueue, this );
    // Simple unary requests don't need initialisation, so proceed to process as soon as a request turns up.
    this->setNextCallState( RiaGrpcCallbackInterface::PROCESS_REQUEST );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcUnaryCallback<ServiceT, RequestT, ReplyT>::onProcessRequest()
{
    // Call request handler method
    this->m_status = m_methodImpl( *this->m_service, &m_context, &this->m_request, &this->m_reply );
    // Simply unary requests are finished as soon as you've done any processing.
    // So next time we receive a new tag on the command queue we should proceed to finish.
    this->setNextCallState( RiaGrpcCallbackInterface::FINISH_REQUEST );
    // Finish will push this callback back on the command queue (now with Finish as the call state).
    m_responder.Finish( this->m_reply, this->m_status, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
QString RiaGrpcUnaryCallback<ServiceT, RequestT, ReplyT>::methodType() const
{
    return "RegularMethod";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcServerToClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::RiaGrpcServerToClientStreamCallback(
    ServiceT*      service,
    MethodImplT    methodImpl,
    MethodRequestT methodRequest,
    StateHandlerT* stateHandler )
    : RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>( service )
    , m_responder( &m_context )
    , m_methodImpl( methodImpl )
    , m_methodRequest( methodRequest )
    , m_stateHandler( stateHandler )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcCallbackInterface*
    RiaGrpcServerToClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createNewFromThis() const
{
    return new RiaGrpcServerToClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>( this->m_service,
                                                                                               m_methodImpl,
                                                                                               m_methodRequest,
                                                                                               new StateHandlerT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcServerToClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createRequestHandler(
    ServerCompletionQueue* completionQueue )
{
    // The Request-method is where the service gets registered to respond to a given request.
    m_methodRequest( *this->m_service, &m_context, &this->m_request, &m_responder, completionQueue, completionQueue, this );
    // Server->Client Streaming requests require initialisation. However, we receive the complete request immediately.
    // So can proceed directly to completion of the init request.
    this->setNextCallState( RiaGrpcCallbackInterface::INIT_REQUEST_COMPLETED );
}

//--------------------------------------------------------------------------------------------------
/// Perform initialisation tasks at the time of receiving a complete request
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcServerToClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onInitRequestCompleted()
{
    // Initialise streaming state handler
    this->m_status = m_stateHandler->init( &this->m_request );

    if ( !this->m_status.ok() )
    {
        // We have an error. Proceed to finish and report the status
        this->setNextCallState( RiaGrpcCallbackInterface::FINISH_REQUEST );
        m_responder.Finish( this->m_status, this );
        return;
    }

    // Move on to processing and perform the first processing immediately since the client will
    // not request anything more.
    this->setNextCallState( RiaGrpcCallbackInterface::PROCESS_REQUEST );
    this->onProcessRequest();
}

//--------------------------------------------------------------------------------------------------
/// Process a streaming request and send one package
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcServerToClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onProcessRequest()
{
    this->m_reply = ReplyT(); // Make sure it is reset

    // Call request handler method
    this->m_status = m_methodImpl( *this->m_service, &m_context, &this->m_request, &this->m_reply, m_stateHandler.get() );

    if ( this->m_status.ok() )
    {
        // The write call will send data to client AND put this callback back on the command queue
        // so that this method gets called again to send the next stream package.
        m_responder.Write( this->m_reply, this );
    }
    else
    {
        this->setNextCallState( RiaGrpcCallbackInterface::FINISH_REQUEST );
        // Out of range means we're finished but it isn't an error
        if ( this->m_status.error_code() == grpc::OUT_OF_RANGE )
        {
            this->m_status = Status::OK;
        }
        // Finish will put this callback back on the command queue, now with a finish state.
        m_responder.Finish( this->m_status, this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
QString RiaGrpcServerToClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::methodType() const
{
    return "StreamingMethod";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::RiaGrpcClientToServerStreamCallback(
    ServiceT*      service,
    MethodImplT    methodImpl,
    MethodRequestT methodRequest,
    StateHandlerT* stateHandler )
    : RiaGrpcServiceCallback<ServiceT, RequestT, ReplyT>( service )
    , m_reader( &m_context )
    , m_methodImpl( methodImpl )
    , m_methodRequest( methodRequest )
    , m_stateHandler( stateHandler )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcCallbackInterface*
    RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createNewFromThis() const
{
    return new RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>( this->m_service,
                                                                                               m_methodImpl,
                                                                                               m_methodRequest,
                                                                                               new StateHandlerT( true ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createRequestHandler(
    ServerCompletionQueue* completionQueue )
{
    // The Request-method is where the service gets registered to respond to a given request.
    m_methodRequest( *this->m_service, &m_context, &this->m_reader, completionQueue, completionQueue, this );
    // The client->server streaming requires initialisation and each request package is streamed asynchronously
    // So we need to start and complete the init request.
    this->setNextCallState( RiaGrpcCallbackInterface::INIT_REQUEST_STARTED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onInitRequestStarted()
{
    this->setNextCallState( RiaGrpcCallbackInterface::INIT_REQUEST_COMPLETED );
    // The read call will start reading the request data and push this callback back onto the command queue
    // when the read call is completed.
    m_reader.Read( &this->m_request, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onInitRequestCompleted()
{
    this->setNextCallState( RiaGrpcCallbackInterface::PROCESS_REQUEST );
    // Fully received the stream package so can now init
    this->m_status = m_stateHandler->init( &this->m_request );

    if ( !this->m_status.ok() )
    {
        // We have an error. Proceed to finish and report the status
        m_reader.FinishWithError( this->m_status, this );
        this->setNextCallState( RiaGrpcCallbackInterface::FINISH_REQUEST );
        return;
    }

    // Start reading and push this back onto the command queue.
    m_reader.Read( &this->m_request, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onProcessRequest()
{
    this->m_reply = ReplyT(); // Make sure it is reset

    // Call request handler method
    this->m_status = m_methodImpl( *this->m_service, &m_context, &this->m_request, &this->m_reply, m_stateHandler.get() );

    if ( !this->m_status.ok() )
    {
        this->setNextCallState( RiaGrpcCallbackInterface::FINISH_REQUEST );
        m_reader.FinishWithError( this->m_status, this );
    }
    else
    {
        CAF_ASSERT( m_stateHandler->streamedValueCount() <= m_stateHandler->totalValueCount() );
        if ( m_stateHandler->streamedValueCount() == m_stateHandler->totalValueCount() )
        {
            this->setNextCallState( RiaGrpcCallbackInterface::FINISH_REQUEST );
            m_reader.Finish( this->m_reply, grpc::Status::OK, this );
        }
        else
        {
            m_reader.Read( &this->m_request, this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onFinishRequest()
{
    m_stateHandler->finish();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
QString RiaGrpcClientToServerStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::methodType() const
{
    return "ClientStreamingMethod";
}
