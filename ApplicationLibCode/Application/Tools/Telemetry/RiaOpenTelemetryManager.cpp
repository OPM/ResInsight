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

#include "RiaOpenTelemetryManager.h"

#include "RiaLogging.h"
#include "RiaPreferencesOpenTelemetry.h"
#include "RifJsonEncodeDecode.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QSysInfo>
#include <QTimer>

#include <algorithm>
#include <random>
#include <sstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaOpenTelemetryManager::HealthSnapshot::getSuccessRate() const
{
    uint64_t total = eventsSent + eventsDropped;
    if ( total == 0 ) return 1.0;
    return static_cast<double>( eventsSent ) / total;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::HealthSnapshot::isHealthy() const
{
    // Consider healthy if success rate > 90% and we've had recent successful sends
    const auto now                  = std::chrono::steady_clock::now();
    const auto timeSinceLastSuccess = now - lastSuccessfulSend;

    return getSuccessRate() > 0.9 && timeSinceLastSuccess < std::chrono::minutes( 5 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOpenTelemetryManager& RiaOpenTelemetryManager::instance()
{
    static RiaOpenTelemetryManager instance;
    return instance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOpenTelemetryManager::RiaOpenTelemetryManager()
    : QObject( nullptr )
{
    m_healthMetrics.systemStartTime    = std::chrono::steady_clock::now();
    m_healthMetrics.lastSuccessfulSend = m_healthMetrics.systemStartTime;

    // Initialize Qt networking components
    m_networkAccessManager = new QNetworkAccessManager( this );

    // Initialize timers
    m_processTimer = new QTimer( this );
    connect( m_processTimer, &QTimer::timeout, this, &RiaOpenTelemetryManager::onProcessEventTimer );

    m_healthTimer = new QTimer( this );
    connect( m_healthTimer, &QTimer::timeout, this, &RiaOpenTelemetryManager::sendHealthSpan );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOpenTelemetryManager::~RiaOpenTelemetryManager()
{
    shutdown();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::initialize()
{
    std::lock_guard<std::mutex> lock( m_configMutex );

    // Check if OpenTelemetry is disabled in preferences
    auto* prefs = RiaPreferencesOpenTelemetry::current();
    if ( prefs && prefs->loggingState() == RiaPreferencesOpenTelemetry::LoggingState::DISABLED )
    {
        RiaLogging::info( "OpenTelemetry is disabled in preferences" );
        return false;
    }

    if ( m_initialized.load() )
    {
        return true;
    }

    if ( !initializeProvider() )
    {
        return false;
    }

    // Start event processing timer (100ms interval for responsive processing)
    m_isShuttingDown = false;
    m_processTimer->start( 100 );

    // Start health monitoring timer (5 minutes interval)
    if ( m_healthMonitoringEnabled )
    {
        m_healthTimer->start( 5 * 60 * 1000 ); // 5 minutes in milliseconds
        sendHealthSpan();
    }

    m_initialized = true;
    m_enabled     = true;

    RiaLogging::info( "OpenTelemetry initialized successfully" );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::shutdown( std::chrono::seconds timeout )
{
    if ( !m_initialized.load() )
    {
        return;
    }

    m_isShuttingDown = true;
    m_enabled        = false;

    // Stop timers
    if ( m_processTimer )
    {
        m_processTimer->stop();
    }
    if ( m_healthTimer )
    {
        m_healthTimer->stop();
    }

    // Flush pending events
    flushPendingEvents();

    m_initialized = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::reportEventAsync( const std::string& eventName, const std::map<std::string, std::string>& attributes )
{
    if ( !isEnabled() || isCircuitBreakerOpen() || !shouldSampleEvent() )
    {
        return;
    }

    std::unique_lock<std::mutex> lock( m_queueMutex );

    // Check queue size and apply backpressure
    if ( m_backpressureEnabled && m_eventQueue.size() >= m_maxQueueSize )
    {
        m_healthMetrics.eventsDropped++;
        return;
    }

    m_eventQueue.emplace( eventName, attributes );
    m_healthMetrics.eventsQueued++;
}

//--------------------------------------------------------------------------------------------------
/// Helper function to extract the relevant part of the file path starting from "ResInsight"
//--------------------------------------------------------------------------------------------------
static std::string extractRelevantPath( const std::string& fullPath )
{
    size_t pos = fullPath.find( "ResInsight" );
    if ( pos != std::string::npos )
    {
        return fullPath.substr( pos );
    }
    return fullPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::reportCrash( int signalCode, const std::stacktrace& trace )
{
    if ( !isEnabled() )
    {
        return;
    }

    // Format stack trace using existing ResInsight formatter
    std::stringstream ss;
    int               frame = 0;
    for ( const auto& entry : trace )
    {
        std::string relevantPath = extractRelevantPath( entry.source_file() );
        ss << "  [" << frame++ << "] " << entry.description() << " at " << relevantPath << ":" << entry.source_line() << "\n";
    }

    std::string rawStackTrace = ss.str();

    std::map<std::string, std::string> attributes;
    attributes["crash.signal"]      = std::to_string( signalCode );
    attributes["crash.thread_id"]   = std::to_string( std::hash<std::thread::id>{}( std::this_thread::get_id() ) );
    attributes["crash.stack_trace"] = rawStackTrace;
    attributes["service.name"]      = RiaPreferencesOpenTelemetry::current()->serviceName().toStdString();
    attributes["service.version"]   = RiaPreferencesOpenTelemetry::current()->serviceVersion().toStdString();

    // Report with high priority (bypass sampling)
    std::unique_lock<std::mutex> lock( m_queueMutex );
    m_eventQueue.emplace( "crash.signal_handler", attributes );
    m_healthMetrics.eventsQueued++;
    lock.unlock();

    RiaLogging::error( QString( "Crash reported to OpenTelemetry (signal: %1)" ).arg( signalCode ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::reportTestCrash( const std::stacktrace& trace )
{
    if ( !isEnabled() )
    {
        return;
    }

    // Format stack trace
    std::stringstream ss;
    int               frame = 0;
    for ( const auto& entry : trace )
    {
        std::string relevantPath = extractRelevantPath( entry.source_file() );
        ss << "  [" << frame++ << "] " << entry.description() << " at " << relevantPath << ":" << entry.source_line() << "\n";
    }

    std::string rawStackTrace = ss.str();

    std::map<std::string, std::string> attributes;
    attributes["test.type"]        = "manual_stack_trace";
    attributes["test.thread_id"]   = std::to_string( std::hash<std::thread::id>{}( std::this_thread::get_id() ) );
    attributes["test.stack_trace"] = rawStackTrace;
    attributes["service.name"]     = RiaPreferencesOpenTelemetry::current()->serviceName().toStdString();
    attributes["service.version"]  = RiaPreferencesOpenTelemetry::current()->serviceVersion().toStdString();

    reportEventAsync( "test.stack_trace", attributes );

    RiaLogging::info( "Test stack trace reported to OpenTelemetry" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::isEnabled() const
{
    return m_enabled.load() && m_initialized.load();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::isInitialized() const
{
    return m_initialized.load();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::setErrorCallback( ErrorCallback callback )
{
    std::lock_guard<std::mutex> lock( m_configMutex );
    m_errorCallback = callback;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::setMaxQueueSize( size_t maxEvents )
{
    std::lock_guard<std::mutex> lock( m_configMutex );
    m_maxQueueSize = maxEvents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::enableBackpressure( bool enable )
{
    std::lock_guard<std::mutex> lock( m_configMutex );
    m_backpressureEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::setMemoryThreshold( size_t maxMemoryMB )
{
    std::lock_guard<std::mutex> lock( m_configMutex );
    m_memoryThresholdMB = maxMemoryMB;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::setSamplingRate( double rate )
{
    std::lock_guard<std::mutex> lock( m_configMutex );
    m_samplingRate = std::clamp( rate, 0.0, 1.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaOpenTelemetryManager::getCurrentQueueSize() const
{
    std::lock_guard<std::mutex> lock( m_queueMutex );
    return m_eventQueue.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOpenTelemetryManager::HealthSnapshot RiaOpenTelemetryManager::getHealthMetrics() const
{
    HealthSnapshot result;
    result.eventsQueued       = m_healthMetrics.eventsQueued.load();
    result.eventsSent         = m_healthMetrics.eventsSent.load();
    result.eventsDropped      = m_healthMetrics.eventsDropped.load();
    result.networkFailures    = m_healthMetrics.networkFailures.load();
    result.lastSuccessfulSend = m_healthMetrics.lastSuccessfulSend;
    result.systemStartTime    = m_healthMetrics.systemStartTime;
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::isHealthy() const
{
    return getHealthMetrics().isHealthy() && !isCircuitBreakerOpen();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::enableHealthMonitoring( bool enable )
{
    std::lock_guard<std::mutex> lock( m_configMutex );
    m_healthMonitoringEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::initializeProvider()
{
    try
    {
        if ( !createExporter() )
        {
            return false;
        }

        setupResourceAttributes();
        return true;
    }
    catch ( const std::exception& e )
    {
        handleError( TelemetryError::InternalError, QString( "Failed to initialize provider: %1" ).arg( e.what() ) );
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// Parse Azure Application Insights connection string
/// Format: InstrumentationKey=<key>;IngestionEndpoint=<endpoint>;...
//--------------------------------------------------------------------------------------------------
static std::map<QString, QString> parseAzureConnectionString( const QString& connectionString )
{
    std::map<QString, QString> result;
    QStringList                parts = connectionString.split( ';', Qt::SkipEmptyParts );

    for ( const QString& part : parts )
    {
        int equalPos = part.indexOf( '=' );
        if ( equalPos > 0 )
        {
            QString key   = part.left( equalPos ).trimmed();
            QString value = part.mid( equalPos + 1 ).trimmed();
            result[key]   = value;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::createExporter()
{
    try
    {
        auto* prefs = RiaPreferencesOpenTelemetry::current();
        if ( !prefs )
        {
            handleError( TelemetryError::ConfigurationError, "Failed to get OpenTelemetry Preferences" );
            return false;
        }

        // Parse and validate connection string
        auto connectionParams = parseAzureConnectionString( prefs->connectionString() );
        if ( !connectionParams.contains( "InstrumentationKey" ) )
        {
            handleError( TelemetryError::ConfigurationError, "InstrumentationKey not found in connection string" );
            return false;
        }

        if ( !connectionParams.contains( "IngestionEndpoint" ) )
        {
            handleError( TelemetryError::ConfigurationError, "IngestionEndpoint not found in connection string" );
            return false;
        }

        // Using Application Insights REST API for telemetry
        RiaLogging::info( QString( "Application Insights REST API configured for production environment" ) );

        return true;
    }
    catch ( const std::exception& e )
    {
        handleError( TelemetryError::NetworkError, QString( "Failed to create exporter: %1" ).arg( e.what() ) );
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::setupResourceAttributes()
{
    // Resource attributes are typically set during provider creation
    // This would be expanded with system information, process details, etc.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::onProcessEventTimer()
{
    if ( !m_isShuttingDown.load() )
    {
        processEvents();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::processEvents()
{
    std::unique_lock<std::mutex> lock( m_queueMutex );

    if ( m_eventQueue.empty() )
    {
        return;
    }

    // Process a batch of events
    std::queue<Event> batch;
    auto*             prefs        = RiaPreferencesOpenTelemetry::current();
    int               maxBatchSize = prefs ? prefs->maxBatchSize() : 100;

    for ( int i = 0; i < maxBatchSize && !m_eventQueue.empty(); ++i )
    {
        batch.push( m_eventQueue.front() );
        m_eventQueue.pop();
    }

    lock.unlock();

    // Process events outside of lock
    while ( !batch.empty() )
    {
        processEvent( batch.front() );
        batch.pop();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::processEvent( const Event& event )
{
    try
    {
        auto* prefs = RiaPreferencesOpenTelemetry::current();
        if ( !prefs )
        {
            handleError( TelemetryError::InternalError, QString( "Failed to access Open Telemetry Preferences." ) );
            updateHealthMetrics( false );
            return;
        }

        // Use Application Insights REST API
        auto connectionParams = parseAzureConnectionString( prefs->connectionString() );

        if ( !connectionParams.contains( "InstrumentationKey" ) || !connectionParams.contains( "IngestionEndpoint" ) )
        {
            updateHealthMetrics( false );
            return;
        }

        // Format timestamp - must match Application Insights format exactly
        const auto        timestampUtc = std::chrono::floor<std::chrono::milliseconds>( event.timestamp );
        const std::string timestamp    = std::format( "{:%Y-%m-%dT%H:%M:%SZ}", timestampUtc );

        // Convert attributes to JSON properties
        QMap<QString, QVariant> properties;
        for ( const auto& [key, value] : event.attributes )
        {
            properties[QString::fromStdString( key )] = QString::fromStdString( value );
        }

        // Add system information
        properties["os.type"]    = QSysInfo::productType();
        properties["os.version"] = QSysInfo::productVersion();
        properties["os.name"]    = QSysInfo::prettyProductName();

        // Create Application Insights telemetry item
        QMap<QString, QVariant> baseData;
        baseData["name"]       = QString::fromStdString( event.name );
        baseData["properties"] = properties;

        QMap<QString, QVariant> data;
        data["baseType"] = "EventData";
        data["baseData"] = baseData;

        QMap<QString, QVariant> telemetryItem;
        telemetryItem["time"] = QString::fromStdString( timestamp );
        telemetryItem["iKey"] = connectionParams["InstrumentationKey"];
        telemetryItem["name"] = "Microsoft.ApplicationInsights.Event";
        telemetryItem["data"] = data;

        // Convert to JSON string
        QString jsonPayload = ResInsightInternalJson::Json::encode( telemetryItem, false );

        // Send to Application Insights using QNetworkAccessManager
        QString url = connectionParams["IngestionEndpoint"] + "/v2/track";

        QNetworkRequest request;
        request.setUrl( QUrl( url ) );
        request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
        request.setHeader( QNetworkRequest::KnownHeaders( QNetworkRequest::UserAgentHeader ), "ResInsight-OpenTelemetry" );
        request.setTransferTimeout( prefs->connectionTimeoutMs() );

        QNetworkReply* reply = m_networkAccessManager->post( request, jsonPayload.toUtf8() );

        // Handle response asynchronously
        connect( reply,
                 &QNetworkReply::finished,
                 this,
                 [this, reply]()
                 {
                     if ( reply->error() == QNetworkReply::NoError )
                     {
                         updateHealthMetrics( true );
                         resetCircuitBreaker();
                     }
                     else
                     {
                         QString errorMsg = QString( "HTTP %1: %2" )
                                                .arg( reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() )
                                                .arg( reply->errorString() );
                         handleError( TelemetryError::NetworkError, QString( "Failed to send telemetry: %1" ).arg( errorMsg ) );
                         updateHealthMetrics( false );
                     }
                     reply->deleteLater();
                 } );
    }
    catch ( const std::exception& e )
    {
        handleError( TelemetryError::InternalError, QString( "Failed to process event: %1" ).arg( e.what() ) );
        updateHealthMetrics( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::shouldSampleEvent() const
{
    if ( m_samplingRate >= 1.0 )
    {
        return true;
    }

    static thread_local std::mt19937                           gen( std::random_device{}() );
    static thread_local std::uniform_real_distribution<double> dis( 0.0, 1.0 );

    return dis( gen ) < m_samplingRate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::flushPendingEvents()
{
    // Process remaining events in the queue
    processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::handleError( TelemetryError error, const QString& context )
{
    m_consecutiveFailures++;

    if ( m_consecutiveFailures >= 3 )
    {
        m_circuitBreakerOpen = true;
        RiaLogging::warning( "OpenTelemetry circuit breaker opened due to consecutive failures" );
    }

    if ( m_errorCallback )
    {
        m_errorCallback( error, context );
    }

    RiaLogging::warning( QString( "OpenTelemetry error: %1" ).arg( context ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::attemptReconnection()
{
    auto now = std::chrono::steady_clock::now();
    if ( now - m_lastReconnectAttempt < std::chrono::minutes( 5 ) )
    {
        return; // Don't retry too frequently
    }

    m_lastReconnectAttempt = now;

    // Try to reinitialize connection
    if ( createExporter() )
    {
        resetCircuitBreaker();
        RiaLogging::info( "OpenTelemetry reconnection successful" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::isCircuitBreakerOpen() const
{
    return m_circuitBreakerOpen.load();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::resetCircuitBreaker()
{
    m_consecutiveFailures = 0;
    m_circuitBreakerOpen  = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::updateHealthMetrics( bool success )
{
    if ( success )
    {
        m_healthMetrics.eventsSent++;
        m_healthMetrics.lastSuccessfulSend = std::chrono::steady_clock::now();
    }
    else
    {
        m_healthMetrics.networkFailures++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::sendHealthSpan()
{
    if ( !isEnabled() )
    {
        return;
    }

    auto                               metrics = getHealthMetrics();
    std::map<std::string, std::string> attributes;
    attributes["health.events_queued"]    = std::to_string( metrics.eventsQueued );
    attributes["health.events_sent"]      = std::to_string( metrics.eventsSent );
    attributes["health.events_dropped"]   = std::to_string( metrics.eventsDropped );
    attributes["health.network_failures"] = std::to_string( metrics.networkFailures );
    attributes["health.success_rate"]     = std::to_string( metrics.getSuccessRate() );
    attributes["health.queue_size"]       = std::to_string( getCurrentQueueSize() );

    reportEventAsync( "health.status", attributes );
}