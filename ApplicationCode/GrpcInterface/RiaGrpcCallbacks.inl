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

inline RiaAbstractGrpcCallback::RiaAbstractGrpcCallback()
    : m_state(CREATE_HANDLER)
    , m_status(Status::OK)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaAbstractGrpcCallback::CallState RiaAbstractGrpcCallback::callState() const
{
    return m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Status& RiaAbstractGrpcCallback::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
inline void RiaAbstractGrpcCallback::setCallState(CallState state)
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>::RiaGrpcRequestCallback(ServiceT* service)
    : m_service(service)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
QString RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>::name() const
{
    QString fullName = QString("%1:%2(%3, %4)")
                           .arg(typeid(ServiceT).name())
                           .arg(methodType())
                           .arg(typeid(RequestT).name())
                           .arg(typeid(ReplyT).name());
    return fullName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
const RequestT& RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>::request() const
{
    return m_request;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
ReplyT& RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>::reply()
{
    return m_reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcCallback<ServiceT, RequestT, ReplyT>::RiaGrpcCallback(ServiceT*      service,
                                                             MethodImplT    methodImpl,
                                                             MethodRequestT methodRequest)
    : RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>(service)
    , m_responder(&m_context)
    , m_methodImpl(methodImpl)
    , m_methodRequest(methodRequest)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaAbstractGrpcCallback* RiaGrpcCallback<ServiceT, RequestT, ReplyT>::createNewFromThis() const
{
    return new RiaGrpcCallback<ServiceT, RequestT, ReplyT>(this->m_service, this->m_methodImpl, this->m_methodRequest);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcCallback<ServiceT, RequestT, ReplyT>::createRequestHandler(ServerCompletionQueue* completionQueue)
{
    // The Request-method is where the request gets filled in with data from the gRPC stack:
    m_methodRequest(*this->m_service, &m_context, &this->m_request, &m_responder, completionQueue, completionQueue, this);
    this->setCallState(RiaAbstractGrpcCallback::INIT_REQUEST_COMPLETED);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcCallback<ServiceT, RequestT, ReplyT>::onInitRequestCompleted()
{
    this->setCallState(RiaAbstractGrpcCallback::PROCESS_REQUEST);
    this->onProcessRequest();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcCallback<ServiceT, RequestT, ReplyT>::onProcessRequest()
{
    this->m_status = m_methodImpl(*this->m_service, &m_context, &this->m_request, &this->m_reply);
    m_responder.Finish(this->m_reply, this->m_status, this);
    this->setCallState(RiaAbstractGrpcCallback::FINISH_REQUEST);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
QString RiaGrpcCallback<ServiceT, RequestT, ReplyT>::methodType() const
{
    return "RegularMethod";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::RiaGrpcStreamCallback(ServiceT*      service,
                                                                                        MethodImplT    methodImpl,
                                                                                        MethodRequestT methodRequest,
                                                                                        StateHandlerT* stateHandler)
    : RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>(service)
    , m_responder(&m_context)
    , m_methodImpl(methodImpl)
    , m_methodRequest(methodRequest)
    , m_stateHandler(stateHandler)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaAbstractGrpcCallback* RiaGrpcStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createNewFromThis() const
{
    return new RiaGrpcStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>(
        this->m_service, m_methodImpl, m_methodRequest, new StateHandlerT);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createRequestHandler(
    ServerCompletionQueue* completionQueue)
{
    m_methodRequest(*this->m_service, &m_context, &this->m_request, &m_responder, completionQueue, completionQueue, this);
    this->setCallState(RiaAbstractGrpcCallback::INIT_REQUEST_COMPLETED);
}

//--------------------------------------------------------------------------------------------------
/// Perform initialisation tasks at the time of receiving a request
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onInitRequestCompleted()
{
    this->m_status = m_stateHandler->init(&this->m_request);
    this->setCallState(RiaAbstractGrpcCallback::PROCESS_REQUEST);
    this->onProcessRequest();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onProcessRequest()
{
    this->m_reply = ReplyT(); // Make sure it is reset

    if (!this->m_status.ok())
    {
        m_responder.Finish(this->m_status, this);
        this->setCallState(RiaAbstractGrpcCallback::FINISH_REQUEST);
        return;
    }

    this->m_status = m_methodImpl(*this->m_service, &m_context, &this->m_request, &this->m_reply, m_stateHandler.get());
    if (this->m_status.ok())
    {
        m_responder.Write(this->m_reply, this);
    }
    else
    {
        this->setCallState(RiaAbstractGrpcCallback::FINISH_REQUEST);
        // Out of range means we're finished but it isn't an error
        if (this->m_status.error_code() == grpc::OUT_OF_RANGE)
        {
            this->m_status = Status::OK;
        }
        m_responder.Finish(this->m_status, this);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
QString RiaGrpcStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::methodType() const
{
    return "StreamingMethod";
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::RiaGrpcClientStreamCallback(ServiceT*      service,
                                                                                                    MethodImplT    methodImpl,
                                                                                                    MethodRequestT methodRequest,
                                                                                                    StateHandlerT* stateHandler)
    : RiaGrpcRequestCallback<ServiceT, RequestT, ReplyT>(service)
    , m_reader(&m_context)
    , m_methodImpl(methodImpl)
    , m_methodRequest(methodRequest)
    , m_stateHandler(stateHandler)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaAbstractGrpcCallback* RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createNewFromThis() const
{
    return new RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>(
        this->m_service, m_methodImpl, m_methodRequest, new StateHandlerT);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::createRequestHandler(
    ServerCompletionQueue* completionQueue)
{
    m_methodRequest(*this->m_service, &m_context, &this->m_reader, completionQueue, completionQueue, this);
    this->setCallState(RiaAbstractGrpcCallback::INIT_REQUEST_STARTED);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onInitRequestStarted()
{
    this->setCallState(RiaAbstractGrpcCallback::INIT_REQUEST_COMPLETED);
    m_reader.Read(&m_request, this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onInitRequestCompleted()
{
    this->setCallState(RiaAbstractGrpcCallback::PROCESS_REQUEST); 
    this->m_status = m_stateHandler->init(&this->m_request); // Fully received the stream package so can now init
    m_reader.Read(&m_request, this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onProcessRequest()
{
    this->m_reply = ReplyT(); // Make sure it is reset

    if (!this->m_status.ok())
    {
        m_reader.Finish(this->m_reply, this->m_status, this);
        this->setCallState(RiaAbstractGrpcCallback::FINISH_REQUEST);
        return;
    }
    
    this->m_status = m_methodImpl(*this->m_service, &m_context, &this->m_request, &this->m_reply, m_stateHandler.get());
    if (!this->m_status.ok())
    {
        this->setCallState(RiaAbstractGrpcCallback::FINISH_REQUEST);
        if (this->m_status.error_code() == grpc::OUT_OF_RANGE)
        {
            m_reader.Finish(this->m_reply, grpc::Status::OK, this);
        }
        else
        {
            m_reader.FinishWithError(this->m_status, this);
        }
    }
    else
    {
        m_reader.Read(&this->m_request, this);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::onFinishRequest()
{
    m_stateHandler->finish();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
QString RiaGrpcClientStreamCallback<ServiceT, RequestT, ReplyT, StateHandlerT>::methodType() const
{
    return "ClientStreamingMethod";
}
