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

#include <algorithm>

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
void RiaSumoConnector::requestGridInfoForEnsemble( const SumoCaseId& caseId, const QString& ensembleName )
{
    m_gridInfos.clear();

    requestTokenBlocking();

    QString         encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    QNetworkRequest m_networkRequest;
    QString url = QString( "%1/cases/%2/ensembles/%3/grid_info_list" ).arg( server() ).arg( caseId.get() ).arg( encodedEnsembleName );
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseGridInfo( reply, caseId, ensembleName );
                 }
                 else
                 {
                     RiaLogging::error( std::format( "Request grid info failed: '{}'", reply->errorString().toStdString() ) );
                     emit gridInfoFinished();
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestGridInfoForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName )
{
    auto        requestCallable = [this, caseId, ensembleName] { requestGridInfoForEnsemble( caseId, ensembleName ); };
    QMetaMethod signalMethod    = QMetaMethod::fromSignal( &RiaSumoConnector::gridInfoFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestGridBlobIdForEnsemble( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    requestTokenBlocking();

    QNetworkRequest m_networkRequest;

    // Properly URL-encode the path components
    QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    QString encodedGridName     = QUrl::toPercentEncoding( gridName );

    QString url = QString( "%1/cases/%2/ensembles/%3/grids/%4/realizations/%5/blob_id" )
                      .arg( server() )
                      .arg( caseId.get() )
                      .arg( encodedEnsembleName )
                      .arg( encodedGridName )
                      .arg( realization );
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, ensembleName, caseId, gridName]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseBlobId( reply, caseId, ensembleName, gridName, false );
                 }
                 else
                 {
                     RiaLogging::error( std::format( "Request grid blob ID failed: '{}'", reply->errorString().toStdString() ) );
                     emit blobIdFinished();
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestGridBlobIdForEnsembleBlocking( const SumoCaseId& caseId,
                                                             const QString&    ensembleName,
                                                             const QString&    gridName,
                                                             int               realization )
{
    auto requestCallable = [this, caseId, ensembleName, gridName, realization]
    { requestGridBlobIdForEnsemble( caseId, ensembleName, gridName, realization ); };
    QMetaMethod signalMethod = QMetaMethod::fromSignal( &RiaSumoConnector::blobIdFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray
    RiaSumoConnector::requestGridDataBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    requestGridBlobIdForEnsembleBlocking( caseId, ensembleName, gridName, realization );

    if ( m_blobId.empty() ) return {};

    // The REST API returns the blob Id
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
void RiaSumoConnector::requestGridPropertyInfoForEnsemble( const SumoCaseId& caseId,
                                                           const QString&    ensembleName,
                                                           const QString&    gridName,
                                                           int               realization )
{
    m_gridPropertyInfos.clear();

    requestTokenBlocking();

    QNetworkRequest m_networkRequest;

    QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    QString encodedGridName     = QUrl::toPercentEncoding( gridName );

    QString url = QString( "%1/cases/%2/ensembles/%3/grids/%4/realizations/%5/property_info_list" )
                      .arg( server() )
                      .arg( caseId.get() )
                      .arg( encodedEnsembleName )
                      .arg( encodedGridName )
                      .arg( realization );
    m_networkRequest.setUrl( QUrl( url ) );

    addStandardHeader( m_networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto reply = m_networkAccessManager->get( m_networkRequest );

    connect( reply,
             &QNetworkReply::finished,
             [this, reply, caseId, ensembleName, gridName, realization]()
             {
                 if ( reply->error() == QNetworkReply::NoError )
                 {
                     parseGridPropertyInfo( reply, caseId, ensembleName, gridName, realization );
                 }
                 else
                 {
                     RiaLogging::error( std::format( "Request grid property info failed: '{}'", reply->errorString().toStdString() ) );
                     emit gridPropertyInfoFinished();
                 }
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::requestGridPropertyInfoForEnsembleBlocking( const SumoCaseId& caseId,
                                                                   const QString&    ensembleName,
                                                                   const QString&    gridName,
                                                                   int               realization )
{
    auto requestCallable = [this, caseId, ensembleName, gridName, realization]
    { requestGridPropertyInfoForEnsemble( caseId, ensembleName, gridName, realization ); };
    QMetaMethod signalMethod = QMetaMethod::fromSignal( &RiaSumoConnector::gridPropertyInfoFinished );
    wrapAndCallNetworkRequest( requestCallable, signalMethod );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::requestGridPropertyBlobIdBlocking( const SumoCaseId& caseId,
                                                             const QString&    ensembleName,
                                                             const QString&    gridName,
                                                             int               realization,
                                                             const QString&    propertyName,
                                                             const QString&    isoDateOrInterval )
{
    auto reply = makeGridPropertyBlobIdRequest( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );

    // Wait for THIS reply only. Binding the event loop to the reply, rather than to the shared blobIdFinished
    // signal, is what makes the mapping correct: a still-pending reply from an earlier property's request can no
    // longer satisfy this wait and hand us its blob id (which previously caused e.g. SWAT to be served the SWCR
    // blob). The blob id is read straight off this reply and never routed through shared state.
    QEventLoop eventLoop;
    QTimer     timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit );
    QObject::connect( reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit );

    timer.start( RiaSumoDefines::requestTimeoutMillis() );
    eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );

    return blobIdFromReply( reply, propertyName );
}

//--------------------------------------------------------------------------------------------------
/// Issue the blob id request for one grid property time step. The reply is returned unfinished, so the caller
/// decides how to wait for it: one at a time, or several at once when prefetching.
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoConnector::makeGridPropertyBlobIdRequest( const SumoCaseId& caseId,
                                                                const QString&    ensembleName,
                                                                const QString&    gridName,
                                                                int               realization,
                                                                const QString&    propertyName,
                                                                const QString&    isoDateOrInterval )
{
    requestTokenBlocking();

    // Properly URL-encode the path components
    QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    QString encodedGridName     = QUrl::toPercentEncoding( gridName );
    QString encodedPropertyName = QUrl::toPercentEncoding( propertyName );

    QString url = QString( "%1/cases/%2/ensembles/%3/grids/%4/realizations/%5/properties/%6/blob_id" )
                      .arg( server() )
                      .arg( caseId.get() )
                      .arg( encodedEnsembleName )
                      .arg( encodedGridName )
                      .arg( realization )
                      .arg( encodedPropertyName );

    // The timestamp/interval is an optional query parameter; omit it for static properties.
    if ( !isoDateOrInterval.isEmpty() )
    {
        url += QString( "?property_iso_date_or_interval=%1" ).arg( QString( QUrl::toPercentEncoding( isoDateOrInterval ) ) );
    }

    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( url ) );
    addStandardHeader( networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    return m_networkAccessManager->get( networkRequest );
}

//--------------------------------------------------------------------------------------------------
/// Read the blob id off a finished blob id reply. The reply is consumed and scheduled for deletion.
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::blobIdFromReply( QNetworkReply* reply, const QString& propertyName )
{
    if ( !reply ) return {};

    if ( !reply->isFinished() || reply->error() != QNetworkReply::NoError )
    {
        if ( reply->error() != QNetworkReply::NoError )
        {
            RiaLogging::error( std::format( "Request grid property blob ID failed: '{}'", reply->errorString().toStdString() ) );
        }
        reply->deleteLater();
        return {};
    }

    // The REST API returns the blob id as a plain string, quoted by FastAPI.
    QString blobId = QString::fromUtf8( reply->readAll() ).trimmed();
    reply->deleteLater();

    if ( blobId.startsWith( '"' ) && blobId.endsWith( '"' ) )
    {
        blobId = blobId.mid( 1, blobId.length() - 2 );
    }

    RiaLogging::debug( std::format( "Received blob ID for vector '{}': {}", propertyName.toStdString(), blobId.toStdString() ) );

    return blobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::requestGridPropertyDataBlocking( const SumoCaseId& caseId,
                                                              const QString&    ensembleName,
                                                              const QString&    gridName,
                                                              int               realization,
                                                              const QString&    propertyName,
                                                              const QString&    isoDateOrInterval )
{
    // Serve from cache when possible. Keyed by the full property identity (not the blob id), a repeat request is
    // answered without even asking Sumo for the blob id. This avoids re-downloading every time step when a
    // property's global legend range is computed, and again when it is displayed.
    const QString cacheKey = gridPropertyCacheKey( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );
    if ( auto cachedContents = gridPropertyBlobFromCache( cacheKey ); !cachedContents.isEmpty() )
    {
        return cachedContents;
    }

    // Resolve the blob id for this exact property. The getter waits on its own reply, so it can only ever return
    // this property's id (or empty on failure) - never a neighbouring request's id.
    const QString blobId = requestGridPropertyBlobIdBlocking( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );
    if ( blobId.isEmpty() ) return {};

    QEventLoop eventLoop;
    QTimer     timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, SIGNAL( timeout() ), &eventLoop, SLOT( quit() ) );
    QObject::connect( this, SIGNAL( parquetDownloadFinished( const QByteArray&, const QString& ) ), &eventLoop, SLOT( quit() ) );

    requestBlobDownload( blobId );

    timer.start( RiaSumoDefines::requestTimeoutMillis() );
    eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );

    // Move the downloaded blob out of the transient redirect list and into the cache. Erasing the consumed entry
    // also keeps m_redirectInfo from growing without bound as more properties are downloaded.
    for ( auto it = m_redirectInfo.begin(); it != m_redirectInfo.end(); ++it )
    {
        if ( it->objectId == blobId )
        {
            QByteArray contents = it->contents;
            m_redirectInfo.erase( it );
            insertGridPropertyBlobInCache( cacheKey, contents );
            return contents;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Fetch several time steps of one grid property at the same time. The blob id requests are issued together
/// and waited for as a group, and so are the blob downloads, turning 2N sequential round trips into 2 batched
/// ones. The results are placed in the blob cache, so the per time step requests that follow are cache hits.
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::prefetchGridPropertyDataBlocking( const SumoCaseId&           caseId,
                                                         const QString&              ensembleName,
                                                         const QString&              gridName,
                                                         int                         realization,
                                                         const QString&              propertyName,
                                                         const std::vector<QString>& isoDatesOrIntervals )
{
    // Only fetch what is not already cached, and drop duplicates so a time step is never requested twice.
    std::vector<QString> cacheKeys;
    std::vector<QString> timestampsToFetch;
    for ( const auto& isoDateOrInterval : isoDatesOrIntervals )
    {
        const QString cacheKey = gridPropertyCacheKey( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );

        if ( std::ranges::find( cacheKeys, cacheKey ) != cacheKeys.end() ) continue;
        if ( m_gridPropertyBlobCache.contains( cacheKey ) ) continue;

        cacheKeys.push_back( cacheKey );
        timestampsToFetch.push_back( isoDateOrInterval );
    }

    if ( timestampsToFetch.size() < 2 ) return; // nothing to gain over the single time step path

    // Phase 1: resolve all blob ids concurrently.
    std::vector<QNetworkReply*> blobIdReplies;
    for ( const auto& isoDateOrInterval : timestampsToFetch )
    {
        blobIdReplies.push_back( makeGridPropertyBlobIdRequest( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval ) );
    }

    waitForRepliesToFinish( blobIdReplies );

    std::vector<QString> blobIds;
    for ( auto reply : blobIdReplies )
    {
        blobIds.push_back( blobIdFromReply( reply, propertyName ) );
    }

    // Phase 2: download all resolved blobs concurrently. requestBlobDownload is fire and forget; each finished
    // download appends to m_redirectInfo, so wait until every requested blob has arrived there.
    std::vector<QString> pendingBlobIds;
    for ( const auto& blobId : blobIds )
    {
        if ( blobId.isEmpty() ) continue;

        pendingBlobIds.push_back( blobId );
        requestBlobDownload( blobId );
    }

    if ( !pendingBlobIds.empty() )
    {
        auto haveAllBlobsArrived = [this, &pendingBlobIds]()
        {
            return std::ranges::all_of( pendingBlobIds,
                                        [this]( const QString& blobId )
                                        {
                                            return std::ranges::any_of( m_redirectInfo,
                                                                        [&blobId]( const SumoRedirect& redirect )
                                                                        { return redirect.objectId == blobId; } );
                                        } );
        };

        // requestBlobDownload can spin an event loop of its own while acquiring a token, so a download may
        // already have completed here. Check before waiting, or the wait would run until it times out.
        if ( !haveAllBlobsArrived() )
        {
            QEventLoop eventLoop;
            QTimer     timer;
            timer.setSingleShot( true );
            QObject::connect( &timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit );

            // Every completed download emits this signal; quit once all of them have landed in m_redirectInfo.
            auto connection = QObject::connect( this,
                                                &RiaSumoConnector::parquetDownloadFinished,
                                                &eventLoop,
                                                [&eventLoop, &haveAllBlobsArrived]()
                                                {
                                                    if ( haveAllBlobsArrived() ) eventLoop.quit();
                                                } );

            // The downloads run concurrently, but allow the single request timeout per blob so a batch is
            // never given less time than the same blobs would get one by one.
            timer.start( static_cast<int>( pendingBlobIds.size() ) * RiaSumoDefines::requestTimeoutMillis() );
            eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );

            QObject::disconnect( connection );
        }
    }

    // Move the downloaded blobs out of the transient redirect list and into the cache. Anything missing was not
    // downloaded in time; the per time step path fetches it again later.
    for ( size_t i = 0; i < blobIds.size(); i++ )
    {
        if ( blobIds[i].isEmpty() ) continue;

        for ( auto it = m_redirectInfo.begin(); it != m_redirectInfo.end(); ++it )
        {
            if ( it->objectId == blobIds[i] )
            {
                insertGridPropertyBlobInCache( cacheKeys[i], it->contents );
                m_redirectInfo.erase( it );
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Wait until every reply has finished, or the timeout expires.
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::waitForRepliesToFinish( const std::vector<QNetworkReply*>& replies )
{
    if ( replies.empty() ) return;

    QEventLoop eventLoop;
    QTimer     timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit );

    auto isAllFinished = [&replies]()
    { return std::ranges::all_of( replies, []( QNetworkReply* reply ) { return reply && reply->isFinished(); } ); };

    std::vector<QMetaObject::Connection> connections;
    for ( auto reply : replies )
    {
        if ( !reply ) continue;

        connections.push_back( QObject::connect( reply,
                                                 &QNetworkReply::finished,
                                                 &eventLoop,
                                                 [&eventLoop, &isAllFinished]()
                                                 {
                                                     if ( isAllFinished() ) eventLoop.quit();
                                                 } ) );
    }

    if ( !isAllFinished() )
    {
        timer.start( RiaSumoDefines::requestTimeoutMillis() );
        eventLoop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );
    }

    for ( const auto& connection : connections )
    {
        QObject::disconnect( connection );
    }
}

//--------------------------------------------------------------------------------------------------
/// The full identity of one grid property time step, used as blob cache key.
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::gridPropertyCacheKey( const SumoCaseId& caseId,
                                                const QString&    ensembleName,
                                                const QString&    gridName,
                                                int               realization,
                                                const QString&    propertyName,
                                                const QString&    isoDateOrInterval )
{
    return QString( "%1|%2|%3|%4|%5|%6" ).arg( caseId.get(), ensembleName, gridName ).arg( realization ).arg( propertyName, isoDateOrInterval );
}

//--------------------------------------------------------------------------------------------------
/// Look up a downloaded grid property blob. Returns an empty array when the blob is not cached, which the
/// callers treat as a miss. A hit is moved to the front of the recency order, so it is evicted last.
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::gridPropertyBlobFromCache( const QString& cacheKey )
{
    auto it = m_gridPropertyBlobCache.find( cacheKey );
    if ( it == m_gridPropertyBlobCache.end() ) return {};

    m_gridPropertyBlobCacheOrder.splice( m_gridPropertyBlobCacheOrder.begin(), m_gridPropertyBlobCacheOrder, it->second.orderIterator );

    return it->second.contents;
}

//--------------------------------------------------------------------------------------------------
/// Cache a downloaded grid property blob, evicting the least recently used blobs until the cache is back
/// within the size limit.
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::insertGridPropertyBlobInCache( const QString& cacheKey, const QByteArray& contents )
{
    if ( contents.isEmpty() ) return;

    const size_t contentsSize = static_cast<size_t>( contents.size() );

    // A blob larger than the whole cache would evict everything else to make room for itself. Leave it uncached
    // instead, so the smaller blobs already present stay available.
    if ( contentsSize > RiaSumoDefines::gridPropertyCacheLimitBytes() ) return;

    // Re-inserting an existing key would leak its order list entry, so drop the previous version first.
    if ( auto it = m_gridPropertyBlobCache.find( cacheKey ); it != m_gridPropertyBlobCache.end() )
    {
        m_gridPropertyBlobCacheSizeBytes -= static_cast<size_t>( it->second.contents.size() );
        m_gridPropertyBlobCacheOrder.erase( it->second.orderIterator );
        m_gridPropertyBlobCache.erase( it );
    }

    m_gridPropertyBlobCacheOrder.push_front( cacheKey );
    m_gridPropertyBlobCache[cacheKey] = { contents, m_gridPropertyBlobCacheOrder.begin() };
    m_gridPropertyBlobCacheSizeBytes += contentsSize;

    while ( m_gridPropertyBlobCacheSizeBytes > RiaSumoDefines::gridPropertyCacheLimitBytes() && !m_gridPropertyBlobCacheOrder.empty() )
    {
        const QString& oldestKey = m_gridPropertyBlobCacheOrder.back();

        if ( auto it = m_gridPropertyBlobCache.find( oldestKey ); it != m_gridPropertyBlobCache.end() )
        {
            m_gridPropertyBlobCacheSizeBytes -= static_cast<size_t>( it->second.contents.size() );
            m_gridPropertyBlobCache.erase( it );
        }

        m_gridPropertyBlobCacheOrder.pop_back();
    }
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
/// Note that this event loop dispatches everything except user input, so while a request is in flight the
/// view update code can run and re-enter a cell result load that is already in progress. That makes the same
/// result load twice. Declining the second load in RigCaseCellResultsData is not a way out: the only "no"
/// that method can return is cvf::UNDEFINED_SIZE_T, which consumers read as "no such result" and act on -
/// RimEclipseResultDefinitionTools::updateCellResultLegend computes the legend range straight after
/// ensureKnownResultLoaded without checking it, and caches a range over no data. Preventing the re-entrancy
/// means not dispatching events here at all, which requires moving the transfers off the GUI thread.
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
void RiaSumoConnector::parseGridInfo( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    m_gridInfos.clear();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc       = QJsonDocument::fromJson( result );
        QJsonArray    jsonArray = doc.array();

        for ( const QJsonValue& value : jsonArray )
        {
            QJsonObject gridObj = value.toObject();

            SumoGridInfo gridInfo;
            gridInfo.name = gridObj["name"].toString();

            for ( const QJsonValue& realizationValue : gridObj["realizations"].toArray() )
            {
                gridInfo.realizations.push_back( realizationValue.toInt() );
            }

            m_gridInfos.push_back( gridInfo );
        }

        RiaLogging::debug( std::format( "Grid info count : {}", m_gridInfos.size() ) );
    }
    else
    {
        RiaLogging::error( std::format( "Request grid info failed: '{}'", reply->errorString().toStdString() ) );
    }

    emit gridInfoFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::parseGridPropertyInfo( QNetworkReply*    reply,
                                              const SumoCaseId& caseId,
                                              const QString&    ensembleName,
                                              const QString&    gridName,
                                              int               realization )
{
    QByteArray result = reply->readAll();
    reply->deleteLater();

    m_gridPropertyInfos.clear();

    if ( reply->error() == QNetworkReply::NoError )
    {
        QJsonDocument doc       = QJsonDocument::fromJson( result );
        QJsonArray    jsonArray = doc.array();

        for ( const QJsonValue& value : jsonArray )
        {
            QJsonObject propertyObj = value.toObject();

            SumoGridPropertyInfo propertyInfo;
            propertyInfo.name = propertyObj["propertyName"].toString();

            // isoDateOrInterval is null for static properties.
            const auto isoValue = propertyObj["isoDateOrInterval"];
            if ( !isoValue.isNull() ) propertyInfo.isoDateOrInterval = isoValue.toString();

            m_gridPropertyInfos.push_back( propertyInfo );
        }

        RiaLogging::debug( std::format( "Grid property info count : {}", m_gridPropertyInfos.size() ) );
    }
    else
    {
        RiaLogging::error( std::format( "Request grid property info failed: '{}'", reply->errorString().toStdString() ) );
    }

    emit gridPropertyInfoFinished();
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
std::vector<SumoGridInfo> RiaSumoConnector::gridInfos() const
{
    return m_gridInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoGridPropertyInfo> RiaSumoConnector::gridPropertyInfos() const
{
    return m_gridPropertyInfos;
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
