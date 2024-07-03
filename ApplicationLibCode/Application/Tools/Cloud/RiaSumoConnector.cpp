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

#include "RiaSumoConnector.h"

#include "RiaFileDownloader.h"
#include "RiaLogging.h"
#include "RiaOsduDefines.h"

#include "OsduImportCommands/RiaOsduOAuthHttpServerReplyHandler.h"

#include <QAbstractOAuth>
#include <QDesktopServices>
#include <QEventLoop>
#include <QJsonArray>
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
#include <QtCore>

#pragma optimize( "", off )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoConnector::RiaSumoConnector( QObject* parent, const QString& server, const QString& authority, const QString& scopes, const QString& clientId )
    : QObject( parent )
    , m_server( server )
    , m_authority( authority )
    , m_scopes( scopes )
    , m_clientId( clientId )
{
    m_authCodeFlow         = new QOAuth2AuthorizationCodeFlow( this );
    m_networkAccessManager = new QNetworkAccessManager( this );
    m_authCodeFlow->setNetworkAccessManager( m_networkAccessManager );

    RiaLogging::debug( "SSL BUILD VERSION: " + QSslSocket::sslLibraryBuildVersionString() );
    RiaLogging::debug( "SSL VERSION STRING: " + QSslSocket::sslLibraryVersionString() );

    // NB: Make sure the port is not in use by another application
    const unsigned int port = 53527;

    connect( m_authCodeFlow,
             &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
             []( QUrl url )
             {
                 RiaLogging::info( "Authorize with url: " + url.toString() );
                 QUrlQuery query( url );
                 url.setQuery( query );
                 QDesktopServices::openUrl( url );
             } );

    QString authUrl = constructAuthUrl( m_authority );
    m_authCodeFlow->setAuthorizationUrl( QUrl( authUrl ) );

    QString tokenUrl = constructTokenUrl( m_authority );
    m_authCodeFlow->setAccessTokenUrl( QUrl( tokenUrl ) );

    // App key
    m_authCodeFlow->setClientIdentifier( m_clientId );
    m_authCodeFlow->setScope( m_scopes );

    auto replyHandler = new RiaOsduOAuthHttpServerReplyHandler( port, this );
    m_authCodeFlow->setReplyHandler( replyHandler );

    connect( m_authCodeFlow, SIGNAL( granted() ), this, SLOT( accessGranted() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::accessGranted()
{
    m_token = m_authCodeFlow->token();

    QString tokenDataJson = tokenDataAsJson( m_authCodeFlow );
    writeTokenData( m_tokenDataFilePath, tokenDataJson );

    emit tokenReady( m_token );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestFailed( const QAbstractOAuth::Error error )
{
    RiaLogging::error( "Request failed: " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parquetDownloadComplete( const QString& blobId, const QByteArray& contents, const QString& url )
{
    SumoRedirect obj;
    obj.objectId = blobId;
    obj.contents = contents;
    obj.url      = url;

    m_redirectInfo.push_back( obj );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::tokenDataAsJson( QOAuth2AuthorizationCodeFlow* authCodeFlow )
{
    QJsonObject obj;
    obj.insert( "token", authCodeFlow->token() );
    obj.insert( "refreshToken", authCodeFlow->refreshToken() );
    obj.insert( "scope", authCodeFlow->scope() );
    obj.insert( "clientIdentifier", authCodeFlow->clientIdentifier() );
    obj.insert( "authorizationUrl", authCodeFlow->authorizationUrl().toString() );
    obj.insert( "accessTokenUrl", authCodeFlow->accessTokenUrl().toString() );

    QJsonDocument doc( obj );
    return doc.toJson( QJsonDocument::Indented );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::initializeTokenDataFromJson( QOAuth2AuthorizationCodeFlow* authCodeFlow, const QString& tokenDataJson )
{
    QJsonDocument doc = QJsonDocument::fromJson( tokenDataJson.toUtf8() );
    QJsonObject   obj = doc.object();

    authCodeFlow->setToken( obj["token"].toString() );
    authCodeFlow->setRefreshToken( obj["refreshToken"].toString() );
    authCodeFlow->setScope( obj["scope"].toString() );
    authCodeFlow->setClientIdentifier( obj["clientIdentifier"].toString() );
    authCodeFlow->setAuthorizationUrl( QUrl( obj["authorizationUrl"].toString() ) );
    authCodeFlow->setAccessTokenUrl( QUrl( obj["accessTokenUrl"].toString() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::writeTokenData( const QString& filePath, const QString& tokenDataJson )
{
    QFile file( filePath );
    if ( file.open( QIODevice::WriteOnly ) )
    {
        QTextStream stream( &file );
        stream << tokenDataJson;
        file.close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::readTokenData( const QString& filePath )
{
    QFile file( filePath );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QTextStream stream( &file );
        QString     result = stream.readAll();
        file.close();
        return result;
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestToken()
{
    RiaLogging::debug( "Requesting token." );
    m_authCodeFlow->grant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoConnector::~RiaSumoConnector()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::token() const
{
    return m_token;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::importTokenFromFile()
{
    auto tokenDataJson = readTokenData( m_tokenDataFilePath );
    if ( !tokenDataJson.isEmpty() )
    {
        initializeTokenDataFromJson( m_authCodeFlow, tokenDataJson );
        m_token = m_authCodeFlow->token();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::setTokenDataFilePath( const QString& filePath )
{
    m_tokenDataFilePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestCasesForField( const QString& fieldName )
{
    m_cases.clear();

    requestTokenBlocking();

    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( constructSearchUrl( m_server ) ) );

    addStandardHeader( m_networkRequest, m_token, RiaDefines::contentTypeJson() );

    QString payloadTemplate = R"(
{
    "query": {
        "bool": {
            "filter": [
                        {"term":{"class.keyword":"case"}},
                        {"term":{"access.asset.name.keyword":"%1"}}
            ]
        }
    },
    "sort": [
            {"tracklog.datetime":{"order":"desc"}}
    ],
    "track_total_hits":true,
    "size":100,
    "from":0
}
)";

    QString payload = payloadTemplate.arg( fieldName );
    auto    reply   = m_networkAccessManager->post( m_networkRequest, payload.toUtf8() );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseCases( reply );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestCasesForFieldBlocking( const QString& fieldName )
{
    QEventLoop loop;
    connect( this, SIGNAL( casesFinished() ), &loop, SLOT( quit() ) );
    QTimer timer;

    requestCasesForField( fieldName );

    // Start the timer
    timer.setSingleShot( true );
    int timeout = 10000;
    timer.start( timeout );
    loop.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestAssets()
{
    requestTokenBlocking();

    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( m_server + "/api/v1/userpermissions" ) );

    addStandardHeader( m_networkRequest, m_token, RiaDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseAssets( reply );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestAssetsBlocking()
{
    QEventLoop loop;
    connect( this, SIGNAL( assetsFinished() ), &loop, SLOT( quit() ) );
    QTimer timer;

    requestAssets();

    // Start the timer
    timer.setSingleShot( true );
    int timeout = 10000;
    timer.start( timeout );
    loop.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestEnsembleByCasesId( const SumoCaseId& caseId )
{
    QString payloadTemplate = R"(

{
    "query": {
        "bool": {
            "filter": [
                        {"term":{"_sumo.parent_object.keyword":"%1"}}
            ]
        }
    },
    "aggs": {
        "aggs_columns": {
            "terms": { "field": "fmu.iteration.name.keyword", "size": 5 }
        }
    },
    "track_total_hits":true,
    "size":20,
    "from":0,
     "_source": false
}

)";

    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( m_server + "/api/v1/search" ) );

    addStandardHeader( m_networkRequest, m_token, RiaDefines::contentTypeJson() );

    auto payload = payloadTemplate.arg( caseId.get() );
    auto reply   = m_networkAccessManager->post( m_networkRequest, payload.toUtf8() );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, caseId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseEnsembleNames( reply, caseId );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestEnsembleByCasesIdBlocking( const SumoCaseId& caseId )
{
    QEventLoop loop;
    connect( this, SIGNAL( ensembleNamesFinished() ), &loop, SLOT( quit() ) );
    QTimer timer;

    requestEnsembleByCasesId( caseId );

    // Start the timer
    timer.setSingleShot( true );
    int timeout = 10000;
    timer.start( timeout );
    loop.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestVectorNamesForEnsemble( const SumoCaseId& caseId, const QString& ensembleName )
{
    QString payloadTemplate = R"(
{
    "track_total_hits": true,
    "query":  {   "bool": {
            "must": [
                {"term": {"class": "table"}},
                {"term": {"_sumo.parent_object.keyword": "%1"}},
                {"term": {"fmu.iteration.name.keyword": "%2"}},
                {"term": {"fmu.context.stage.keyword": "iteration"}},
                {"term": {"fmu.aggregation.operation.keyword": "collection"}},
                {"term": {"data.tagname.keyword": "summary"}},
                {"term": {"data.content.keyword": "timeseries"}}
            ]}
        },
    "aggs": {
        "smry_tables": {
            "terms": {
                "field": "data.name.keyword"
            },
            "aggs": {
                "smry_columns": {
                    "terms": {
                        "field": "data.spec.columns.keyword",
                        "size": 65535
                    }
                }
            }
        }
    },
    "_source": false,
    "size": 0
})";

    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( m_server + "/api/v1/search" ) );

    addStandardHeader( m_networkRequest, m_token, RiaDefines::contentTypeJson() );

    auto payload = payloadTemplate.arg( caseId.get() ).arg( ensembleName );
    auto reply   = m_networkAccessManager->post( m_networkRequest, payload.toUtf8() );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseVectorNames( reply, caseId, ensembleName );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestVectorNamesForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName )
{
    QEventLoop loop;
    connect( this, SIGNAL( vectorNamesFinished() ), &loop, SLOT( quit() ) );
    QTimer timer;

    requestVectorNamesForEnsemble( caseId, ensembleName );

    // Start the timer
    timer.setSingleShot( true );
    int timeout = 10000;
    timer.start( timeout );
    loop.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestRealizationIdsForEnsemble( const SumoCaseId& caseId, const QString& ensembleName )
{
    QString payloadTemplate = R"(
{
    "track_total_hits": true,
    "query":  {
           "bool": {
            "must": [
                {"term": {"class": "table"}},
                {"term": {"_sumo.parent_object.keyword": "%1"}},
                {"term": {"fmu.iteration.name.keyword": "%2"}},
                {"term": {"fmu.context.stage.keyword": "iteration"}},
                {"term": {"fmu.aggregation.operation.keyword": "collection"}},
                {"term": {"data.tagname.keyword": "summary"}},
                {"term": {"data.content.keyword": "timeseries"}}
            ]}
    },
    "aggs": {
        "realization-ids": {
            "terms": {
            "field": "fmu.aggregation.realization_ids",
            "size":1000
            }
        }
    },
    "_source": false,
    "size":0
}
)";
    m_realizationIds.clear();

    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( m_server + "/api/v1/search" ) );

    addStandardHeader( m_networkRequest, m_token, RiaDefines::contentTypeJson() );

    auto payload = payloadTemplate.arg( caseId.get() ).arg( ensembleName );
    auto reply   = m_networkAccessManager->post( m_networkRequest, payload.toUtf8() );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseRealizationNumbers( reply, caseId, ensembleName );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestRealizationIdsForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName )
{
    QEventLoop loop;
    connect( this, SIGNAL( realizationIdsFinished() ), &loop, SLOT( quit() ) );
    QTimer timer;

    requestRealizationIdsForEnsemble( caseId, ensembleName );

    // Start the timer
    timer.setSingleShot( true );
    int timeout = 10000;
    timer.start( timeout );
    loop.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobIdForEnsemble( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    QString payloadTemplate = R"(
{
    "track_total_hits": true,
    "query":  {   "bool": {
            "must": [
                {"term": {"class": "table"}},
                {"term": {"_sumo.parent_object.keyword": "%1"}},
                {"term": {"fmu.iteration.name.keyword": "%2"}},
                {"term": {"fmu.context.stage.keyword": "iteration"}},
                {"term": {"fmu.aggregation.operation.keyword": "collection"}},
                {"term": {"data.tagname.keyword": "summary"}},
                {"term": {"data.spec.columns.keyword": "%3"}}
            ]}
        },
         "fields": [
            "data.name",
            "_sumo.blob_name"
        ],
    "_source": true,
    "size": 1
}
)";

    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( m_server + "/api/v1/search" ) );

    addStandardHeader( m_networkRequest, m_token, RiaDefines::contentTypeJson() );

    auto payload = payloadTemplate.arg( caseId.get() ).arg( ensembleName ).arg( vectorName );
    auto reply   = m_networkAccessManager->post( m_networkRequest, payload.toUtf8() );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId, vectorName]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseBlobIds( reply, caseId, ensembleName, vectorName );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobIdForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    QEventLoop loop;
    connect( this, SIGNAL( blobIdFinished() ), &loop, SLOT( quit() ) );
    QTimer timer;

    requestBlobIdForEnsemble( caseId, ensembleName, vectorName );

    // Start the timer
    timer.setSingleShot( true );
    int timeout = 10000;
    timer.start( timeout );
    loop.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobDownload( const QString& blobId )
{
    QString url = constructDownloadUrl( m_server, blobId );

    QNetworkRequest networkRequest;
    networkRequest.setUrl( url );

    // Other redirection policies are NoLessSafeRedirectPolicy, SameOriginRedirectPolicy, UserVerifiedRedirectPolicy. They were tested, but
    // did not work. Use ManualRedirectPolicy instead, and inspect the reply for the redirection target.
    networkRequest.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );

    addStandardHeader( networkRequest, m_token, RiaDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, blobId, url]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     auto contents = reply->readAll();

                     QVariant redirectUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
                     if ( redirectUrl.isValid() )
                     {
                         requestBlobByRedirectUri( blobId, redirectUrl.toString() );
                     }
                     else
                     {
                         QString errorMessage = "Not able to parse and interpret valid redirect Url";
                         RiaLogging::error( errorMessage );
                     }
                 }
                 else
                 {
                     QString errorMessage = "Download failed: " + url + " failed." + reply->errorString();
                     RiaLogging::error( errorMessage );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobByRedirectUri( const QString& blobId, const QString& redirectUri )
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl( redirectUri );

    auto reply = m_networkAccessManager->get( networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, blobId, redirectUri]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     auto contents = reply->readAll();

                     QString msg = "Received data from : " + redirectUri;
                     RiaLogging::info( msg );

                     parquetDownloadComplete( blobId, contents, redirectUri );

                     emit parquetDownloadFinished( contents, redirectUri );
                 }
                 else
                 {
                     QString errorMessage = "Download failed: " + redirectUri + " failed." + reply->errorString();
                     RiaLogging::error( errorMessage );

                     emit parquetDownloadFinished( {}, redirectUri );
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::requestParquetDataBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    requestBlobIdForEnsembleBlocking( caseId, ensembleName, vectorName );

    if ( m_blobUrl.empty() ) return {};

    auto blobId = m_blobUrl.back();

    QEventLoop loop;
    connect( this, SIGNAL( parquetDownloadFinished( const QByteArray&, const QString& ) ), &loop, SLOT( quit() ) );
    QTimer timer;

    requestBlobDownload( blobId );

    // Start the timer
    timer.setSingleShot( true );
    int timeout = 10000;
    timer.start( timeout );
    loop.exec();

    for ( const auto& blobData : m_redirectInfo )
    {
        if ( blobData.objectId == blobId )
        {
            return blobData.contents;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::constructSearchUrl( const QString& server )
{
    return server + "/api/v1/search";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::constructDownloadUrl( const QString& server, const QString& blobId )
{
    return server + "/api/v1/objects('" + blobId + "')/blob";
    // https: // main-sumo-prod.radix.equinor.com/api/v1/objects('76d6d11f-2278-3fe2-f12f-77142ad163c6')/blob
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::constructAuthUrl( const QString& authority )
{
    return authority + "/oauth2/v2.0/authorize";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::constructTokenUrl( const QString& authority )
{
    return authority + "/oauth2/v2.0/token";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoConnector::makeRequest( const std::map<QString, QString>& parameters, const QString& server, const QString& token )
{
    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( constructSearchUrl( server ) ) );

    addStandardHeader( m_networkRequest, token, RiaDefines::contentTypeJson() );

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
void RiaSumoConnector::parseAssets( QNetworkReply* reply )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc     = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj = doc.object();

        m_assets.clear();

        for ( auto key : jsonObj.keys() )
        {
            QString id;
            QString kind;
            QString fieldName = key;
            m_assets.push_back( SumoAsset{ SumoAssetId( id ), kind, fieldName } );
        }

        for ( auto a : m_assets )
        {
            RiaLogging::info( QString( "Asset: %1" ).arg( a.name ) );
        }
    }
    emit assetsFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parseEnsembleNames( QNetworkReply* reply, const SumoCaseId& caseId )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        m_ensembleNames.clear();

        QJsonDocument doc     = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj = doc.object();
        auto          keys_1  = jsonObj.keys();

        auto aggregationsObject = jsonObj["aggregations"].toObject();

        QJsonObject aggregationColumnsObject = aggregationsObject["aggs_columns"].toObject();
        auto        keys_2                   = aggregationColumnsObject.keys();

        QJsonArray bucketsArray = aggregationColumnsObject["buckets"].toArray();
        foreach ( const QJsonValue& bucket, bucketsArray )
        {
            QJsonObject bucketObj = bucket.toObject();
            auto        keys_3    = bucketObj.keys();

            auto ensembleName = bucketObj["key"].toString();
            m_ensembleNames.push_back( { caseId, ensembleName } );
        }
    }

    emit ensembleNamesFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parseCases( QNetworkReply* reply )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc      = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj  = doc.object();
        QJsonObject   rootHits = jsonObj["hits"].toObject();

        QJsonArray hitsObjects = rootHits["hits"].toArray();

        m_cases.clear();

        foreach ( const QJsonValue& value, hitsObjects )
        {
            QJsonObject resultObj = value.toObject();
            auto        keys_1    = resultObj.keys();

            QJsonObject sourceObj  = resultObj["_source"].toObject();
            auto        sourceKeys = sourceObj.keys();

            QJsonObject fmuObj     = sourceObj["fmu"].toObject();
            auto        fmuObjKeys = fmuObj.keys();

            QJsonObject fmuCase     = fmuObj["case"].toObject();
            auto        fmuCaseKeys = fmuCase.keys();

            QString id        = resultObj["_id"].toString();
            QString kind      = "";
            QString fieldName = fmuCase["name"].toString();
            m_cases.push_back( SumoCase{ SumoCaseId( id ), kind, fieldName } );
        }

        emit casesFinished();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parseVectorNames( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    m_vectorNames.clear();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc     = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj = doc.object();

        QJsonArray tableHits = jsonObj["aggregations"].toObject()["smry_tables"].toObject()["buckets"].toArray();
        for ( const auto& tableHit : tableHits )
        {
            QJsonArray columnHits = tableHit.toObject()["smry_columns"].toObject()["buckets"].toArray();
            for ( const auto& columnHit : columnHits )
            {
                m_vectorNames.push_back( columnHit.toObject()["key"].toString() );
            }
        }
    }

    emit vectorNamesFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parseRealizationNumbers( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc     = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj = doc.object();

        QJsonArray hits = jsonObj["aggregations"].toObject()["realization-ids"].toObject()["buckets"].toArray();
        for ( const auto& hit : hits )
        {
            QJsonObject resultObj = hit.toObject();
            auto        keys_1    = resultObj.keys();

            auto val      = resultObj.value( "key" );
            auto intValue = val.toInt();

            auto realizationId = QString::number( intValue );
            m_realizationIds.push_back( realizationId );
        }
    }

    emit realizationIdsFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parseBlobIds( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    m_blobUrl.clear();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc     = QJsonDocument::fromJson( result );
        QJsonObject   jsonObj = doc.object();

        QJsonObject rootHits    = jsonObj["hits"].toObject();
        QJsonArray  hitsObjects = rootHits["hits"].toArray();

        foreach ( const QJsonValue& value, hitsObjects )
        {
            QJsonObject resultObj = value.toObject();
            auto        keys_1    = resultObj.keys();

            QJsonObject sourceObj  = resultObj["_source"].toObject();
            auto        sourceKeys = sourceObj.keys();

            QJsonObject fmuObj     = sourceObj["_sumo"].toObject();
            auto        fmuObjKeys = fmuObj.keys();

            auto blobName = fmuObj["blob_name"].toString();
            m_blobUrl.push_back( blobName );
        }
    }

    emit blobIdFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::saveFile( QNetworkReply* reply, const QString& fileId )
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
void RiaSumoConnector::addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& contentType )
{
    networkRequest.setHeader( QNetworkRequest::ContentTypeHeader, contentType );
    networkRequest.setRawHeader( "Authorization", "Bearer " + token.toUtf8() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::requestTokenBlocking()
{
    if ( !m_token.isEmpty() ) return m_token;

    QTimer timer;
    timer.setSingleShot( true );
    QEventLoop loop;
    connect( this, SIGNAL( tokenReady( const QString& ) ), &loop, SLOT( quit() ) );
    connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
    requestToken();
    timer.start( 10000 );
    loop.exec();
    return m_token;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoConnector::makeDownloadRequest( const QString& url, const QString& token, const QString& contentType )
{
    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token, contentType );

    auto reply = m_networkAccessManager->get( m_networkRequest );
    return reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestParquetData( const QString& url, const QString& token )
{
    RiaLogging::info( "Requesting download of parquet from: " + url );

    auto reply = makeDownloadRequest( url, token, RiaDefines::contentTypeJson() );
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
QString RiaSumoConnector::generateRandomString( int randomStringLength )
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
QString RiaSumoConnector::server() const
{
    return m_server;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoAsset> RiaSumoConnector::assets() const
{
    return m_assets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoCase> RiaSumoConnector::cases() const
{
    return m_cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoConnector::ensembleNamesForCase( const SumoCaseId& caseId ) const
{
    std::vector<QString> ensembleNames;
    for ( const auto& ensemble : m_ensembleNames )
    {
        if ( ensemble.caseId == caseId )
        {
            ensembleNames.push_back( ensemble.name );
        }
    }
    return ensembleNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoConnector::vectorNames() const
{
    return m_vectorNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoConnector::realizationIds() const
{
    return m_realizationIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoConnector::blobUrls() const
{
    return m_blobUrl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoRedirect> RiaSumoConnector::blobContents() const
{
    return m_redirectInfo;
}
