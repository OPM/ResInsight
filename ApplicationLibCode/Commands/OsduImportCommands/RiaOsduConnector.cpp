// #include "Downloader.h"
#include "RiaOsduConnector.h"
#include "RiaFileDownloader.h"
#include "RiaOsduOAuthHttpServerReplyHandler.h"

#include <QAbstractOAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOAuth2AuthorizationCodeFlow>
#include <QObject>

#include <QDesktopServices>
#include <QOAuthHttpServerReplyHandler>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

static const QString FIELD_KIND               = "osdu:wks:master-data--Field:1.0.0";
static const QString WELL_KIND                = "osdu:wks:master-data--Well:1.1.0";
static const QString WELLBORE_KIND            = "osdu:wks:master-data--Wellbore:1.1.0";
static const QString WELLBORE_TRAJECTORY_KIND = "osdu:wks:work-product-component--WellboreTrajectory:1.1.0";

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

    printf( "REQUEST TOKEN\n" );
    int port = 35327;

    connect( m_osdu,
             &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
             []( QUrl url )
             {
                 printf( "AUTHORIZE WITH URL. Thread: %p\n", QThread::currentThread() );
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

    connect( m_osdu, SIGNAL( granted() ), this, SLOT( granted() ), Qt::QueuedConnection );
}

void RiaOsduConnector::granted()
{
    QString token = m_osdu->token();
    emit    tokenReady( token );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestToken()
{
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
void RiaOsduConnector::requestFieldsByName( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldName )
{
    std::map<QString, QString> params;
    params["kind"]  = FIELD_KIND;
    params["limit"] = "10000";
    params["query"] = "data.FieldName:IVAR*";
    makeRequest( params, server, dataPartitionId, token );

    connect( m_networkAccessManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( parseWells( QNetworkReply* ) ) );
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
    makeRequest( params, server, dataPartitionId, token );

    connect( m_networkAccessManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( parseWells( QNetworkReply* ) ) );
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
    makeRequest( params, server, dataPartitionId, token );

    connect( m_networkAccessManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( parseWells( QNetworkReply* ) ) );
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
    makeRequest( params, server, dataPartitionId, token );

    connect( m_networkAccessManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( parseWellTrajectory( QNetworkReply* ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::requestFileDownloadByFileId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fileId )
{
    // if ( fileId.endsWith( ":" ) ) fileId.truncate( fileId.lastIndexOf( QChar( ':' ) ) );
    makeDownloadRequest( server, dataPartitionId, fileId, token );

    connect( m_networkAccessManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( saveFile( QNetworkReply* ) ) );
}

//--------------------------------------------------------------------------------------------------
///
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
void RiaOsduConnector::parseWells( QNetworkReply* reply )
{
    qDebug() << "REQUEST FINISHED. Error? " << ( reply->error() != QNetworkReply::NoError );

    QByteArray result = reply->readAll();

    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson( result );
    // Extract the JSON object from the QJsonDocument
    QJsonObject jsonObj = doc.object();

    // Access "results" array from the JSON object
    QJsonArray resultsArray = jsonObj["results"].toArray();

    // Iterate through each element in the "results" array
    qDebug() << "Found " << resultsArray.size() << " items.";
    foreach ( const QJsonValue& value, resultsArray )
    {
        QJsonObject resultObj = value.toObject();

        // Accessing specific fields from the result object
        QString id   = resultObj["id"].toString();
        QString kind = resultObj["kind"].toString();

        qDebug() << "Id:" << id << " kind: " << kind;

        printf( "%s\n", id.toStdString().c_str() );
    }
    emit finished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::parseWellTrajectory( QNetworkReply* reply )
{
    qDebug() << "REQUEST FINISHED. Error? " << ( reply->error() != QNetworkReply::NoError );

    QByteArray result = reply->readAll();

    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson( result );
    // Extract the JSON object from the QJsonDocument
    QJsonObject jsonObj = doc.object();

    QString formattedJsonString = doc.toJson( QJsonDocument::Indented );
    // qDebug() << formattedJsonString;

    // Access "results" array from the JSON object
    QJsonArray resultsArray = jsonObj["results"].toArray();

    // Iterate through each element in the "results" array
    //  qDebug() << "Found " << resultsArray.size() << " items.";
    foreach ( const QJsonValue& value, resultsArray )
    {
        QJsonObject resultObj = value.toObject();

        // Accessing specific fields from the result object
        QJsonObject dataObj = resultObj["data"].toObject();

        //    QJsonArray id = resultObj["data"].toArray();
        QJsonArray dataSets = dataObj["Datasets"].toArray();
        foreach ( const QJsonValue& dataSet, dataSets )
        {
            printf( "%s\n", dataSet.toString().toStdString().c_str() );
        }
    }

    emit finished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOsduConnector::saveFile( QNetworkReply* reply )
{
    qDebug() << "REQUEST FINISHED. Error? " << ( reply->error() != QNetworkReply::NoError );
    // if (reply->error() == QNetworkReply::NoError) {

    qDebug() << reply->errorString();
    QByteArray result = reply->readAll();

    reply->deleteLater();

    QEventLoop loop;

    QJsonDocument doc = QJsonDocument::fromJson( result );
    // Extract the JSON object from the QJsonDocument
    QJsonObject jsonObj = doc.object();

    QString signedUrl = jsonObj["SignedUrl"].toString();

    RiaFileDownloader downloader;
    QUrl              url( signedUrl );
    QString           filePath = "/tmp/" + generateRandomString( 30 ) + ".txt";
    downloader.downloadFile( url, filePath );

    QString formattedJsonString = doc.toJson( QJsonDocument::Indented );
    qDebug() << formattedJsonString;

    printf( "%s => %s\n", signedUrl.toStdString().c_str(), filePath.toStdString().c_str() );

    connect( &downloader, SIGNAL( done() ), this, SIGNAL( finished() ) );

    loop.exec();
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
