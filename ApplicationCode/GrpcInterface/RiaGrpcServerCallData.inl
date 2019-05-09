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
void RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::createRequest(ServerCompletionQueue* completionQueue)
{
    m_methodRequest(*m_service, &m_context, &m_request, &m_responder, completionQueue, completionQueue, this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
Status RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::processRequest()
{
    Status status = m_methodImpl(*m_service, &m_context, &m_request, &m_reply);
    m_responder.Finish(m_reply, status, this);
    return status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
ReplyT& RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::reply()
{
    return m_reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT>::RiaGrpcServerStreamingCallData(ServiceT*     service,
                                                                                           MethodImpl    methodImpl,
                                                                                           MethodRequest methodRequest)
    : RiaGrpcServerCallData(service)
    , m_responder(&m_context)
    , m_methodImpl(methodImpl)
    , m_methodRequest(methodRequest)
    , m_serverWriter(&m_context)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcServerCallMethod* RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT>::createNewFromThis() const
{
    return new RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT>(m_service, m_methodImpl, m_methodRequest);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT>::createRequest(ServerCompletionQueue* completionQueue)
{
    m_methodRequest(*m_service, &m_context, &m_request, &m_responder, completionQueue, completionQueue, this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
Status RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT>::processRequest()
{
    return m_methodImpl(*m_service, &m_context, &m_request, &m_reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcServerStreamingCallData<ServiceT, RequestT, ReplyT>::finishRequest()
{
    m_responder.Finish(status, this);
}
