#include "RiaOsduConnector.h"
#include "RiaFileDownloader.h"
#include "RiaLogging.h"
#include "RiaOsduDefines.h"
#include "RiaOsduOAuthHttpServerReplyHandler.h"

#include <QAbstractOAuth>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOsduConnector::RiaOsduConnector( QObject*       parent,
                                    const QString& server,
                                    const QString& dataPartitionId,
                                    const QString& authority,
                                    const QString& scopes,
                                    const QString& clientId )
    : QObject( parent )
    , m_server( server )
    , m_dataPartitionId( dataPartitionId )
    , m_authority( authority )
    , m_scopes( scopes )
    , m_clientId( clientId )
{
    m_networkAccessManager = new QNetworkAccessManager( this );

    m_osdu = new QOAuth2AuthorizationCodeFlow( this );

    RiaLogging::debug( "SSL BUILD VERSION: " + QSslSocket::sslLibraryBuildVersionString() );
    RiaLogging::debug( "SSL VERSION STRING: " + QSslSocket::sslLibraryVersionString() );

    int port = 35327;

    connect( m_osdu,
             &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
             []( QUrl url )
             {
                 RiaLogging::info( "Authorize with url: " + url.toString() );
                 QUrlQuery query( url );
                 url.setQuery( query );
                 QDesktopServices::openUrl( url );
             } );

    QString authUrl = constructAuthUrl( m_authority );
    m_osdu->setAuthorizationUrl( QUrl( authUrl ) );

    QString tokenUrl = constructTokenUrl( m_authority );
    m_osdu->setAccessTokenUrl( QUrl( tokenUrl ) );

    // App key
    m_osdu->setClientIdentifier( m_clientId );
    m_osdu->setScope( m_scopes );

    auto replyHandler = new RiaOsduOAuthHttpServerReplyHandler( port, this );
    m_osdu->setReplyHandler( replyHandler );

    connect( m_osdu, SIGNAL( granted() ), this, SLOT( accessGranted() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::accessGranted()
{
    m_token = m_osdu->token();
    emit tokenReady( m_token );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestToken()
{
    if ( m_token.isEmpty() )
    {
        RiaLogging::debug( "Requesting token." );
        m_osdu->grant();
    }
    else
    {
        RiaLogging::debug( "Has token: skipping token request." );
        emit accessGranted();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOsduConnector::~RiaOsduConnector()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::clearCachedData()
{
    QMutexLocker lock( &m_mutex );
    m_fields.clear();
    m_wells.clear();
    m_wellbores.clear();
    m_wellboreTrajectories.clear();
    m_wellLogs.clear();
    m_parquetData.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestFieldsByName( const QString& token, const QString& fieldName )
{
    requestFieldsByName( m_server, m_dataPartitionId, token, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestFieldsByName( const QString& fieldName )
{
    requestFieldsByName( m_token, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestFieldsByName( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldName )
{
    std::map<QString, QString> params;
    params["kind"]  = RiaDefines::osduFieldKind();
    params["limit"] = "10000";
    params["query"] = "data.FieldName:" + fieldName;

    auto reply = makeSearchRequest( params, server, dataPartitionId, token );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseFields( reply );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellsByFieldId( const QString& fieldId )
{
    requestWellsByFieldId( m_server, m_dataPartitionId, m_token, fieldId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellsByFieldId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldId )
{
    std::map<QString, QString> params;
    params["kind"]  = RiaDefines::osduWellKind();
    params["limit"] = "10000";
    params["query"] = QString( "nested(data.GeoContexts, (FieldID:\"%1\"))" ).arg( fieldId );

    auto reply = makeSearchRequest( params, server, dataPartitionId, token );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply, fieldId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseWells( reply );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellboresByWellId( const QString& wellId )
{
    requestWellboresByWellId( m_server, m_dataPartitionId, m_token, wellId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellboresByWellId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& wellId )
{
    std::map<QString, QString> params;
    params["kind"]  = RiaDefines::osduWellboreKind();
    params["limit"] = "10000";
    params["query"] = "data.WellID: \"" + wellId + "\"";

    auto reply = makeSearchRequest( params, server, dataPartitionId, token );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply, wellId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseWellbores( reply, wellId );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduWellLog> RiaOsduConnector::requestWellLogsByWellboreIdBlocking( const QString& wellboreId )
{
    QString token = requestTokenBlocking();

    QEventLoop loop;
    connect( this, SIGNAL( wellLogsFinished( const QString& ) ), &loop, SLOT( quit() ) );
    requestWellLogsByWellboreId( m_server, m_dataPartitionId, token, wellboreId );
    loop.exec();

    return m_wellLogs[wellboreId];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellLogsByWellboreId( const QString& wellboreId )
{
    requestWellLogsByWellboreId( m_server, m_dataPartitionId, m_token, wellboreId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellLogsByWellboreId( const QString& server,
                                                    const QString& dataPartitionId,
                                                    const QString& token,
                                                    const QString& wellboreId )
{
    std::map<QString, QString> params;
    params["kind"]  = RiaDefines::osduWellLogKind();
    params["limit"] = "10000";
    params["query"] = "data.WellboreID: \"" + wellboreId + "\"";

    auto reply = makeSearchRequest( params, server, dataPartitionId, token );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply, wellboreId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseWellLogs( reply, wellboreId );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellboreTrajectoryByWellboreId( const QString& wellboreId )
{
    requestWellboreTrajectoryByWellboreId( m_server, m_dataPartitionId, m_token, wellboreId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestWellboreTrajectoryByWellboreId( const QString& server,
                                                              const QString& dataPartitionId,
                                                              const QString& token,
                                                              const QString& wellboreId )
{
    std::map<QString, QString> params;
    params["kind"]  = RiaDefines::osduWellboreTrajectoryKind();
    params["limit"] = "10000";
    params["query"] = "data.WellboreID: \"" + wellboreId + "\"";

    auto reply = makeSearchRequest( params, server, dataPartitionId, token );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply, wellboreId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseWellTrajectory( reply, wellboreId );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::constructSearchUrl( const QString& server )
{
    return server + "/api/search/v2/query";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::constructFileDownloadUrl( const QString& server, const QString& fileId )
{
    return server + "/api/file/v2/files/" + fileId + "/downloadURL";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::constructAuthUrl( const QString& authority )
{
    return authority + "/oauth2/v2.0/authorize";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::constructTokenUrl( const QString& authority )
{
    return authority + "/oauth2/v2.0/token";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::constructWellLogDownloadUrl( const QString& server, const QString& wellLogId )
{
    return server + "/api/os-wellbore-ddms/ddms/v3/welllogs/" + wellLogId + "/data";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::constructWellboreTrajectoriesDownloadUrl( const QString& server, const QString& wellboreTrajectoryId )
{
    return server + "/api/os-wellbore-ddms/ddms/v3/wellboretrajectories/" + wellboreTrajectoryId + "/data";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaOsduConnector::makeSearchRequest( const std::map<QString, QString>& parameters,
                                                    const QString&                    server,
                                                    const QString&                    dataPartitionId,
                                                    const QString&                    token )
{
    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( constructSearchUrl( server ) ) );

    addStandardHeader( m_networkRequest, token, dataPartitionId, RiaDefines::contentTypeJson() );

    QJsonObject obj;
    for ( auto [key, value] : parameters )
    {
        obj.insert( key, value );
    }

    QJsonDocument doc( obj );
    QString       strJson( doc.toJson( QJsonDocument::Compact ) );

    auto reply = m_networkAccessManager->post( m_networkRequest, strJson.toUtf8() );
    return reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::parseFields( QNetworkReply* reply )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc          = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj      = doc.object();
        QJsonArray    resultsArray = jsonObj["results"].toArray();

        {
            QMutexLocker lock( &m_mutex );
            m_fields.clear();

            for ( const QJsonValue& value : resultsArray )
            {
                QJsonObject resultObj = value.toObject();

                QString id        = resultObj["id"].toString();
                QString kind      = resultObj["kind"].toString();
                QString fieldName = resultObj["data"].toObject()["FieldName"].toString();
                m_fields.push_back( OsduField{ id, kind, fieldName } );
            }
        }

        emit fieldsFinished();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::parseWells( QNetworkReply* reply )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc          = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj      = doc.object();
        QJsonArray    resultsArray = jsonObj["results"].toArray();

        {
            QMutexLocker lock( &m_mutex );
            m_wells.clear();
            for ( const QJsonValue& value : resultsArray )
            {
                QJsonObject resultObj = value.toObject();
                QString     id        = resultObj["id"].toString();
                QString     kind      = resultObj["kind"].toString();
                QString     name      = resultObj["data"].toObject()["FacilityName"].toString();
                m_wells.push_back( OsduWell{ id, kind, name } );
            }
        }

        emit wellsFinished();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::parseWellbores( QNetworkReply* reply, const QString& wellId )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc          = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj      = doc.object();
        QJsonArray    resultsArray = jsonObj["results"].toArray();

        {
            QMutexLocker lock( &m_mutex );
            m_wellbores[wellId].clear();
            for ( const QJsonValue& value : resultsArray )
            {
                QJsonObject resultObj = value.toObject();
                QString     id        = resultObj["id"].toString();
                QString     kind      = resultObj["kind"].toString();
                QString     name      = resultObj["data"].toObject()["FacilityName"].toString();

                // Extract datum elevation. The DefaultVerticalMeasurementID is probably the datum elevation needed.
                // Default to 0.0 if nothing is found, but finding nothing is suspicious.
                double     datumElevation               = std::numeric_limits<double>::infinity();
                QString    defaultVerticalMeasurementId = resultObj["data"].toObject()["DefaultVerticalMeasurementID"].toString();
                QJsonArray verticalMeasurementsArray    = resultObj["data"].toObject()["VerticalMeasurements"].toArray();
                for ( const QJsonValue& vma : verticalMeasurementsArray )
                {
                    QString verticalMeasurementId = vma["VerticalMeasurementID"].toString();
                    if ( verticalMeasurementId == defaultVerticalMeasurementId )
                    {
                        double verticalMeasurement = vma["VerticalMeasurement"].toDouble( 0.0 );
                        datumElevation             = verticalMeasurement;
                    }
                }

                if ( std::isinf( datumElevation ) )
                {
                    RiaLogging::warning( QString( "Missing datum elevation for well bore '%1'. Id: %2" ).arg( name ).arg( id ) );
                    datumElevation = 0.0;
                }

                m_wellbores[wellId].push_back( OsduWellbore{ id, kind, name, wellId, datumElevation } );
            }
        }

        emit wellboresFinished( wellId );
    }
    else
    {
        RiaLogging::error( "Failed to download well with id " + wellId + ": " + reply->errorString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::parseWellTrajectory( QNetworkReply* reply, const QString& wellboreId )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc          = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj      = doc.object();
        QJsonArray    resultsArray = jsonObj["results"].toArray();

        {
            QMutexLocker lock( &m_mutex );
            m_wellboreTrajectories[wellboreId].clear();
            for ( const QJsonValue& value : resultsArray )
            {
                QJsonObject resultObj = value.toObject();
                QString     id        = resultObj["id"].toString();
                QString     kind      = resultObj["kind"].toString();

                m_wellboreTrajectories[wellboreId].push_back( OsduWellboreTrajectory{ id, kind, wellboreId } );
            }
        }

        emit wellboreTrajectoryFinished( wellboreId, resultsArray.size(), "" );
    }
    else
    {
        emit wellboreTrajectoryFinished( wellboreId, 0, "Failed to download: " + reply->errorString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::parseWellLogs( QNetworkReply* reply, const QString& wellboreId )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc          = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj      = doc.object();
        QJsonArray    resultsArray = jsonObj["results"].toArray();

        {
            QMutexLocker lock( &m_mutex );
            m_wellLogs[wellboreId].clear();
            for ( const QJsonValue& value : resultsArray )
            {
                QJsonObject resultObj = value.toObject();
                QString     id        = resultObj["id"].toString();
                QString     kind      = resultObj["kind"].toString();

                m_wellLogs[wellboreId].push_back( OsduWellLog{ id, kind, wellboreId } );
            }
        }

        emit wellLogsFinished( wellboreId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::addStandardHeader( QNetworkRequest& networkRequest,
                                          const QString&   token,
                                          const QString&   dataPartitionId,
                                          const QString&   contentType )
{
    networkRequest.setHeader( QNetworkRequest::ContentTypeHeader, contentType );
    networkRequest.setRawHeader( "Authorization", "Bearer " + token.toUtf8() );
    networkRequest.setRawHeader( QByteArray( "Data-Partition-Id" ), dataPartitionId.toUtf8() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QNetworkReply*
    RiaOsduConnector::makeDownloadRequest( const QString& url, const QString& dataPartitionId, const QString& token, const QString& contentType )
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( networkRequest, token, dataPartitionId, contentType );

    auto reply = m_networkAccessManager->get( networkRequest );
    return reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::server() const
{
    return m_server;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::dataPartition() const
{
    return m_dataPartitionId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduField> RiaOsduConnector::fields() const
{
    QMutexLocker lock( &m_mutex );
    return m_fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduWell> RiaOsduConnector::wells() const
{
    QMutexLocker lock( &m_mutex );
    return m_wells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduWellbore> RiaOsduConnector::wellbores( const QString& wellId ) const
{
    QMutexLocker lock( &m_mutex );

    auto it = m_wellbores.find( wellId );
    if ( it != m_wellbores.end() )
        return it->second;
    else
        return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::wellIdForWellboreId( const QString& wellboreId ) const
{
    auto findWellIdForWellboreId = []( const std::vector<OsduWellbore>& wellbores, const QString& wellboreId )
    {
        auto it = std::find_if( wellbores.begin(), wellbores.end(), [wellboreId]( const OsduWellbore& w ) { return w.id == wellboreId; } );
        if ( it != wellbores.end() )
            return it->wellId;
        else
            return QString();
    };

    QMutexLocker lock( &m_mutex );
    for ( auto [wellId, wellbores] : m_wellbores )
    {
        if ( auto res = findWellIdForWellboreId( wellbores, wellboreId ); !res.isEmpty() )
        {
            return wellId;
        }
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduWellboreTrajectory> RiaOsduConnector::wellboreTrajectories( const QString& wellboreId ) const
{
    QMutexLocker lock( &m_mutex );

    auto it = m_wellboreTrajectories.find( wellboreId );
    if ( it != m_wellboreTrajectories.end() )
        return it->second;
    else
        return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QByteArray, QString> RiaOsduConnector::requestWellboreTrajectoryParquetDataById( const QString& wellboreTrajectoryId )
{
    QString url = constructWellboreTrajectoriesDownloadUrl( m_server, wellboreTrajectoryId );
    RiaLogging::debug( "Wellbore trajectory URL: " + url );
    return requestParquetDataByUrl( url );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QByteArray, QString> RiaOsduConnector::requestWellLogParquetDataById( const QString& wellLogId )
{
    QString url = constructWellLogDownloadUrl( m_server, wellLogId );
    RiaLogging::debug( "Well log URL: " + url );
    return requestParquetDataByUrl( url );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QByteArray, QString> RiaOsduConnector::requestParquetDataByUrl( const QString& url )
{
    QString token = requestTokenBlocking();

    QEventLoop loop;
    connect( this,
             SIGNAL( parquetDownloadFinished( const QByteArray&, const QString& ) ),
             this,
             SLOT( parquetDownloadComplete( const QByteArray&, const QString& ) ) );
    connect( this, SIGNAL( parquetDownloadFinished( const QByteArray&, const QString& ) ), &loop, SLOT( quit() ) );

    requestParquetData( url, m_dataPartitionId, token );
    loop.exec();

    return { m_parquetData, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestParquetData( const QString& url, const QString& dataPartitionId, const QString& token )
{
    RiaLogging::info( "Requesting download of parquet from: " + url );

    auto reply = makeDownloadRequest( url, dataPartitionId, token, RiaDefines::contentTypeParquet() );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply, url]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     QByteArray contents = reply->readAll();
                     RiaLogging::info( QString( "Download succeeded: %1 bytes." ).arg( contents.length() ) );
                     emit parquetDownloadFinished( contents, "" );
                 }
                 else
                 {
                     QString errorMessage = "Download failed: " + url + " failed." + reply->errorString();
                     RiaLogging::error( errorMessage );
                     emit parquetDownloadFinished( QByteArray(), errorMessage );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::parquetDownloadComplete( const QByteArray& contents, const QString& url )
{
    m_parquetData = contents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::requestTokenBlocking()
{
    if ( m_token.isEmpty() )
    {
        QEventLoop loop;
        connect( this, SIGNAL( tokenReady( const QString& ) ), &loop, SLOT( quit() ) );
        requestToken();
        loop.exec();
    }

    return m_token;
}
