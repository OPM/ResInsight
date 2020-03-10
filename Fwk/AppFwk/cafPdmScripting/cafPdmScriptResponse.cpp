//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#include "cafPdmScriptResponse.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmScriptResponse::PdmScriptResponse( Status status, const QString& message )
    : m_status( COMMAND_OK )
{
    updateStatus( status, message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmScriptResponse::PdmScriptResponse( PdmObject* ok_result )
    : m_status( COMMAND_OK )
    , m_result( ok_result )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmScriptResponse::Status PdmScriptResponse::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
/// The resulting message is sent in HTTP metadata and must not have any newlines.
//--------------------------------------------------------------------------------------------------
QString PdmScriptResponse::sanitizedResponseMessage() const
{
    QString completeMessage = m_messages.join( ";;" );
    completeMessage.replace( '\n', ";;" );
    return completeMessage;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList PdmScriptResponse::messages() const
{
    return m_messages;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObject* PdmScriptResponse::result() const
{
    return m_result.get();
}

//--------------------------------------------------------------------------------------------------
/// Takes ownership of the result object
//--------------------------------------------------------------------------------------------------
void PdmScriptResponse::setResult( PdmObject* result )
{
    m_result.reset( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptResponse::updateStatus( Status status, const QString& message )
{
    m_status = std::max( m_status, status );
    if ( !message.isEmpty() )
    {
        m_messages.push_back( QString( "%1: %2" ).arg( statusLabel( status ) ).arg( message ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmScriptResponse::statusLabel( Status status )
{
    switch ( status )
    {
        case COMMAND_WARNING:
            return "Warning";
        case COMMAND_ERROR:
            return "Error";
        default:
            return "";
    }
}
