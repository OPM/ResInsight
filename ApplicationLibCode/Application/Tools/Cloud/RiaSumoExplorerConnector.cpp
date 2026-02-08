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

#include "RiaSumoExplorerConnector.h"

#include "RiaLogging.h"
#include "RiaSumoExplorerDefines.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QThread>
#include <QTimer>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoExplorerConnector::RiaSumoExplorerConnector( QObject* parent, const QString& pythonPath, unsigned int port )
    : QObject( parent )
    , m_serverProcess( nullptr )
    , m_networkManager( new QNetworkAccessManager( this ) )
    , m_pythonPath( pythonPath )
    , m_port( port )
    , m_serverRunning( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoExplorerConnector::~RiaSumoExplorerConnector()
{
    stopServer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSumoExplorerConnector::startServer()
{
    if ( m_serverRunning )
    {
        RiaLogging::info( "Sumo Explorer server already running" );
        return true;
    }

    // Determine Python executable
    QString pythonCmd = m_pythonPath.isEmpty() ? "python" : m_pythonPath;

    // Determine server script path
    QString appPath    = QCoreApplication::applicationDirPath();
    QString serverPath = appPath + "/Python/sumo_explorer_server/sumo_explorer_server.py";

    if ( !QFile::exists( serverPath ) )
    {
        setError( QString( "Sumo Explorer server script not found: %1" ).arg( serverPath ) );
        return false;
    }

    // Create process
    m_serverProcess = new QProcess( this );

    connect( m_serverProcess, &QProcess::errorOccurred, this, &RiaSumoExplorerConnector::onProcessError );
    connect( m_serverProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &RiaSumoExplorerConnector::onProcessFinished );
    connect( m_serverProcess, &QProcess::readyReadStandardOutput, this, &RiaSumoExplorerConnector::onProcessReadyReadStandardOutput );
    connect( m_serverProcess, &QProcess::readyReadStandardError, this, &RiaSumoExplorerConnector::onProcessReadyReadStandardError );

    // Build command
    QStringList args;
    args << "-m"
         << "uvicorn"
         << "sumo_explorer_server.sumo_explorer_server:app"
         << "--host"
         << "127.0.0.1"
         << "--port" << QString::number( m_port );

    // Set working directory to Python directory
    QString pythonDir = appPath + "/Python";
    m_serverProcess->setWorkingDirectory( pythonDir );

    RiaLogging::info( QString( "Starting Sumo Explorer server: %1 %2" ).arg( pythonCmd ).arg( args.join( " " ) ) );

    // Start process
    m_serverProcess->start( pythonCmd, args );

    if ( !m_serverProcess->waitForStarted( 5000 ) )
    {
        QString errorMsg = QString( "Failed to start Sumo Explorer server process. Command: %1 %2" ).arg( pythonCmd ).arg( args.join( " " ) );
        setError( errorMsg );

        // Log process error details
        if ( m_serverProcess->error() == QProcess::FailedToStart )
        {
            RiaLogging::error( "Process failed to start. Check that Python is installed and in PATH." );
        }

        delete m_serverProcess;
        m_serverProcess = nullptr;
        return false;
    }

    RiaLogging::info( "Python process started, waiting for server to be ready..." );

    // Wait for server to be ready
    if ( !waitForServerReady() )
    {
        // Read any error output before stopping
        if ( m_serverProcess )
        {
            QString stdErr = QString::fromUtf8( m_serverProcess->readAllStandardError() );
            QString stdOut = QString::fromUtf8( m_serverProcess->readAllStandardOutput() );

            if ( !stdErr.isEmpty() )
            {
                RiaLogging::error( QString( "Server stderr: %1" ).arg( stdErr ) );
            }
            if ( !stdOut.isEmpty() )
            {
                RiaLogging::info( QString( "Server stdout: %1" ).arg( stdOut ) );
            }
        }

        setError( QString( "Sumo Explorer server failed to become ready after 10 seconds. Check that uvicorn and required packages are installed (run: pip install -r %1/Python/sumo_explorer_server/requirements.txt)" ).arg( appPath ) );
        stopServer();
        return false;
    }

    m_serverRunning = true;
    RiaLogging::info( QString( "Sumo Explorer server started on port %1" ).arg( m_port ) );
    emit serverStarted();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::stopServer()
{
    if ( !m_serverProcess ) return;

    RiaLogging::info( "Stopping Sumo Explorer server" );

    m_serverProcess->terminate();

    if ( !m_serverProcess->waitForFinished( 5000 ) )
    {
        RiaLogging::warning( "Sumo Explorer server did not terminate, killing process" );
        m_serverProcess->kill();
        m_serverProcess->waitForFinished( 1000 );
    }

    delete m_serverProcess;
    m_serverProcess = nullptr;
    m_serverRunning = false;

    emit serverStopped();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSumoExplorerConnector::isServerRunning() const
{
    return m_serverRunning;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSumoExplorerConnector::waitForServerReady( int timeoutMs )
{
    QString healthUrl = makeUrl( "/health" );

    QElapsedTimer timer;
    timer.start();

    int attemptCount = 0;

    while ( timer.elapsed() < timeoutMs )
    {
        attemptCount++;
        RiaLogging::debug( QString( "Health check attempt %1, URL: %2" ).arg( attemptCount ).arg( healthUrl ) );

        QNetworkRequest request( healthUrl );
        QNetworkReply*  reply = m_networkManager->get( request );

        QEventLoop loop;
        connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );

        QTimer timeoutTimer;
        timeoutTimer.setSingleShot( true );
        connect( &timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit );
        timeoutTimer.start( 1000 );

        loop.exec();

        if ( reply->error() == QNetworkReply::NoError )
        {
            RiaLogging::info( QString( "Server health check succeeded after %1 attempts" ).arg( attemptCount ) );
            reply->deleteLater();
            return true;
        }
        else
        {
            RiaLogging::debug( QString( "Health check failed: %1" ).arg( reply->errorString() ) );
        }

        reply->deleteLater();

        // Wait a bit before retrying
        QThread::msleep( 500 );
    }

    RiaLogging::error( QString( "Server failed to respond to health checks after %1 attempts over %2ms" ).arg( attemptCount ).arg( timeoutMs ) );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestAssetsBlocking()
{
    auto requestCallable = [this]() { requestAssets(); };
    auto replyHandler    = [this]( QNetworkReply* reply ) { parseAssetsReply( reply ); };
    wrapAndCallNetworkRequest( requestCallable, replyHandler );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestCasesForFieldBlocking( const QString& fieldName )
{
    auto requestCallable = [this, fieldName]() { requestCasesForField( fieldName ); };
    auto replyHandler    = [this]( QNetworkReply* reply ) { parseCasesReply( reply ); };
    wrapAndCallNetworkRequest( requestCallable, replyHandler );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestEnsemblesForCaseBlocking( const QString& caseId )
{
    auto requestCallable = [this, caseId]() { requestEnsemblesForCase( caseId ); };
    auto replyHandler    = [this]( QNetworkReply* reply ) { parseEnsemblesReply( reply ); };
    wrapAndCallNetworkRequest( requestCallable, replyHandler );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestVectorNamesBlocking( const QString& caseId, const QString& ensembleName )
{
    auto requestCallable = [this, caseId, ensembleName]() { requestVectorNames( caseId, ensembleName ); };
    auto replyHandler    = [this]( QNetworkReply* reply ) { parseVectorNamesReply( reply ); };
    wrapAndCallNetworkRequest( requestCallable, replyHandler );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestRealizationIdsBlocking( const QString& caseId, const QString& ensembleName )
{
    auto requestCallable = [this, caseId, ensembleName]() { requestRealizationIds( caseId, ensembleName ); };
    auto replyHandler    = [this]( QNetworkReply* reply ) { parseRealizationIdsReply( reply ); };
    wrapAndCallNetworkRequest( requestCallable, replyHandler );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoExplorerConnector::requestSummaryDataBlocking( const QString& caseId, const QString& ensembleName, const QString& vectorName )
{
    QString url = makeUrl( QString( "/summary/data?case_id=%1&ensemble=%2&vector=%3" )
                               .arg( QString( QUrl::toPercentEncoding( caseId ) ) )
                               .arg( QString( QUrl::toPercentEncoding( ensembleName ) ) )
                               .arg( QString( QUrl::toPercentEncoding( vectorName ) ) ) );

    QByteArray response = executeGetRequest( url );
    if ( response.isEmpty() ) return {};

    QJsonDocument doc    = QJsonDocument::fromJson( response );
    QJsonObject   obj    = doc.object();
    QString       base64 = obj["data_base64"].toString();

    return base64ToBytes( base64 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoExplorerConnector::requestParametersBlocking( const QString& caseId, const QString& ensembleName )
{
    QString url = makeUrl( QString( "/summary/parameters?case_id=%1&ensemble=%2" )
                               .arg( QString( QUrl::toPercentEncoding( caseId ) ) )
                               .arg( QString( QUrl::toPercentEncoding( ensembleName ) ) ) );

    QByteArray response = executeGetRequest( url );
    if ( response.isEmpty() ) return {};

    QJsonDocument doc    = QJsonDocument::fromJson( response );
    QJsonObject   obj    = doc.object();
    QString       base64 = obj["data_base64"].toString();

    return base64ToBytes( base64 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestAssets()
{
    if ( !m_serverRunning )
    {
        RiaLogging::error( "Sumo Explorer server not running. Please start the server first." );
        return;
    }

    QString url = makeUrl( "/assets" );
    RiaLogging::debug( QString( "Requesting assets from: %1" ).arg( url ) );

    QNetworkRequest request( url );
    QNetworkReply*  reply = m_networkManager->get( request );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, url]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseAssetsReply( reply );
                 }
                 else
                 {
                     RiaLogging::error( QString( "Failed to request assets from %1: %2" ).arg( url ).arg( reply->errorString() ) );
                     if ( reply->error() == QNetworkReply::ConnectionRefusedError )
                     {
                         m_serverRunning = false;
                         RiaLogging::error( "Server connection refused. The server may have stopped or failed to start properly." );
                     }
                 }
                 reply->deleteLater();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestCasesForField( const QString& fieldName )
{
    if ( !m_serverRunning )
    {
        RiaLogging::error( "Sumo Explorer server not running" );
        return;
    }

    QString         url = makeUrl( QString( "/cases/%1" ).arg( QString( QUrl::toPercentEncoding( fieldName ) ) ) );
    QNetworkRequest request( url );
    QNetworkReply*  reply = m_networkManager->get( request );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseCasesReply( reply );
                 }
                 else
                 {
                     RiaLogging::error( QString( "Failed to request cases: %1" ).arg( reply->errorString() ) );
                 }
                 reply->deleteLater();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestEnsemblesForCase( const QString& caseId )
{
    if ( !m_serverRunning )
    {
        RiaLogging::error( "Sumo Explorer server not running" );
        return;
    }

    QString         url = makeUrl( QString( "/ensembles/%1" ).arg( QString( QUrl::toPercentEncoding( caseId ) ) ) );
    QNetworkRequest request( url );
    QNetworkReply*  reply = m_networkManager->get( request );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseEnsemblesReply( reply );
                 }
                 else
                 {
                     RiaLogging::error( QString( "Failed to request ensembles: %1" ).arg( reply->errorString() ) );
                 }
                 reply->deleteLater();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestVectorNames( const QString& caseId, const QString& ensembleName )
{
    if ( !m_serverRunning )
    {
        RiaLogging::error( "Sumo Explorer server not running" );
        return;
    }

    QString url = makeUrl( QString( "/summary/vectors?case_id=%1&ensemble=%2" )
                               .arg( QString( QUrl::toPercentEncoding( caseId ) ) )
                               .arg( QString( QUrl::toPercentEncoding( ensembleName ) ) ) );

    QNetworkRequest request( url );
    QNetworkReply*  reply = m_networkManager->get( request );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseVectorNamesReply( reply );
                 }
                 else
                 {
                     RiaLogging::error( QString( "Failed to request vector names: %1" ).arg( reply->errorString() ) );
                 }
                 reply->deleteLater();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::requestRealizationIds( const QString& caseId, const QString& ensembleName )
{
    if ( !m_serverRunning )
    {
        RiaLogging::error( "Sumo Explorer server not running" );
        return;
    }

    QString url = makeUrl( QString( "/summary/realizations?case_id=%1&ensemble=%2" )
                               .arg( QString( QUrl::toPercentEncoding( caseId ) ) )
                               .arg( QString( QUrl::toPercentEncoding( ensembleName ) ) ) );

    QNetworkRequest request( url );
    QNetworkReply*  reply = m_networkManager->get( request );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseRealizationIdsReply( reply );
                 }
                 else
                 {
                     RiaLogging::error( QString( "Failed to request realization IDs: %1" ).arg( reply->errorString() ) );
                 }
                 reply->deleteLater();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoExplorerAsset> RiaSumoExplorerConnector::assets() const
{
    return m_assets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoExplorerCase> RiaSumoExplorerConnector::cases() const
{
    return m_cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoExplorerEnsemble> RiaSumoExplorerConnector::ensembles() const
{
    return m_ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoExplorerVectorInfo> RiaSumoExplorerConnector::vectorNames() const
{
    return m_vectorNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoExplorerRealizationInfo> RiaSumoExplorerConnector::realizationIds() const
{
    return m_realizationIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoExplorerConnector::serverUrl() const
{
    return QString( "http://127.0.0.1:%1" ).arg( m_port );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoExplorerConnector::lastError() const
{
    return m_lastError;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::onProcessError( QProcess::ProcessError error )
{
    QString errorMsg;
    switch ( error )
    {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start Sumo Explorer server (Python not found or script missing)";
            break;
        case QProcess::Crashed:
            errorMsg = "Sumo Explorer server crashed";
            break;
        default:
            errorMsg = QString( "Sumo Explorer server error: %1" ).arg( static_cast<int>( error ) );
            break;
    }

    setError( errorMsg );
    m_serverRunning = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    if ( exitStatus == QProcess::CrashExit )
    {
        RiaLogging::error( QString( "Sumo Explorer server crashed with exit code %1" ).arg( exitCode ) );
    }
    else if ( exitCode != 0 )
    {
        RiaLogging::warning( QString( "Sumo Explorer server exited with code %1" ).arg( exitCode ) );
    }

    m_serverRunning = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::onProcessReadyReadStandardOutput()
{
    if ( !m_serverProcess ) return;

    QByteArray data = m_serverProcess->readAllStandardOutput();
    logServerOutput( QString::fromUtf8( data ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::onProcessReadyReadStandardError()
{
    if ( !m_serverProcess ) return;

    QByteArray data   = m_serverProcess->readAllStandardError();
    QString    output = QString::fromUtf8( data );

    // Log error output
    if ( !output.trimmed().isEmpty() )
    {
        RiaLogging::warning( QString( "Sumo Explorer server: %1" ).arg( output.trimmed() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::parseAssetsReply( QNetworkReply* reply )
{
    m_assets.clear();

    QByteArray    data = reply->readAll();
    QJsonDocument doc  = QJsonDocument::fromJson( data );

    if ( !doc.isArray() )
    {
        RiaLogging::error( "Invalid assets response format" );
        emit assetsFinished();
        return;
    }

    QJsonArray array = doc.array();
    for ( const QJsonValue& value : array )
    {
        QJsonObject obj = value.toObject();

        SumoExplorerAsset asset;
        asset.assetId = obj["asset_id"].toString();
        asset.kind    = obj["kind"].toString();
        asset.name    = obj["name"].toString();

        m_assets.push_back( asset );
    }

    emit assetsFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::parseCasesReply( QNetworkReply* reply )
{
    m_cases.clear();

    QByteArray    data = reply->readAll();
    QJsonDocument doc  = QJsonDocument::fromJson( data );

    if ( !doc.isArray() )
    {
        RiaLogging::error( "Invalid cases response format" );
        emit casesFinished();
        return;
    }

    QJsonArray array = doc.array();
    for ( const QJsonValue& value : array )
    {
        QJsonObject obj = value.toObject();

        SumoExplorerCase sumoCase;
        sumoCase.caseId  = obj["case_id"].toString();
        sumoCase.kind    = obj["kind"].toString();
        sumoCase.name    = obj["name"].toString();
        sumoCase.assetId = obj["asset_id"].toString();

        m_cases.push_back( sumoCase );
    }

    emit casesFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::parseEnsemblesReply( QNetworkReply* reply )
{
    m_ensembles.clear();

    QByteArray    data = reply->readAll();
    QJsonDocument doc  = QJsonDocument::fromJson( data );

    if ( !doc.isArray() )
    {
        RiaLogging::error( "Invalid ensembles response format" );
        emit ensemblesFinished();
        return;
    }

    QJsonArray array = doc.array();
    for ( const QJsonValue& value : array )
    {
        QJsonObject obj = value.toObject();

        SumoExplorerEnsemble ensemble;
        ensemble.ensembleName = obj["ensemble_name"].toString();
        ensemble.caseId       = obj["case_id"].toString();

        m_ensembles.push_back( ensemble );
    }

    emit ensemblesFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::parseVectorNamesReply( QNetworkReply* reply )
{
    m_vectorNames.clear();

    QByteArray    data = reply->readAll();
    QJsonDocument doc  = QJsonDocument::fromJson( data );

    if ( !doc.isArray() )
    {
        RiaLogging::error( "Invalid vector names response format" );
        emit vectorNamesFinished();
        return;
    }

    QJsonArray array = doc.array();
    for ( const QJsonValue& value : array )
    {
        QJsonObject obj = value.toObject();

        SumoExplorerVectorInfo vectorInfo;
        vectorInfo.name = obj["name"].toString();

        m_vectorNames.push_back( vectorInfo );
    }

    emit vectorNamesFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::parseRealizationIdsReply( QNetworkReply* reply )
{
    m_realizationIds.clear();

    QByteArray    data = reply->readAll();
    QJsonDocument doc  = QJsonDocument::fromJson( data );

    if ( !doc.isArray() )
    {
        RiaLogging::error( "Invalid realization IDs response format" );
        emit realizationIdsFinished();
        return;
    }

    QJsonArray array = doc.array();
    for ( const QJsonValue& value : array )
    {
        QJsonObject obj = value.toObject();

        SumoExplorerRealizationInfo realInfo;
        realInfo.realizationId = obj["realization_id"].toInt();

        m_realizationIds.push_back( realInfo );
    }

    emit realizationIdsFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoExplorerConnector::makeUrl( const QString& path ) const
{
    return QString( "http://127.0.0.1:%1%2" ).arg( m_port ).arg( path );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::logServerOutput( const QString& output )
{
    if ( output.trimmed().isEmpty() ) return;

    // Log server output at debug level to avoid spam
    RiaLogging::debug( QString( "Sumo Explorer server: %1" ).arg( output.trimmed() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::setError( const QString& error )
{
    m_lastError = error;
    RiaLogging::error( error );
    emit serverError( error );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoExplorerConnector::executeGetRequest( const QString& url )
{
    if ( !m_serverRunning )
    {
        RiaLogging::error( "Sumo Explorer server not running" );
        return {};
    }

    QNetworkRequest request( url );
    QNetworkReply*  reply = m_networkManager->get( request );

    QEventLoop loop;
    connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );

    QTimer timer;
    timer.setSingleShot( true );
    connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    timer.start( RiaSumoExplorerDefines::requestTimeoutMillis() );

    loop.exec();

    QByteArray result;
    if ( reply->error() == QNetworkReply::NoError )
    {
        result = reply->readAll();
    }
    else
    {
        RiaLogging::error( QString( "Request failed: %1" ).arg( reply->errorString() ) );
    }

    reply->deleteLater();
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoExplorerConnector::wrapAndCallNetworkRequest( std::function<void()>                        requestCallable,
                                                          const std::function<void( QNetworkReply* )>& replyHandler )
{
    QEventLoop eventLoop;

    QTimer timer;
    timer.setSingleShot( true );

    QObject::connect( &timer, &QTimer::timeout, [&] { RiaLogging::error( "Sumo Explorer request timed out." ); } );
    QObject::connect( &timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit );

    // Call the function that will execute the request
    requestCallable();

    timer.start( RiaSumoExplorerDefines::requestTimeoutMillis() );
    eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoExplorerConnector::base64ToBytes( const QString& base64 )
{
    return QByteArray::fromBase64( base64.toUtf8() );
}
