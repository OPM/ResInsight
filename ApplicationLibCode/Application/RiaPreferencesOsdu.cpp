/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RiaApplication.h"

#include "RiaPreferences.h"
#include "RiaPreferencesOsdu.h"

CAF_PDM_SOURCE_INIT( RiaPreferencesOsdu, "RiaPreferencesOsdu" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOsdu::RiaPreferencesOsdu()
{
    CAF_PDM_InitFieldNoDefault( &m_server, "server", "Server" );
    CAF_PDM_InitFieldNoDefault( &m_dataPartitionId, "dataPartitionId", "Data Partition Id" );
    CAF_PDM_InitFieldNoDefault( &m_authority, "authority", "Authority" );
    CAF_PDM_InitFieldNoDefault( &m_scopes, "scopes", "Scopes" );
    CAF_PDM_InitFieldNoDefault( &m_clientId, "clientId", "Client Id" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOsdu* RiaPreferencesOsdu::current()
{
    return RiaApplication::instance()->preferences()->osduPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesOsdu::setData( const std::map<QString, QString>& keyValuePairs )
{
    for ( const auto& [key, value] : keyValuePairs )
    {
        if ( key == "server" )
        {
            m_server = value;
        }
        else if ( key == "dataPartitionId" )
        {
            m_dataPartitionId = value;
        }
        else if ( key == "authority" )
        {
            m_authority = value;
        }
        else if ( key == "scopes" )
        {
            m_scopes = value;
        }
        else if ( key == "clientId" )
        {
            m_clientId = value;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesOsdu::setFieldsReadOnly()
{
    m_server.uiCapability()->setUiReadOnly( true );
    m_dataPartitionId.uiCapability()->setUiReadOnly( true );
    m_authority.uiCapability()->setUiReadOnly( true );
    m_scopes.uiCapability()->setUiReadOnly( true );
    m_clientId.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOsdu::server() const
{
    return m_server;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOsdu::dataPartitionId() const
{
    return m_dataPartitionId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOsdu::authority() const
{
    return m_authority;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOsdu::scopes() const
{
    return m_scopes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOsdu::clientId() const
{
    return m_clientId;
}
