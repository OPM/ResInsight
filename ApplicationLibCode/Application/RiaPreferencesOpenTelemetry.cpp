/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RiaPreferencesOpenTelemetry.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaVersionInfo.h"

#include "cafPdmUiTextEditor.h"

namespace caf
{
template <>
void RiaPreferencesOpenTelemetry::LoggingStateType::setUp()
{
    addItem( RiaPreferencesOpenTelemetry::LoggingState::DISABLED, "DISABLED", "Disabled" );
    addItem( RiaPreferencesOpenTelemetry::LoggingState::DEFAULT, "DEFAULT", "Default" );
    addItem( RiaPreferencesOpenTelemetry::LoggingState::ALL, "ALL", "All" );
    setDefault( RiaPreferencesOpenTelemetry::LoggingState::DEFAULT );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RiaPreferencesOpenTelemetry, "RiaPreferencesOpenTelemetry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOpenTelemetry::RiaPreferencesOpenTelemetry()
{
    CAF_PDM_InitObject( "OpenTelemetry Configuration", "", "", "Configuration for OpenTelemetry crash reporting and telemetry" );

    CAF_PDM_InitField( &m_loggingState, "loggingState", LoggingStateType( LoggingState::DISABLED ), "Logging State" );
    CAF_PDM_InitField( &m_connectionString, "connectionString", QString(), "Azure Connection String" );
    m_connectionString.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_batchTimeoutMs, "batchTimeoutMs", 5000, "Batch Timeout (ms)" );
    CAF_PDM_InitField( &m_maxBatchSize, "maxBatchSize", 512, "Max Batch Size" );
    CAF_PDM_InitField( &m_maxQueueSize, "maxQueueSize", 10000, "Max Queue Size" );
    CAF_PDM_InitField( &m_memoryThresholdMb, "memoryThresholdMb", 50, "Memory Threshold (MB)" );
    CAF_PDM_InitField( &m_samplingRate, "samplingRate", 1.0, "Sampling Rate" );
    CAF_PDM_InitField( &m_connectionTimeoutMs, "connectionTimeoutMs", 10000, "Connection Timeout (ms)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOpenTelemetry::~RiaPreferencesOpenTelemetry()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOpenTelemetry* RiaPreferencesOpenTelemetry::current()
{
    return RiaApplication::instance()->preferences()->openTelemetryPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesOpenTelemetry::setData( const std::map<QString, QString>& keyValuePairs )
{
    for ( const auto& [key, value] : keyValuePairs )
    {
        if ( key == "connection_string" )
        {
            m_connectionString = value;
        }
        else if ( key == "batch_timeout_ms" )
        {
            m_batchTimeoutMs = value.toInt();
        }
        else if ( key == "max_batch_size" )
        {
            m_maxBatchSize = value.toInt();
        }
        else if ( key == "max_queue_size" )
        {
            m_maxQueueSize = value.toInt();
        }
        else if ( key == "memory_threshold_mb" )
        {
            m_memoryThresholdMb = value.toInt();
        }
        else if ( key == "sampling_rate" )
        {
            m_samplingRate = value.toDouble();
        }
        else if ( key == "connection_timeout_ms" )
        {
            m_connectionTimeoutMs = value.toInt();
        }
        else
        {
            RiaLogging::warning( QString( "Unknown OpenTelemetry config key: '%1'" ).arg( key ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesOpenTelemetry::setFieldsReadOnly()
{
    std::vector<caf::PdmFieldHandle*> fields = this->fields();
    for ( auto field : fields )
    {
        // Keep logging state editable
        if ( field != &m_loggingState )
        {
            field->uiCapability()->setUiReadOnly( true );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesOpenTelemetry::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_loggingState );

    // Only show configuration fields if not disabled
    if ( m_loggingState() != LoggingState::DISABLED )
    {
        uiOrdering.add( &m_connectionString );
        uiOrdering.add( &m_batchTimeoutMs );
        uiOrdering.add( &m_maxBatchSize );
        uiOrdering.add( &m_maxQueueSize );
        uiOrdering.add( &m_memoryThresholdMb );
        uiOrdering.add( &m_samplingRate );
        uiOrdering.add( &m_connectionTimeoutMs );
    }
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOpenTelemetry::serviceName() const
{
    return QStringLiteral( "ResInsight" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOpenTelemetry::serviceVersion() const
{
    return QString( STRPRODUCTVER );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOpenTelemetry::connectionString() const
{
    return m_connectionString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferencesOpenTelemetry::batchTimeoutMs() const
{
    return m_batchTimeoutMs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferencesOpenTelemetry::maxBatchSize() const
{
    return m_maxBatchSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferencesOpenTelemetry::maxQueueSize() const
{
    return m_maxQueueSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferencesOpenTelemetry::memoryThresholdMb() const
{
    return m_memoryThresholdMb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaPreferencesOpenTelemetry::samplingRate() const
{
    return m_samplingRate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferencesOpenTelemetry::connectionTimeoutMs() const
{
    return m_connectionTimeoutMs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOpenTelemetry::LoggingState RiaPreferencesOpenTelemetry::loggingState() const
{
    return m_loggingState();
}
