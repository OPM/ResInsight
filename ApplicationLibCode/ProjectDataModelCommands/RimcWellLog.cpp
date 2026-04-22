/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimcWellLog.h"

#include "KeyValueStore/RiaKeyValueStoreUtil.h"
#include "RiaApplication.h"

#include "RimWellLog.h"
#include "RimcDataContainerString.h"

#include "Well/RigWellLogData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QStringList>

namespace
{
std::vector<float> toFloatVector( const std::vector<double>& values )
{
    std::vector<float> result;
    result.reserve( values.size() );
    for ( double v : values )
    {
        result.push_back( static_cast<float>( v ) );
    }
    return result;
}

void writeFloatArrayToKeyValueStore( const QString& key, const std::vector<double>& values )
{
    auto floatValues = toFloatVector( values );
    RiaApplication::instance()->keyValueStore()->set( key.toStdString(), RiaKeyValueStoreUtil::convertToByteVector( floatValues ) );
}
} // namespace

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLog, RimcWellLog_channelNames, "ChannelNamesInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellLog_channelNames::RimcWellLog_channelNames( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Channel Names", "", "", "Channel names for this well log." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellLog_channelNames::execute()
{
    auto wellLog = self<RimWellLog>();
    if ( !wellLog ) return std::unexpected( "No well log found" );

    RigWellLogData* data = wellLog->wellLogData();
    if ( !data ) return std::unexpected( "Well log has no data" );

    auto container = new RimcDataContainerString();

    std::vector<QString> names;
    for ( const QString& name : data->wellLogChannelNames() )
    {
        names.push_back( name );
    }
    container->m_stringValues = names;

    return container;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellLog_channelNames::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLog, RimcWellLog_readWellLogDataInternal, "ReadWellLogDataInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellLog_readWellLogDataInternal::RimcWellLog_readWellLogDataInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Read Well Log Data", "", "", "Read Well Log Data" );

    CAF_PDM_InitScriptableField( &m_measuredDepthKey, "MeasuredDepthKey", QString(), "Measured Depth Key" );
    CAF_PDM_InitScriptableField( &m_tvdMslKey, "TvdMslKey", QString(), "TVD MSL Key" );
    CAF_PDM_InitScriptableField( &m_tvdRkbKey, "TvdRkbKey", QString(), "TVD RKB Key" );
    CAF_PDM_InitScriptableField( &m_channelKeysCsv, "ChannelKeysCsv", QString(), "Channel Keys CSV" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellLog_readWellLogDataInternal::execute()
{
    auto wellLog = self<RimWellLog>();
    if ( !wellLog ) return std::unexpected( "No well log found" );

    RigWellLogData* data = wellLog->wellLogData();
    if ( !data ) return std::unexpected( "Well log has no data" );

    if ( !m_measuredDepthKey().isEmpty() )
    {
        writeFloatArrayToKeyValueStore( m_measuredDepthKey(), data->depthValues() );
    }

    if ( !m_tvdMslKey().isEmpty() && data->hasTvdMslChannel() )
    {
        writeFloatArrayToKeyValueStore( m_tvdMslKey(), data->tvdMslValues() );
    }

    if ( !m_tvdRkbKey().isEmpty() && data->hasTvdRkbChannel() )
    {
        writeFloatArrayToKeyValueStore( m_tvdRkbKey(), data->tvdRkbValues() );
    }

    if ( !m_channelKeysCsv().isEmpty() )
    {
        QStringList availableChannels = data->wellLogChannelNames();
        QStringList channelMappings   = m_channelKeysCsv().split( ",", Qt::SkipEmptyParts );
        for ( const QString& mapping : channelMappings )
        {
            QStringList parts = mapping.split( ":", Qt::SkipEmptyParts );
            if ( parts.size() != 2 )
            {
                return std::unexpected( QString( "Invalid channel mapping format: %1" ).arg( mapping ) );
            }

            QString channelName = parts[0];
            QString channelKey  = parts[1];

            if ( channelName.isEmpty() || channelKey.isEmpty() )
            {
                return std::unexpected( QString( "Empty channel name or key in mapping: %1" ).arg( mapping ) );
            }

            if ( !availableChannels.contains( channelName ) )
            {
                return std::unexpected( QString( "Channel '%1' not found in well log" ).arg( channelName ) );
            }

            writeFloatArrayToKeyValueStore( channelKey, data->values( channelName ) );
        }
    }

    return nullptr;
}
