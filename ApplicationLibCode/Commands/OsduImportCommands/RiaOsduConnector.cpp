#include "RiaOsduConnector.h"
#include "RiaFileDownloader.h"
#include "RiaLogging.h"
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
    RiaLogging::debug( "Requesting token." );
    m_osdu->grant();
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
    params["kind"]  = FIELD_KIND;
    params["limit"] = "10000";
    params["query"] = "data.FieldName:" + fieldName;

    auto reply = makeRequest( params, server, dataPartitionId, token );
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
    params["kind"]  = WELL_KIND;
    params["limit"] = "10000";
    params["query"] = QString( "nested(data.GeoContexts, (FieldID:\"%1\"))" ).arg( fieldId );

    auto reply = makeRequest( params, server, dataPartitionId, token );
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
    params["kind"]  = WELLBORE_KIND;
    params["limit"] = "10000";
    params["query"] = "data.WellID: \"" + wellId + "\"";

    auto reply = makeRequest( params, server, dataPartitionId, token );
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
void RiaOsduConnector::requestWellboreTrajectoryByWellboreId( const QString& wellboreId )
{
    requestWellboreTrajectoryByWellboreId( m_server, m_dataPartitionId, m_token, wellboreId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestFileDownloadByFileId( const QString& fileId )
{
    requestFileDownloadByFileId( m_server, m_dataPartitionId, m_token, fileId );
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
    params["kind"]  = WELLBORE_TRAJECTORY_KIND;
    params["limit"] = "10000";
    params["query"] = "data.WellboreID: \"" + wellboreId + "\"";

    auto reply = makeRequest( params, server, dataPartitionId, token );
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
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestFileDownloadByFileId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fileId )
{
    RiaLogging::info( "Requesting download of file id: " + fileId );
    auto reply = makeDownloadRequest( server, dataPartitionId, fileId, token );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply, fileId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     saveFile( reply, fileId );
                 }
                 else
                 {
                     RiaLogging::error( "File request for id " + fileId + " failed." + reply->errorString() );
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
QString RiaOsduConnector::constructDownloadUrl( const QString& server, const QString& fileId )
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
QNetworkReply* RiaOsduConnector::makeRequest( const std::map<QString, QString>& parameters,
                                              const QString&                    server,
                                              const QString&                    dataPartitionId,
                                              const QString&                    token )
{
    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( constructSearchUrl( server ) ) );

    addStandardHeader( m_networkRequest, token, dataPartitionId );

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

        m_fields.clear();

        foreach ( const QJsonValue& value, resultsArray )
        {
            QJsonObject resultObj = value.toObject();

            QString id        = resultObj["id"].toString();
            QString kind      = resultObj["kind"].toString();
            QString fieldName = resultObj["data"].toObject()["FieldName"].toString();
            m_fields.push_back( OsduField{ id, kind, fieldName } );
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

        RiaLogging::info( QString( "Found %1 wells." ).arg( +resultsArray.size() ) );

        m_wells.clear();
        foreach ( const QJsonValue& value, resultsArray )
        {
            QJsonObject resultObj = value.toObject();
            QString     id        = resultObj["id"].toString();
            QString     kind      = resultObj["kind"].toString();
            QString     name      = resultObj["data"].toObject()["FacilityName"].toString();
            m_wells.push_back( OsduWell{ id, kind, name } );
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

        RiaLogging::info( QString( "Found %1 wellbores." ).arg( resultsArray.size() ) );

        m_wellbores[wellId].clear();
        foreach ( const QJsonValue& value, resultsArray )
        {
            QJsonObject resultObj = value.toObject();
            QString     id        = resultObj["id"].toString();
            QString     kind      = resultObj["kind"].toString();
            QString     name      = resultObj["data"].toObject()["FacilityName"].toString();
            m_wellbores[wellId].push_back( OsduWellbore{ id, kind, name, wellId } );
        }

        emit wellboresFinished( wellId );
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
        m_wellboreTrajectories.clear();
        foreach ( const QJsonValue& value, resultsArray )
        {
            QJsonObject resultObj = value.toObject();
            QJsonObject dataObj   = resultObj["data"].toObject();
            QJsonArray  dataSets  = dataObj["Datasets"].toArray();
            if ( dataSets.size() == 1 )
            {
                QString id          = resultObj["id"].toString();
                QString kind        = resultObj["kind"].toString();
                QString dataSetId   = dataSets[0].toString();
                QString description = dataObj["Description"].toString();

                m_wellboreTrajectories[wellboreId].push_back( OsduWellboreTrajectory{ id, kind, description, dataSetId, wellboreId } );
            }
            else if ( dataSets.size() > 1 )
            {
                RiaLogging::error( "Encountered dataset with more than on file: currently not supported." );
            }
        }

        emit wellboreTrajectoryFinished( wellboreId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::saveFile( QNetworkReply* reply, const QString& fileId )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QEventLoop loop;

        QJsonDocument doc     = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj = doc.object();

        QString signedUrl = jsonObj["SignedUrl"].toString();

        RiaFileDownloader* downloader = new RiaFileDownloader;
        QUrl               url( signedUrl );
        QString            filePath = "/tmp/" + generateRandomString( 30 ) + ".txt";

        QString formattedJsonString = doc.toJson( QJsonDocument::Indented );

        RiaLogging::info( QString( "File download: %1 => %2" ).arg( signedUrl ).arg( filePath ) );
        connect( this, SIGNAL( fileDownloadFinished( const QString&, const QString& ) ), &loop, SLOT( quit() ) );
        connect( downloader,
                 &RiaFileDownloader::done,
                 [this, fileId, filePath]()
                 {
                     RiaLogging::info( QString( "Download complete %1 => %2" ).arg( fileId ).arg( filePath ) );
                     emit( fileDownloadFinished( fileId, filePath ) );
                 } );
        RiaLogging::info( "Starting download" );
        downloader->downloadFile( url, filePath );

        downloader->deleteLater();
        loop.exec();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& dataPartitionId )
{
    networkRequest.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
    networkRequest.setRawHeader( "Authorization", "Bearer " + token.toUtf8() );
    networkRequest.setRawHeader( QByteArray( "Data-Partition-Id" ), dataPartitionId.toUtf8() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QNetworkReply*
    RiaOsduConnector::makeDownloadRequest( const QString& server, const QString& dataPartitionId, const QString& id, const QString& token )
{
    QNetworkRequest m_networkRequest;

    QString url = constructDownloadUrl( server, id );

    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token, dataPartitionId );

    auto reply = m_networkAccessManager->get( m_networkRequest );
    return reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduConnector::generateRandomString( int randomStringLength )
{
    const QString possibleCharacters( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" );
    QString       randomString;
    for ( int i = 0; i < randomStringLength; ++i )
    {
        quint32 value    = QRandomGenerator::global()->generate();
        int     index    = value % possibleCharacters.length();
        QChar   nextChar = possibleCharacters.at( index );
        randomString.append( nextChar );
    }
    return randomString;
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
    return m_fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduWell> RiaOsduConnector::wells() const
{
    return m_wells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduWellbore> RiaOsduConnector::wellbores( const QString& wellId ) const
{
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
    auto it = m_wellboreTrajectories.find( wellboreId );
    if ( it != m_wellboreTrajectories.end() )
        return it->second;
    else
        return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::fileDownloadComplete( const QString& fileId, const QString& filePath )
{
    m_filePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RiaOsduConnector::requestFileContentsById( const QString& fileId )
{
    if ( m_token.isEmpty() )
    {
        // TODO: improve this..
        QEventLoop loop;
        connect( this, SIGNAL( tokenReady( const QString& ) ), &loop, SLOT( quit() ) );
        requestToken();
        loop.exec();
    }

    QEventLoop loop2;
    connect( this,
             SIGNAL( fileDownloadFinished( const QString&, const QString& ) ),
             this,
             SLOT( fileDownloadComplete( const QString&, const QString& ) ) );
    connect( this, SIGNAL( fileDownloadFinished( const QString&, const QString& ) ), &loop2, SLOT( quit() ) );
    requestFileDownloadByFileId( m_server, m_dataPartitionId, m_token, fileId );
    loop2.exec();

    QFile dataFile( m_filePath );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        return { "", "Unable to open file: " + m_filePath };
    }

    QTextStream stream( &dataFile );
    auto        fileContent = stream.readAll();

    return { fileContent, "" };
}
