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

#include <QCryptographicHash>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QString>
#include <QSysInfo>
#include <QTimer>
#include <QVariantList>

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
/// Get system username in a cross-platform way
/// Returns username from environment variables (USERNAME on Windows, USER on Linux)
//--------------------------------------------------------------------------------------------------
QString RiaOpenTelemetryManager::getSystemUsername()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Try Windows environment variable first
    QString username = env.value( "USERNAME" );
    if ( !username.isEmpty() )
    {
        return username;
    }

    // Try Linux/Unix environment variable. Can be empty on some systems
    return env.value( "USER" );
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

    // Skip initialization if no config file was found (connection string is empty)
    auto* prefs = RiaPreferencesOpenTelemetry::current();
    if ( !prefs || prefs->connectionString().isEmpty() )
    {
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
void RiaOpenTelemetryManager::shutdown()
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

    // Disconnect network manager to prevent callbacks during shutdown
    if ( m_networkAccessManager )
    {
        m_networkAccessManager->disconnect();
    }

    // Flush pending events
    flushPendingEvents();

    m_initialized = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::reinitialize()
{
    shutdown();
    return initialize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaOpenTelemetryManager::isEventAllowed( const std::string& eventName ) const
{
    // Crash events are always reported regardless of filter settings
    if ( eventName.starts_with( "crash." ) ) return true;

    auto* prefs = RiaPreferencesOpenTelemetry::current();
    if ( !prefs ) return true;

    const QString name = QString::fromStdString( eventName );

    auto matches = []( const QString& name, const QStringList& patterns ) -> bool
    {
        for ( const QString& pattern : patterns )
        {
            QRegularExpression re( QRegularExpression::wildcardToRegularExpression( pattern.trimmed() ) );
            if ( re.match( name ).hasMatch() ) return true;
        }
        return false;
    };

    const QStringList allowlist = prefs->eventAllowlist();
    if ( !allowlist.isEmpty() && !matches( name, allowlist ) ) return false;

    const QStringList denylist = prefs->eventDenylist();
    return denylist.isEmpty() || !matches( name, denylist );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace
{
// Create a stable, anonymized hash of username for privacy-preserving telemetry
// Uses SHA-256 to ensure consistent hashing across sessions while protecting user identity
std::string hashUsername( const std::string& username )
{
    if ( username.empty() )
    {
        return "";
    }

    QByteArray usernameBytes = QString::fromStdString( username ).toUtf8();
    QByteArray hash          = QCryptographicHash::hash( usernameBytes, QCryptographicHash::Sha256 );

    // Convert to hex string for readability in telemetry
    return hash.toHex().toStdString();
}

void addOsInfo( std::map<std::string, std::string>& attributes )
{
    attributes["os.type"]    = QSysInfo::productType().toStdString();
    attributes["os.version"] = QSysInfo::productVersion().toStdString();
    attributes["os.name"]    = QSysInfo::prettyProductName().toStdString();
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOpenTelemetryManager::reportEventAsync( const std::string& eventName, const std::map<std::string, std::string>& attributes )
{
    const bool isCrashEvent = eventName.starts_with( "crash." );

    if ( !isCrashEvent && ( !isEnabled() || isCircuitBreakerOpen() ) )
    {
        return;
    }

    if ( !isEventAllowed( eventName ) )
    {
        return;
    }

    std::unique_lock<std::mutex> lock( m_queueMutex );

    // Check queue size and apply backpressure (crash events always bypass the queue limit)
    if ( !isCrashEvent && m_backpressureEnabled && m_eventQueue.size() >= m_maxQueueSize )
    {
        m_healthMetrics.eventsDropped++;
        return;
    }

    // Create mutable copy and add hashed username if configured (privacy-preserving for regular events)
    // Note: crash events already have real username added in reportCrash()
    std::map<std::string, std::string> enrichedAttributes = attributes;

    if ( !isCrashEvent )
    {
        std::lock_guard<std::mutex> configLock( m_configMutex );
        if ( !m_username.empty() )
        {
            enrichedAttributes["user.hash"] = hashUsername( m_username );
        }

        addOsInfo( enrichedAttributes );
    }

    m_eventQueue.emplace( eventName, enrichedAttributes );
    m_healthMetrics.eventsQueued++;
}

//--------------------------------------------------------------------------------------------------
/// Helper function to extract the relevant part of the file path starting from "ResInsight"
/// Returns empty string if "ResInsight" is not found in the path
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

    // Format stack trace as string for logging
    std::stringstream ss;
    int               frame = 0;
    for ( const auto& entry : trace )
    {
        std::string relevantPath = extractRelevantPath( entry.source_file() );
        ss << "  [" << frame++ << "] " << entry.description() << " at " << relevantPath << ":" << entry.source_line() << "\n";
    }
    std::string rawStackTrace = ss.str();

    // Store structured stack trace data for Application Insights parsedStack
    QJsonArray jsonFrames;
    int        frameIndex = 0;
    for ( const auto& entry : trace )
    {
        std::string relevantPath = extractRelevantPath( entry.source_file() );
        std::string methodName   = entry.description();

        // Ensure method name is never empty (Application Insights requirement)
        if ( methodName.empty() )
        {
            methodName = "UnknownMethod";
        }

        int lineNumber = static_cast<int>( entry.source_line() );

        QJsonObject frame;
        frame["level"]    = frameIndex;
        frame["method"]   = QString::fromStdString( methodName );
        frame["fileName"] = QString::fromStdString( relevantPath );
        frame["line"]     = lineNumber;

        jsonFrames.append( frame );
        frameIndex++;
    }

    std::map<std::string, std::string> attributes;
    attributes["crash.signal"]            = std::to_string( signalCode );
    attributes["crash.thread_id"]         = std::to_string( std::hash<std::thread::id>{}( std::this_thread::get_id() ) );
    attributes["crash.stack_trace"]       = rawStackTrace;
    attributes["crash.parsed_stack_json"] = QString::fromUtf8( QJsonDocument( jsonFrames ).toJson( QJsonDocument::Compact ) ).toStdString();
    attributes["service.name"]            = RiaPreferencesOpenTelemetry::current()->serviceName().toStdString();
    attributes["service.version"]         = RiaPreferencesOpenTelemetry::current()->serviceVersion().toStdString();

    // Add system information
    addOsInfo( attributes );

    // Add real username for crash reports (not hashed - needed for debugging critical issues)
    {
        std::lock_guard<std::mutex> lock( m_configMutex );
        if ( !m_username.empty() )
        {
            attributes["user.name"] = m_username;
        }
    }

    reportEventAsync( "crash.signal_handler", attributes );
    flushPendingEvents();

    // Wait for the pending network reply to complete before the process exits.
    // flushPendingEvents() initiates an async HTTP POST; without pumping the event loop here
    // the reply never gets a chance to finish and the crash is silently dropped.
    if ( m_pendingReplies.load() > 0 )
    {
        auto* prefs          = RiaPreferencesOpenTelemetry::current();
        int   timeoutMs      = prefs ? prefs->connectionTimeoutMs() : 5000;
        int   elapsedMs      = 0;
        int   pollIntervalMs = 50;
        while ( m_pendingReplies.load() > 0 && elapsedMs < timeoutMs )
        {
            QEventLoop loop;
            QTimer::singleShot( pollIntervalMs, &loop, &QEventLoop::quit );
            loop.exec();
            elapsedMs += pollIntervalMs;
        }
    }

    RiaLogging::error( QString( "Crash reported to OpenTelemetry (signal: %1)" ).arg( signalCode ) );
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
void RiaOpenTelemetryManager::setUsername( const std::string& username )
{
    std::lock_guard<std::mutex> lock( m_configMutex );
    // Store original username - will be hashed for regular events but kept plain for crash reports
    m_username = username;
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

        // Determine if this is a crash event
        bool isCrashEvent = ( event.name == "crash.signal_handler" );

        QMap<QString, QVariant> baseData;
        QMap<QString, QVariant> data;
        QMap<QString, QVariant> telemetryItem;

        if ( isCrashEvent )
        {
            // Create ExceptionData for crash reports
            QMap<QString, QVariant> exception;
            exception["typeName"]     = QString( "ResInsightCrash" );
            exception["message"]      = QString( "Application crash (signal: %1)" ).arg( properties["crash.signal"].toString() );
            exception["hasFullStack"] = true;
            exception["stack"]        = properties["crash.stack_trace"];

            // Parse structured stack trace for Application Insights
            QString parsedStackJson = properties["crash.parsed_stack_json"].toString();
            if ( !parsedStackJson.isEmpty() )
            {
                // Parse the JSON array of stack frames
                QJsonDocument doc = QJsonDocument::fromJson( parsedStackJson.toUtf8() );
                if ( doc.isArray() )
                {
                    exception["parsedStack"] = doc.array().toVariantList();
                }
            }

            QList<QVariant> exceptions;
            exceptions.append( exception );

            // Remove stack trace data from properties as it's now in the exception
            properties.remove( "crash.stack_trace" );
            properties.remove( "crash.parsed_stack_json" );

            baseData["ver"]        = 2;
            baseData["exceptions"] = exceptions;
            baseData["properties"] = properties;

            data["baseType"] = "ExceptionData";
            data["baseData"] = baseData;

            telemetryItem["name"] = "Microsoft.ApplicationInsights.Exception";
        }
        else
        {
            // Create EventData for regular events
            baseData["ver"]        = 2;
            baseData["name"]       = QString::fromStdString( event.name );
            baseData["properties"] = properties;

            data["baseType"] = "EventData";
            data["baseData"] = baseData;

            telemetryItem["name"] = "Microsoft.ApplicationInsights.Event";
        }

        telemetryItem["time"] = QString::fromStdString( timestamp );
        telemetryItem["iKey"] = connectionParams["InstrumentationKey"];
        telemetryItem["data"] = data;

        // Convert to JSON string
        QVariantList envelope;
        envelope.append( telemetryItem );
        QString jsonPayload = ResInsightInternalJson::Json::encode( envelope, false );

        // Send to Application Insights using QNetworkAccessManager
        QString url = connectionParams["IngestionEndpoint"] + "v2/track";

        QNetworkRequest request;
        request.setUrl( QUrl( url ) );
        request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
        request.setHeader( QNetworkRequest::KnownHeaders( QNetworkRequest::UserAgentHeader ), "ResInsight-OpenTelemetry" );
        request.setTransferTimeout( prefs->connectionTimeoutMs() );

        m_pendingReplies++;
        QNetworkReply* reply = m_networkAccessManager->post( request, jsonPayload.toUtf8() );

        // Handle response asynchronously
        connect( reply,
                 &QNetworkReply::finished,
                 this,
                 [this, reply]()
                 {
                     const int     statusCode   = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
                     const QString responseBody = QString::fromUtf8( reply->readAll() );
                     if ( reply->error() == QNetworkReply::NoError )
                     {
                         updateHealthMetrics( true );
                         resetCircuitBreaker();
                     }
                     else
                     {
                         const QString errorMsg =
                             QString( "HTTP %1: %2 (%3)" ).arg( statusCode ).arg( reply->errorString() ).arg( responseBody );
                         handleError( TelemetryError::NetworkError, QString( "Failed to send telemetry: %1" ).arg( errorMsg ) );
                         updateHealthMetrics( false );
                     }
                     m_pendingReplies--;
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