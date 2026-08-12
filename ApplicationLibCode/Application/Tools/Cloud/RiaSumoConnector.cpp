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

#include "RiaCloudDefines.h"
#include "RiaLogging.h"
#include "RiaOAuthHttpServerReplyHandler.h"
#include "RiaOsduDefines.h"
#include "RiaQStringFormatter.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoConnector::RiaSumoConnector( QObject*                 parent,
                                    std::function<QString()> serverUrlProvider,
                                    const QString&           authority,
                                    const QString&           scopes,
                                    const QString&           clientId,
                                    unsigned int             port )
    : RiaCloudConnector( parent, {}, authority, scopes, clientId, port )
    , m_serverUrlProvider( std::move( serverUrlProvider ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::server() const
{
    // Ask for the address on every call. The server is bound to the first available port, and gets a new
    // port if it is restarted, so a cached address goes stale.
    if ( !m_serverUrlProvider ) return {};

    return m_serverUrlProvider();
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
RiaSumoConnector::~RiaSumoConnector()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestCasesForField( const QString& fieldName )
{
    m_cases.clear();

    requestTokenBlocking();

    QNetworkRequest m_networkRequest;
    QString         url = QString( "%1/cases?asset_name=%2" ).arg( server() ).arg( fieldName );
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 // parseCases handles the error case and always emits casesFinished, so the blocking caller
                 // returns immediately instead of waiting for the request to time out.
                 parseCases( reply );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestCasesForFieldBlocking( const QString& fieldName )
{
    auto        requestCallable = [this, fieldName] { requestCasesForField( fieldName ); };
    QMetaMethod signalMethod    = QMetaMethod::fromSignal( &RiaSumoConnector::casesFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestAssets()
{
    requestTokenBlocking();

    QNetworkRequest m_networkRequest;
    m_networkRequest.setUrl( QUrl( QString( "%1/assets" ).arg( server() ) ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply]()
             {
                 // parseAssets handles the error case and always emits assetsFinished, so the blocking caller
                 // returns immediately instead of waiting for the request to time out.
                 parseAssets( reply );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestAssetsBlocking()
{
    auto        requestCallable = [this] { requestAssets(); };
    QMetaMethod signalMethod    = QMetaMethod::fromSignal( &RiaSumoConnector::assetsFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestEnsembleByCasesId( const SumoCaseId& caseId )
{
    requestTokenBlocking();

    QNetworkRequest m_networkRequest;
    QString         url = QString( "%1/cases/%2/ensembles" ).arg( server() ).arg( caseId.get() );
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, caseId]()
             {
                 // parseEnsembleNames handles the error case and always emits ensembleNamesFinished, so the
                 // blocking caller returns immediately instead of waiting for the request to time out.
                 parseEnsembleNames( reply, caseId );
             } );
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

        QJsonDocument doc       = QJsonDocument::fromJson( result );
        QJsonArray    jsonArray = doc.array();

        for ( const QJsonValue& value : jsonArray )
        {
            QJsonObject ensembleObj  = value.toObject();
            QString     ensembleName = ensembleObj["name"].toString();
            m_ensembleNames.push_back( { caseId, ensembleName } );
        }

        RiaLogging::debug( std::format( "Ensemble count : {}", m_ensembleNames.size() ) );
    }
    else
    {
        RiaLogging::error( std::format( "Request ensemble names failed: '{}'", reply->errorString() ) );
    }

    emit ensembleNamesFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestEnsembleByCasesIdBlocking( const SumoCaseId& caseId )
{
    auto        requestCallable = [this, caseId] { requestEnsembleByCasesId( caseId ); };
    QMetaMethod signalMethod    = QMetaMethod::fromSignal( &RiaSumoConnector::ensembleNamesFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestVectorNamesForEnsemble( const SumoCaseId& caseId, const QString& ensembleName )
{
    requestTokenBlocking();

    QNetworkRequest m_networkRequest;
    QString         url = QString( "%1/cases/%2/ensembles/%3/vector_list" ).arg( server() ).arg( caseId.get() ).arg( ensembleName );
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId]()
             {
                 // parseVectorNames handles the error case and always emits vectorNamesFinished, so the
                 // blocking caller returns immediately instead of waiting for the request to time out.
                 parseVectorNames( reply, caseId, ensembleName );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestVectorNamesForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName )
{
    auto        requestCallable = [this, caseId, ensembleName] { requestVectorNamesForEnsemble( caseId, ensembleName ); };
    QMetaMethod signalMethod    = QMetaMethod::fromSignal( &RiaSumoConnector::vectorNamesFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestRealizationIdsForEnsemble( const SumoCaseId& caseId, const QString& ensembleName )
{
    m_realizationIds.clear();

    requestTokenBlocking();

    QNetworkRequest m_networkRequest;
    QString         url = QString( "%1/cases/%2/ensembles/%3/realizations" ).arg( server() ).arg( caseId.get() ).arg( ensembleName );
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId]()
             {
                 // parseRealizationNumbers handles the error case and always emits realizationIdsFinished, so
                 // the blocking caller returns immediately instead of waiting for the request to time out.
                 parseRealizationNumbers( reply, caseId, ensembleName );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestRealizationIdsForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName )
{
    auto        requestCallable = [this, caseId, ensembleName] { requestRealizationIdsForEnsemble( caseId, ensembleName ); };
    QMetaMethod signalMethod    = QMetaMethod::fromSignal( &RiaSumoConnector::realizationIdsFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::requestParametersParquetDataBlocking( const SumoCaseId& caseId, const QString& ensembleName )
{
    requestParametersBlobIdForEnsembleBlocking( caseId, ensembleName );

    if ( m_blobId.empty() ) return {};

    auto blobId = m_blobId.back();

    QEventLoop eventLoop;
    QTimer     timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, SIGNAL( timeout() ), &eventLoop, SLOT( quit() ) );
    QObject::connect( this, SIGNAL( parquetDownloadFinished( const QByteArray&, const QString& ) ), &eventLoop, SLOT( quit() ) );

    requestBlobDownload( blobId );

    timer.start( RiaSumoDefines::requestTimeoutMillis() );
    eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );

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
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestParametersBlobIdForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName )
{
    auto        requestCallable = [this, caseId, ensembleName] { requestParametersBlobIdForEnsemble( caseId, ensembleName ); };
    QMetaMethod signalMethod    = QMetaMethod::fromSignal( &RiaSumoConnector::blobIdFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestParametersBlobIdForEnsemble( const SumoCaseId& caseId, const QString& ensembleName )
{
    requestTokenBlocking();

    QNetworkRequest networkRequest;

    // Properly URL-encode the path components
    QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    QString url = QString( "%1/cases/%2/ensembles/%3/parameters/blob_id" ).arg( server() ).arg( caseId.get() ).arg( encodedEnsembleName );
    networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId]()
             {
                 // parseBlobId handles the error case and always emits blobIdFinished, so the blocking
                 // caller returns immediately instead of waiting for the request to time out.
                 parseBlobId( reply, caseId, ensembleName, "", true );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobIdForEnsemble( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    requestTokenBlocking();

    QNetworkRequest networkRequest;

    // Properly URL-encode the path components
    QString encodedVectorName   = QUrl::toPercentEncoding( vectorName );
    QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    QString url =
        QString( "%1/cases/%2/ensembles/%3/vectors/%4/blob_id" ).arg( server() ).arg( caseId.get() ).arg( encodedEnsembleName ).arg( encodedVectorName );
    networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId, vectorName]()
             {
                 // parseBlobId handles the error case and always emits blobIdFinished, so the blocking
                 // caller returns immediately instead of waiting for the request to time out.
                 parseBlobId( reply, caseId, ensembleName, vectorName, false ); // false = vector data
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobIdForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    auto requestCallable     = [this, caseId, ensembleName, vectorName] { requestBlobIdForEnsemble( caseId, ensembleName, vectorName ); };
    QMetaMethod signalMethod = QMetaMethod::fromSignal( &RiaSumoConnector::blobIdFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobDownload( const QString& blobId )
{
    requestTokenBlocking();

    QString url = QString( "%1/blobs/%2/sas_token_and_blob_base_uri" ).arg( server() ).arg( blobId );

    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, blobId, url]()
             {
                 reply->deleteLater(); // don't leak the reply

                 if ( reply->error() != QNetworkReply::NoError )
                 {
                     RiaLogging::error( ( "Download failed: " + url + " failed. " + reply->errorString() ).toStdString() );
                     return;
                 }

                 // The backend returns BlobAccessInfo as JSON: { "sasToken": "...", "blobStoreBaseUri": "..." }
                 const QByteArray    contents = reply->readAll();
                 QJsonParseError     parseError;
                 const QJsonDocument doc = QJsonDocument::fromJson( contents, &parseError );
                 if ( parseError.error != QJsonParseError::NoError || !doc.isObject() )
                 {
                     RiaLogging::error(
                         std::format( "Could not parse blob access info response as JSON: {}", parseError.errorString().toStdString() ) );
                     return;
                 }

                 const QJsonObject obj         = doc.object();
                 const QString     sasToken    = obj.value( "sasToken" ).toString();
                 const QString     blobBaseUri = obj.value( "blobStoreBaseUri" ).toString();
                 if ( blobBaseUri.isEmpty() )
                 {
                     RiaLogging::error( "Blob access info response did not contain a blobStoreBaseUri." );
                     return;
                 }

                 const QString sasUri = constructSasUri( blobBaseUri, blobId, sasToken );
                 requestBlobBySasUri( blobId, sasUri );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestBlobBySasUri( const QString& blobId, const QString& sasUri )
{
    RiaLogging::debug( std::format( "Requesting blob. Id: {} SAS URI: {}", blobId, sasUri ) );

    QNetworkRequest networkRequest;
    networkRequest.setUrl( sasUri );

    // The pre-signed SAS URI carries its own credential (signature in the query string),
    // so no Authorization header is added here. Do NOT forward the bearer token to the
    // storage host. Redirect policy is set explicitly so behaviour is not Qt-version dependent.
    networkRequest.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );

    auto reply = m_networkAccessManager->get( networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, blobId, sasUri]()
             {
                 reply->deleteLater();

                 if ( reply->error() != QNetworkReply::NoError )
                 {
                     QString errorMessage = "Download failed: " + sasUri + " failed." + reply->errorString();
                     RiaLogging::error( errorMessage.toStdString() );

                     emit parquetDownloadFinished( {}, sasUri );
                     return;
                 }

                 auto statusCode     = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
                 auto contentLength  = reply->header( QNetworkRequest::ContentLengthHeader ).toLongLong();
                 auto bytesAvailable = reply->bytesAvailable();

                 RiaLogging::debug(
                     std::format( "Response: status={}, content-length={}, bytes-available={}", statusCode, contentLength, bytesAvailable ) );

                 auto contents = reply->readAll();

                 RiaLogging::debug( std::format( "Read {} bytes from reply", contents.size() ) );

                 // Guard against a silently truncated transfer: a dropped connection on a
                 // chunked response can still report NoError. If the server told us how many
                 // bytes to expect and we got fewer, treat it as a failure.
                 if ( contentLength > 0 && contents.size() != contentLength )
                 {
                     RiaLogging::error( std::format( "Download truncated: expected {} bytes, got {}.", contentLength, contents.size() ) );

                     emit parquetDownloadFinished( {}, sasUri );
                     return;
                 }

                 QString msg = "Received data from : " + sasUri;
                 RiaLogging::debug( msg.toStdString() );

                 parquetDownloadComplete( blobId, contents, sasUri );

                 emit parquetDownloadFinished( contents, sasUri );
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::requestParquetDataBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    requestBlobIdForEnsembleBlocking( caseId, ensembleName, vectorName );

    if ( m_blobId.empty() ) return {};

    // The REST API now returns the complete blob URL, not just an ID
    auto blobId = m_blobId.back();

    QEventLoop eventLoop;
    QTimer     timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, SIGNAL( timeout() ), &eventLoop, SLOT( quit() ) );
    QObject::connect( this, SIGNAL( parquetDownloadFinished( const QByteArray&, const QString& ) ), &eventLoop, SLOT( quit() ) );

    requestBlobDownload( blobId );

    timer.start( RiaSumoDefines::requestTimeoutMillis() );
    eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );

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
/// Assemble the pre-signed download URI: {blobStoreBaseUri}/{blobId}?{sasToken}
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::constructSasUri( const QString& blobStoreBaseUri, const QString& blobId, const QString& sasToken )
{
    QString sasUri = blobStoreBaseUri;
    if ( !sasUri.endsWith( '/' ) ) sasUri += '/';
    sasUri += blobId;
    if ( !sasToken.isEmpty() )
    {
        sasUri += ( sasToken.startsWith( '?' ) ? sasToken : ( "?" + sasToken ) );
    }
    return sasUri;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::wrapAndCallNetworkRequest( std::function<void()> requestCallable, const QMetaMethod& signalMethod )
{
    QEventLoop eventLoop;

    QTimer timer;
    timer.setSingleShot( true );

    QObject::connect( &timer, &QTimer::timeout, [] { RiaLogging::error( "Sumo request timed out." ); } );
    QObject::connect( &timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit );

    // Not able to use the modern connect syntax here, as the signal is communicated as a QMetaMethod
    int         methodIndex = eventLoop.metaObject()->indexOfMethod( "quit()" );
    QMetaMethod quitMethod  = eventLoop.metaObject()->method( methodIndex );
    QObject::connect( this, signalMethod, &eventLoop, quitMethod );

    // Call the function that will execute the request
    requestCallable();

    timer.start( RiaSumoDefines::requestTimeoutMillis() );
    eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );
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
        QJsonDocument doc       = QJsonDocument::fromJson( result );
        QJsonArray    jsonArray = doc.array();

        m_assets.clear();

        // This json is an array of AssetInfo
        for ( const QJsonValue& assetInfo : jsonArray )
        {
            QString assetName = assetInfo["name"].toString();
            m_assets.push_back( SumoAsset{ SumoAssetId( "" ), "", assetName } );
        }

        for ( auto a : m_assets )
        {
            RiaLogging::debug( std::format( "Asset: {}", a.name ) );
        }
    }
    else
    {
        RiaLogging::error( std::format( "Request assets failed: '{}'", reply->errorString() ) );
    }

    emit assetsFinished();
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
        QJsonDocument doc       = QJsonDocument::fromJson( result );
        QJsonArray    jsonArray = doc.array();

        m_cases.clear();

        for ( const QJsonValue& value : jsonArray )
        {
            QJsonObject caseObj = value.toObject();

            QString id   = caseObj["id"].toString();
            QString kind = "";
            QString name = caseObj["name"].toString();
            m_cases.push_back( SumoCase{ SumoCaseId( id ), kind, name } );
        }

        RiaLogging::debug( std::format( "Case count : {}", m_cases.size() ) );
    }
    else
    {
        RiaLogging::error( std::format( "Request cases failed: '{}'", reply->errorString() ) );
    }

    emit casesFinished();
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
        QJsonDocument doc       = QJsonDocument::fromJson( result );
        QJsonArray    jsonArray = doc.array();

        for ( const QJsonValue& value : jsonArray )
        {
            QJsonObject vectorObj  = value.toObject();
            QString     vectorName = vectorObj["name"].toString();
            m_vectorNames.push_back( vectorName );
        }
    }
    else
    {
        RiaLogging::error( std::format( "Request vector names failed: '{}'", reply->errorString() ) );
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
        QJsonDocument doc       = QJsonDocument::fromJson( result );
        QJsonArray    jsonArray = doc.array();

        for ( const QJsonValue& value : jsonArray )
        {
            int  intValue      = value.toInt();
            auto realizationId = QString::number( intValue );
            m_realizationIds.push_back( realizationId );
        }
    }
    else
    {
        RiaLogging::error( std::format( "Request realization IDs failed: '{}'", reply->errorString() ) );
    }

    emit realizationIdsFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parseBlobId( QNetworkReply*    reply,
                                    const SumoCaseId& caseId,
                                    const QString&    ensembleName,
                                    const QString&    vectorName,
                                    bool              isParameters )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    m_blobId.clear();

    if ( reply->error() == QNetworkReply::NoError )
    {
        // The REST API returns a plain string (the blob id)
        QString blobId = QString::fromUtf8( result ).trimmed();

        // Remove quotes if present (FastAPI returns strings with quotes)
        if ( blobId.startsWith( '"' ) && blobId.endsWith( '"' ) )
        {
            blobId = blobId.mid( 1, blobId.length() - 2 );
        }

        m_blobId.push_back( blobId );

        // Context-aware logging
        if ( isParameters )
        {
            RiaLogging::debug( std::format( "Received blob ID for parameters: {}", blobId.toStdString() ) );
        }
        else
        {
            RiaLogging::debug( std::format( "Received blob ID for vector '{}': {}", vectorName.toStdString(), blobId.toStdString() ) );
        }
    }
    else
    {
        // Context-aware error logging
        QString errorContext = isParameters ? "parameters" : QString( "vector '%1'" ).arg( vectorName );
        RiaLogging::error( std::format( "Request blob ID failed for {}: {}", errorContext.toStdString(), reply->errorString().toStdString() ) );
    }

    emit blobIdFinished();
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
    RiaLogging::debug( "Requesting download of parquet from: " + url.toStdString() );

    auto reply = makeDownloadRequest( url, token, RiaCloudDefines::contentTypeJson() );
    connect( reply,
             &QNetworkReply::finished,
             [this, reply, url]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     QByteArray contents = reply->readAll();
                     RiaLogging::debug( std::format( "Download succeeded: {} bytes.", contents.length() ) );
                     RiaLogging::debug( std::format( "Download succeeded for url: {}", url.toStdString() ) );
                     emit parquetDownloadFinished( contents, "" );
                 }
                 else
                 {
                     QString errorMessage = "Download failed: " + url + " failed." + reply->errorString();
                     RiaLogging::error( errorMessage.toStdString() );
                     emit parquetDownloadFinished( QByteArray(), errorMessage );
                 }
             } );
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
std::vector<QString> RiaSumoConnector::blobIds() const
{
    return m_blobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoRedirect> RiaSumoConnector::blobContents() const
{
    return m_redirectInfo;
}
