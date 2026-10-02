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

#include "RiaCloudApiService.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaQStringFormatter.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QTcpServer>
#include <QUrl>

#include <algorithm>
#include <limits>

namespace
{
// Grace period after starting the process before the first health check, to allow for the
// initial uvicorn boot and module imports. Kept short so that the reported status settles quickly;
// a service that needs longer is covered by the health check retries rather than by waiting here.
constexpr int startupGraceMs = 4 * 1000;

// Poll the /alive endpoint at this interval.
constexpr int healthCheckIntervalMs = 10 * 1000;

// Restart the service after this many consecutive failed health checks.
constexpr int maxConsecutiveFailures = 2;

// Per-request timeout for the health check, kept well below the poll interval.
constexpr int healthCheckTimeoutMs = 5 * 1000;

// Interval between the /alive attempts made while waiting for the service to come up. Much shorter than
// the periodic health check, as a caller is blocked meanwhile.
constexpr int readinessPollIntervalMs = 250;

// Number of ports scanned, starting at the wanted port, when looking for a free port to bind to.
constexpr int portRangeLength = 100;

// Shared hint appended to errors caused by a service that never responded, pointing at the most likely
// fix without claiming to have diagnosed the exact cause.
const std::string pythonEnvironmentHint =
    "This could be caused by the configured Python environment (Preferences -> RI Cloud API, or "
    "Preferences -> Scripting) missing the packages required by ri-cloud-api. Check the log above for "
    "details, and fix the environment if needed.";
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCloudApiService::RiaCloudApiService( const QString& serverAddress, int wantedPort, QObject* parent )
    : QObject( parent )
    , m_process( nullptr )
    , m_networkAccessManager( new QNetworkAccessManager( this ) )
    , m_serverAddress( serverAddress )
    , m_wantedPort( wantedPort )
    , m_port( -1 )
    , m_consecutiveFailures( 0 )
    , m_consecutiveLaunchFailures( 0 )
    , m_isResponding( false )
    , m_hasRespondedSinceStart( false )
    , m_hasGivenUp( false )
{
    // Never route loopback requests through a system/corporate proxy, which can add several seconds
    // per request.
    m_networkAccessManager->setProxy( QNetworkProxy::NoProxy );

    m_healthTimer.setInterval( healthCheckIntervalMs );
    connect( &m_healthTimer, &QTimer::timeout, this, &RiaCloudApiService::onHealthCheck );

    // After the startup grace period has elapsed, begin periodic health checks.
    m_startupTimer.setSingleShot( true );
    m_startupTimer.setInterval( startupGraceMs );
    connect( &m_startupTimer,
             &QTimer::timeout,
             this,
             [this]()
             {
                 // A QTimer does not fire on start, so check once here instead of waiting a full
                 // interval. Start the timer first, since the check may restart the service.
                 m_healthTimer.start();
                 onHealthCheck();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCloudApiService::~RiaCloudApiService()
{
    stop();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaCloudApiService::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaCloudApiService::isResponding() const
{
    return isRunning() && m_isResponding;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaCloudApiService::port() const
{
    return m_port;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudApiService::serverUrl() const
{
    if ( m_port < 0 ) return {};

    return QString( "%1:%2" ).arg( m_serverAddress ).arg( m_port );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudApiService::start( StartTrigger trigger )
{
    // An Automatic call (e.g. Sumo's tokenReady, which can fire well after the give-up, such as on a
    // background token refresh) must not silently re-arm a retry loop that already gave up. Only an
    // explicit Manual start/restart may do that.
    if ( trigger == StartTrigger::Automatic && m_hasGivenUp ) return;

    if ( isRunning() ) return;

    m_consecutiveLaunchFailures = 0;
    m_hasRespondedSinceStart    = false;
    m_hasGivenUp                = false;

    launchProcess();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudApiService::launchProcess()
{
    // Clean up any stale QProcess instance (e.g. after an unexpected exit) before re-starting.
    if ( m_process ) stop();

    const QString pythonExecutable = RiaApplication::instance()->cloudApiPythonPath();
    if ( pythonExecutable.isEmpty() )
    {
        RiaLogging::error( "Cloud API service: no Python executable configured, cannot start service." );
        return;
    }

    const QString workingDirectory = serviceWorkingDirectory();
    if ( workingDirectory.isEmpty() )
    {
        RiaLogging::error( "Cloud API service: could not locate the 'ri_cloud_api' service directory, cannot start service." );
        return;
    }

    const QString host = QUrl( m_serverAddress ).host();
    if ( host.isEmpty() )
    {
        RiaLogging::error( std::format( "Cloud API service: invalid server address '{}', cannot start service.", m_serverAddress ) );
        return;
    }

    const int port = findAvailablePortNumber( m_wantedPort );
    if ( port < 0 )
    {
        RiaLogging::error( "Cloud API service: no available port found, cannot start service." );
        return;
    }
    m_port = port;

    QStringList arguments;
    arguments << "-m"
              << "uvicorn"
              << "ri_cloud_api.main:app"
              << "--host" << host << "--port" << QString::number( m_port );

    m_process = new QProcess( this );
    m_process->setWorkingDirectory( workingDirectory );
    m_process->setProcessEnvironment( buildProcessEnvironment( workingDirectory ) );
    m_process->setProcessChannelMode( QProcess::MergedChannels );

    // Forward the server's stdout/stderr to the ResInsight log so boot failures (e.g. a missing
    // 'uvicorn' or import errors in the service) are visible.
    connect( m_process, &QProcess::readyReadStandardOutput, this, &RiaCloudApiService::onReadyReadStandardOutput );

    connect( m_process,
             &QProcess::errorOccurred,
             this,
             [this]( QProcess::ProcessError )
             { RiaLogging::error( std::format( "Cloud API service: process error: {}", m_process->errorString() ) ); } );

    connect( m_process,
             &QProcess::finished,
             this,
             [this]( int exitCode, QProcess::ExitStatus exitStatus )
             {
                 const std::string message = std::format( "Cloud API service: process finished (exit code {}).", exitCode );

                 // A non-zero exit means the service never came up, or died. Anything else is an
                 // ordinary shutdown.
                 const bool failed = exitCode != 0 || exitStatus != QProcess::NormalExit;
                 if ( failed )
                 {
                     RiaLogging::error( message );
                 }
                 else
                 {
                     RiaLogging::info( message );
                 }

                 m_isResponding = false;
                 emit statusChanged();

                 // React immediately rather than waiting for a timer. Deferred via singleShot, since
                 // restarting the process here, during its own finished() signal, would not be safe.
                 if ( failed )
                 {
                     m_startupTimer.stop();
                     m_healthTimer.stop();
                     QTimer::singleShot( 0, this, &RiaCloudApiService::autoRestart );
                 }
             } );

    m_consecutiveFailures = 0;
    m_isResponding        = false;

    RiaLogging::info( std::format( "Cloud API service: launching '{} {}' (working directory '{}').",
                                   pythonExecutable,
                                   arguments.join( ' ' ),
                                   workingDirectory ) );

    m_process->start( pythonExecutable, arguments );

    RiaLogging::info( std::format( "Cloud API service: starting on {}.", serverUrl() ) );

    emit statusChanged();

    // Delay the first health check by the startup grace period to allow the server to boot.
    m_startupTimer.start();
}

//--------------------------------------------------------------------------------------------------
/// The periodic health check first runs a grace period after the launch, which is far too long to make a
/// caller wait for. Poll the service directly instead, so a request can be issued as soon as it answers.
//--------------------------------------------------------------------------------------------------
bool RiaCloudApiService::waitUntilResponding( int timeoutMs )
{
    if ( m_isResponding ) return true;

    if ( !isRunning() ) start( StartTrigger::Automatic );

    // Nested event loops below let the periodic health check fire and restart() the service while this
    // call is still waiting for it to boot. Stop both timers for the duration of the wait to avoid that.
    m_startupTimer.stop();
    m_healthTimer.stop();

    QElapsedTimer elapsed;
    elapsed.start();

    while ( elapsed.elapsed() < timeoutMs )
    {
        if ( m_port >= 0 )
        {
            QNetworkRequest request( QUrl( serverUrl() + "/alive" ) );
            request.setTransferTimeout( healthCheckTimeoutMs );

            QNetworkReply* reply = m_networkAccessManager->get( request );

            QEventLoop eventLoop;
            QObject::connect( reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit );
            eventLoop.exec();

            const int  statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
            const bool answered   = reply->error() == QNetworkReply::NoError && statusCode == 200;
            reply->deleteLater();

            if ( answered )
            {
                m_consecutiveFailures       = 0;
                m_consecutiveLaunchFailures = 0;
                m_hasRespondedSinceStart    = true;

                if ( !m_isResponding )
                {
                    m_isResponding = true;
                    emit statusChanged();
                }

                // The service is known to be up, so the grace period before the periodic check has served
                // its purpose. Fall back to the ordinary health check from here.
                m_startupTimer.stop();
                if ( !m_healthTimer.isActive() ) m_healthTimer.start();

                return true;
            }
        }

        // The process may still be booting, or may have exited on a missing dependency.
        if ( !isRunning() ) break;

        QEventLoop delayLoop;
        QTimer     delayTimer;
        delayTimer.setSingleShot( true );
        QObject::connect( &delayTimer, &QTimer::timeout, &delayLoop, &QEventLoop::quit );
        delayTimer.start( readinessPollIntervalMs );
        delayLoop.exec();
    }

    if ( m_hasGivenUp )
    {
        RiaLogging::error( std::format( "Cloud API service: not available, giving up. {}", pythonEnvironmentHint ) );
    }
    else
    {
        RiaLogging::error( std::format( "Cloud API service: not responding after {} ms, giving up.", elapsed.elapsed() ) );
    }

    // Resume background health-checking if the process is still running, so it can still recover on
    // its own without another blocking wait.
    if ( isRunning() ) m_healthTimer.start();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudApiService::stop()
{
    m_startupTimer.stop();
    m_healthTimer.stop();

    if ( m_process )
    {
        // Disconnect to avoid acting on the finished() signal during a deliberate shutdown.
        m_process->disconnect( this );
        if ( m_process->state() != QProcess::NotRunning )
        {
            m_process->kill();
            m_process->waitForFinished( 3000 );
        }
        delete m_process;
        m_process = nullptr;
    }

    m_port         = -1;
    m_isResponding = false;

    emit statusChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudApiService::restart()
{
    RiaLogging::warning( "Cloud API service: not responding, restarting." );
    stop();
    start();
}

//--------------------------------------------------------------------------------------------------
/// Called when the process exits/crashes, or when a health check times out. If the service has never
/// answered a health check since the current start (see m_hasRespondedSinceStart), this is treated as a
/// configuration problem rather than a transient crash: retrying is unlikely to help, so this gives up
/// immediately instead of restarting, and sets m_hasGivenUp so a later Automatic start() (e.g. Sumo's
/// tokenReady) does not quietly re-arm the loop. If the service HAD been responding, this is instead
/// treated as an ordinary crash of an otherwise working server, and is retried no matter how many times it
/// takes to come back.
//--------------------------------------------------------------------------------------------------
void RiaCloudApiService::autoRestart()
{
    m_consecutiveLaunchFailures++;

    if ( !m_hasRespondedSinceStart )
    {
        RiaLogging::error( std::format( "Cloud API service: giving up after {} failed launch attempt{}. {}",
                                        m_consecutiveLaunchFailures,
                                        m_consecutiveLaunchFailures == 1 ? "" : "s",
                                        pythonEnvironmentHint ) );
        m_hasGivenUp = true;
        stop();
        return;
    }

    RiaLogging::warning( "Cloud API service: not responding, restarting." );
    stop();
    launchProcess();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudApiService::onReadyReadStandardOutput()
{
    const QString output = QString::fromLocal8Bit( m_process->readAllStandardOutput() );
    for ( const QString& line : output.split( '\n', Qt::SkipEmptyParts ) )
    {
        RiaLogging::debug( std::format( "Cloud API service: {}", line.trimmed() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudApiService::onHealthCheck()
{
    if ( !isRunning() || m_port < 0 )
    {
        autoRestart();
        return;
    }

    QNetworkRequest request( QUrl( serverUrl() + "/alive" ) );
    request.setTransferTimeout( healthCheckTimeoutMs );

    QNetworkReply* reply = m_networkAccessManager->get( request );
    connect( reply,
             &QNetworkReply::finished,
             this,
             [this, reply]()
             {
                 reply->deleteLater();

                 const bool wasResponding = m_isResponding;

                 const int statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
                 if ( reply->error() == QNetworkReply::NoError && statusCode == 200 )
                 {
                     m_consecutiveFailures       = 0;
                     m_consecutiveLaunchFailures = 0;
                     m_hasRespondedSinceStart    = true;
                     m_isResponding              = true;
                 }
                 else
                 {
                     m_isResponding = false;
                     m_consecutiveFailures++;
                 }

                 // Notify before a possible restart, so the transition out of "responding" is seen.
                 if ( m_isResponding != wasResponding ) emit statusChanged();

                 if ( !m_isResponding && m_consecutiveFailures >= maxConsecutiveFailures )
                 {
                     autoRestart();
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaCloudApiService::findAvailablePortNumber( int firstPort )
{
    const int endPort = std::min( firstPort + portRangeLength, (int)std::numeric_limits<quint16>::max() );

    QTcpServer serverTest;
    for ( quint16 port = static_cast<quint16>( firstPort ); port <= static_cast<quint16>( endPort ); ++port )
    {
        if ( serverTest.listen( QHostAddress::LocalHost, port ) )
        {
            serverTest.close();
            return static_cast<int>( port );
        }
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudApiService::serviceWorkingDirectory()
{
    // Probe a set of candidate locations and return the first one that actually contains the
    // 'ri_cloud_api' package.
    const QString appDir = QCoreApplication::applicationDirPath();

    QStringList candidates;

    // Location relative to the executable, used for an installed ResInsight. Probed first so that
    // an installed build always uses the copy it shipped with. The preferences are shared by every
    // ResInsight on the machine, so "Shared Script Folder(s)" may well point at a source checkout
    // set up for a development build, and would otherwise shadow the installed copy.
    candidates << appDir + "/CloudServiceApi/";

    // Configured "Shared Script Folder(s)" (semicolon-separated, defaults to the Python examples
    // folder). This is what a development build relies on, since its executable lives in the build
    // tree and not next to the Python examples.
    const QStringList scriptDirectories = RiaApplication::instance()->scriptDirectories().split( ';', Qt::SkipEmptyParts );
    for ( const QString& scriptDir : scriptDirectories )
    {
        candidates << scriptDir.trimmed();
    }

    for ( const QString& candidate : candidates )
    {
        if ( candidate.isEmpty() ) continue;

        // Probe the candidate itself and a nested 'ri-cloud-api' repository folder, so a shared
        // script folder can point either at the repository or at the folder containing it.
        const QStringList probePaths = { candidate, candidate + "/ri-cloud-api" };
        for ( const QString& path : probePaths )
        {
            QDir dir( path );
            if ( dir.exists( "ri_cloud_api" ) )
            {
                return dir.absolutePath();
            }
        }
    }

    RiaLogging::error( std::format( "Cloud API service: 'ri_cloud_api' not found in any of: {}", candidates.join( ", " ) ) );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QProcessEnvironment RiaCloudApiService::buildProcessEnvironment( const QString& workingDirectory )
{
    // The service runs in the configured Python interpreter (typically a virtual environment), so
    // uvicorn/fastapi are resolved from that environment. The 'ri_cloud_api' package additionally
    // depends on local workspace libraries laid out as 'libs/<lib>/src', beside the package. Add the
    // discovered 'src' folders to PYTHONPATH so the service can be imported without a separate
    // install step.
    //
    // INTERIM: this PYTHONPATH injection is a temporary measure while the service runs from the
    // source tree. Once 'ri_cloud_api' is distributed and installed as a pip package, the workspace
    // libraries will resolve from site-packages and this block should be removed. See the package
    // README.md for details.
    QProcessEnvironment environment = RiaApplication::instance()->pythonProcessEnvironment();

    QStringList pythonPaths;

    QDir                libsDir( workingDirectory + "/libs" );
    const QFileInfoList libEntries = libsDir.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QFileInfo& libEntry : libEntries )
    {
        const QString srcPath = libEntry.absoluteFilePath() + "/src";
        if ( QDir( srcPath ).exists() )
        {
            pythonPaths << QDir::toNativeSeparators( srcPath );
        }
    }

    if ( !pythonPaths.isEmpty() )
    {
        const QString existing = environment.value( "PYTHONPATH" );
        if ( !existing.isEmpty() ) pythonPaths << existing;

        environment.insert( "PYTHONPATH", pythonPaths.join( QDir::listSeparator() ) );
    }

    return environment;
}
