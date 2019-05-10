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

inline RiaGrpcServerCallMethod::RiaGrpcServerCallMethod()
    : m_state(CREATE_HANDLER)
    , m_status(Status::OK)
{
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServerCallMethod::CallState RiaGrpcServerCallMethod::callState() const
{
    return m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Status& RiaGrpcServerCallMethod::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
inline void RiaGrpcServerCallMethod::setCallState(CallState state)
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcServerAbstractCallData<ServiceT, RequestT, ReplyT>::RiaGrpcServerAbstractCallData(ServiceT* service)
    : m_service(service)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
const char* RiaGrpcServerAbstractCallData<ServiceT, RequestT, ReplyT>::name() const
{
    return typeid(ServiceT).name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
const RequestT& RiaGrpcServerAbstractCallData<ServiceT, RequestT, ReplyT>::request() const
{
    return m_request;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
ReplyT& RiaGrpcServerAbstractCallData<ServiceT, RequestT, ReplyT>::reply()
{
    return m_reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::RiaGrpcServerCallData(ServiceT*     service,
                                                                         MethodImpl    methodImpl,
                                                                         MethodRequest methodRequest)
    : RiaGrpcServerAbstractCallData(service)
    , m_responder(&m_context)
    , m_methodImpl(methodImpl)
    , m_methodRequest(methodRequest)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcServerCallMethod* RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::createNewFromThis() const
{
    return new RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>(m_service, m_methodImpl, m_methodRequest);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::createRequestHandler(ServerCompletionQueue* completionQueue)
{
    m_methodRequest(*m_service, &m_context, &m_request, &m_responder, completionQueue, completionQueue, this);    
    setCallState(RiaGrpcServerCallMethod::INIT_REQUEST);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::initRequest()
{
    setCallState(RiaGrpcServerCallMethod::PROCESS_REQUEST);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::processRequest()
{
    m_status = m_methodImpl(*m_service, &m_context, &m_request, &m_reply);
    m_responder.Finish(m_reply, m_status, this);
    setCallState(RiaGrpcServerCallMethod::FINISH_REQUEST);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT, StateHandlerT>::RiaGrpcServerStreamingCallData(ServiceT*      service,
                                                                                                          MethodImpl     methodImpl,
                                                                                                          MethodRequest   methodRequest,
                                                                                                          StateHandlerT*  stateHandler)
    : RiaGrpcServerAbstractCallData(service)
    , m_responder(&m_context)
    , m_methodImpl(methodImpl)
    , m_methodRequest(methodRequest)
    , m_dataCount(0u)
    , m_stateHandler(stateHandler)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
RiaGrpcServerCallMethod* RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT, StateHandlerT>::createNewFromThis() const
{
    return new RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT, StateHandlerT>(m_service, m_methodImpl, m_methodRequest, new StateHandlerT);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT, StateHandlerT>::createRequestHandler(ServerCompletionQueue* completionQueue)
{
    m_methodRequest(*m_service, &m_context, &m_request, &m_responder, completionQueue, completionQueue, this);
    setCallState(RiaGrpcServerCallMethod::INIT_REQUEST);
}

//--------------------------------------------------------------------------------------------------
/// Perform initialisation tasks at the time of receiving a request
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT, StateHandlerT>::initRequest()
{
    m_status = m_stateHandler->init(&m_request);
    setCallState(RiaGrpcServerCallMethod::PROCESS_REQUEST);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT, typename StateHandlerT>
void RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT, StateHandlerT>::processRequest()
{
    m_reply = ReplyT(); // Make sure it is reset

    if (!m_status.ok())
    {
        m_responder.Finish(m_status, this);
        setCallState(RiaGrpcServerCallMethod::FINISH_REQUEST);
        return;
    }
    
    m_status = m_methodImpl(*m_service, &m_context, &m_request, &m_reply, m_stateHandler.get());
    if (m_status.ok())
    {
        m_responder.Write(m_reply, this);
    }
    else
    {
        setCallState(FINISH_REQUEST);
        // Out of range means we're finished but it isn't an error
        if (m_status.error_code() == grpc::OUT_OF_RANGE)
        {
            RiaLogging::info(QString("Finished sending data packages"));
            m_status = Status::OK;
        }
        RiaLogging::info(QString("Send finished response"));
        m_responder.Finish(m_status, this);
    }
}
