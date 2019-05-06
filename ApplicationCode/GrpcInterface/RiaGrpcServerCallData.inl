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
RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::RiaGrpcServerCallData(ServiceT*              service,
                                                                         ServerCompletionQueue* cq,
                                                                         const std::string&     methodName,
                                                                         MethodImpl             methodImpl,
                                                                         RequestImpl            methodRequest)
    : RiaGrpcServerCallMethod(methodName)
    , m_service(service)
    , m_completionQueue(cq)
    , m_responder(&m_context)
    , m_methodImpl(methodImpl)
    , m_methodRequest(methodRequest)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
ServerContext& RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::context()
{
    return m_context;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RequestT& RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::request()
{
    return m_request;
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
ServerAsyncResponseWriter<ReplyT>& RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::responder()
{
    return m_responder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
RiaGrpcServerCallMethod* RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::clone() const
{
    return new RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>(
        m_service, m_completionQueue, methodName(), m_methodImpl, m_methodRequest);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
void RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::createRequest()
{
    m_methodRequest(*m_service, &m_context, &m_request, &m_responder, m_completionQueue, m_completionQueue, this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename ServiceT, typename RequestT, typename ReplyT>
Status RiaGrpcServerCallData<ServiceT, RequestT, ReplyT>::processRequest()
{
    Status status = m_methodImpl(*m_service, &m_context, &m_request, &m_reply);
    responder().Finish(m_reply, status, this);
    return status;
}
