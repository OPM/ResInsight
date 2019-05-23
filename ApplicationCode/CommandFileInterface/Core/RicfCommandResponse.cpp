/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
/////////////////////////////////////////////////////////////////////////////////
#include "RicfCommandResponse.h"



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse::RicfCommandResponse(Status status, const QString& message)    
{
    updateStatus(status, message);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse::Status RicfCommandResponse::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicfCommandResponse::message() const
{
    return m_messages.join("\n");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RicfCommandResponse::result() const
{
    return m_result.p();
}

//--------------------------------------------------------------------------------------------------
/// Takes ownership of the result object
//--------------------------------------------------------------------------------------------------
void RicfCommandResponse::setResult(caf::PdmObject* result)
{
    m_result = result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfCommandResponse::updateStatus(Status status, const QString& message)
{
    m_status = std::max(m_status, status);
    if (!message.isEmpty())
    m_messages.push_back(QString("%1:%2").arg(statusLabel(status)).arg(message));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicfCommandResponse::statusLabel(Status status)
{
    switch (status)
    {
    case COMMAND_WARNING:
        return "WARNING";
    case COMMAND_ERROR:
        return "ERROR";
    default:
        return "";
    }
}
