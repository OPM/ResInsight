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

#include "cafProgressInfo.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSemaphore>
#include <QThread>
#include <QTimer>

#include <algorithm>
#include <optional>

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
    , m_explore( *this )
    , m_grid( *this )
    , m_summary( *this )
{
    // The transfer thread runs the network requests issued by the blocking wrappers, so the calling thread can
    // wait for them without dispatching events. The context object gives us something with transfer thread
    // affinity to post work to, and the network manager has to be constructed on the thread that uses it.
    m_transferThread  = new QThread( this );
    m_transferContext = new QObject;
    m_transferContext->moveToThread( m_transferThread );

    QObject::connect( m_transferThread,
                      &QThread::started,
                      m_transferContext,
                      [this]() { m_transferNetworkAccessManager = new QNetworkAccessManager( m_transferContext ); } );

    // The context object lives on the transfer thread, so let that thread delete it when it stops.
    QObject::connect( m_transferThread, &QThread::finished, m_transferContext, &QObject::deleteLater );

    m_transferThread->start();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoExplore& RiaSumoConnector::explore()
{
    return m_explore;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoGrid& RiaSumoConnector::grid()
{
    return m_grid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoSummary& RiaSumoConnector::summary()
{
    return m_summary;
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
RiaSumoConnector::~RiaSumoConnector()
{
    if ( m_transferThread )
    {
        m_transferThread->quit();
        m_transferThread->wait();
    }
}

//--------------------------------------------------------------------------------------------------
/// The network manager of the calling thread. A QNetworkAccessManager can only be used from the thread it
/// was created on, and the connector is used from both the GUI thread (the token flow and the Sumo Data
/// dialog) and the transfer thread (everything issued by a blocking wrapper).
//--------------------------------------------------------------------------------------------------
QNetworkAccessManager* RiaSumoConnector::networkAccessManager()
{
    if ( m_transferThread && QThread::currentThread() == m_transferThread && m_transferNetworkAccessManager )
    {
        return m_transferNetworkAccessManager;
    }

    return m_networkAccessManager;
}

//--------------------------------------------------------------------------------------------------
/// Run work on the transfer thread and return at once. Nothing is waited for here, so the result has to be
/// delivered by a callback. The token is requested first, while still on the calling thread, because
/// refreshing it may need the authentication objects that live there.
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::runOnTransferThread( const std::function<void()>& work )
{
    requestTokenBlocking();

    // Without a transfer thread there is nothing to hand the work to, and running it here is the only option.
    // Already on the transfer thread, run directly rather than queue behind work that may be waiting for us.
    if ( !m_transferThread || !m_transferContext || QThread::currentThread() == m_transferThread )
    {
        work();
        return;
    }

    QMetaObject::invokeMethod( m_transferContext, work, Qt::QueuedConnection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::invokeOnConnectorThread( const std::function<void()>& work )
{
    if ( QThread::currentThread() == thread() )
    {
        work();
        return;
    }

    QMetaObject::invokeMethod( this, work, Qt::QueuedConnection );
}

//--------------------------------------------------------------------------------------------------
/// A blob is fetched in two steps, first the pre-signed URI and then the data itself. Both are started here
/// and neither is waited for: each step continues from the reply of the one before.
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::downloadBlobAsync( const QString& blobId, const std::function<void( const QByteArray& )>& onFinished )
{
    const QString url = QString( "%1/blobs/%2/sas_token_and_blob_base_uri" ).arg( server() ).arg( blobId );

    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( url ) );
    addStandardHeader( networkRequest, token(), RiaCloudDefines::contentTypeJson() );

    auto accessInfoReply = networkAccessManager()->get( networkRequest );
    abortIfNotFinishedWithin( accessInfoReply, RiaSumoDefines::requestTimeoutMillis() );

    QObject::connect( accessInfoReply,
                      &QNetworkReply::finished,
                      m_transferContext,
                      [this, accessInfoReply, blobId, onFinished]()
                      {
                          const QString sasUri = sasUriFromReply( accessInfoReply, blobId );
                          if ( sasUri.isEmpty() )
                          {
                              onFinished( {} );
                              return;
                          }

                          RiaLogging::debug( std::format( "Requesting blob. Id: {} SAS URI: {}", blobId, sasUri ) );

                          QNetworkRequest blobRequest;
                          blobRequest.setUrl( sasUri );

                          // The pre-signed SAS URI carries its own credential, so no Authorization header is
                          // added. Do NOT forward the bearer token to the storage host.
                          blobRequest.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );

                          auto blobReply = networkAccessManager()->get( blobRequest );
                          abortIfNotFinishedWithin( blobReply, RiaSumoDefines::requestTimeoutMillis() );

                          QObject::connect( blobReply,
                                            &QNetworkReply::finished,
                                            m_transferContext,
                                            [blobReply, sasUri, onFinished]() { onFinished( blobContentsFromReply( blobReply, sasUri ) ); } );
                      } );
}

//--------------------------------------------------------------------------------------------------
/// Nothing waits on an async reply, so a request that never answers would otherwise keep its data pending
/// for good. Aborting makes the reply finish with an error, which the chain reports as a failed transfer.
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::abortIfNotFinishedWithin( QNetworkReply* reply, int timeoutMillis )
{
    if ( !reply ) return;

    QTimer::singleShot( timeoutMillis,
                        reply,
                        [reply]()
                        {
                            if ( !reply->isFinished() ) reply->abort();
                        } );
}

//--------------------------------------------------------------------------------------------------
/// Run work on the transfer thread and wait for it to finish, without dispatching any events on the calling
/// thread. This is what keeps the view update code from re-entering a load that is already running.
//--------------------------------------------------------------------------------------------------
void RiaSumoConnector::runOnTransferThreadBlocking( const std::function<void()>& work, const QString& progressText )
{
    // A blocking request can be issued from inside another one, for instance the blob id lookup done while
    // downloading a grid property. Already on the transfer thread, run directly instead of deadlocking on a
    // thread that is busy waiting for us.
    if ( !m_transferThread || !m_transferContext || QThread::currentThread() == m_transferThread )
    {
        work();
        return;
    }

    // Tell the user something is being loaded while this thread waits. Created only here, after the branch
    // above has returned for calls made from the transfer thread: caf::ProgressInfo hands construction to the
    // thread owning the user interface and waits for it, which would deadlock against a thread already
    // waiting for this work.
    //
    // The dialog must not be delayed. A delayed dialog is put up by a timer, and no events are dispatched on
    // this thread while the work runs, so it would never appear for exactly the requests slow enough to want
    // it. There is one step: the work is a single wait, with nothing to count along the way.
    std::optional<caf::ProgressInfo> progressInfo;
    if ( !progressText.isEmpty() )
    {
        const bool delayShowingProgress = false;
        progressInfo.emplace( 1, progressText, delayShowingProgress );
    }

    QSemaphore semaphore;

    QMetaObject::invokeMethod(
        m_transferContext,
        [&work, &semaphore]()
        {
            // Release on every exit path. A request that fails or times out must not leave the caller waiting.
            struct Releaser
            {
                QSemaphore& semaphore;
                ~Releaser() { semaphore.release(); }
            } releaser{ semaphore };

            work();
        },
        Qt::QueuedConnection );

    semaphore.acquire();

    // The transfer thread hands its log messages to the thread owning the message panel. Deliver them here,
    // while the request they describe is still the most recent thing that happened, or they would appear
    // after whatever the caller logs next and the log would read out of order.
    RiaLogging::flushPendingMessages();
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
        // The requests run concurrently, but give the group the timeout each of them would have been
        // given on its own. A batch must not be more likely to time out than the same requests made one
        // by one, and the server can take a while to answer: a summary vector that has not been
        // aggregated yet is produced on demand by the first request that asks for it.
        timer.start( static_cast<int>( replies.size() ) * RiaSumoDefines::requestTimeoutMillis() );
        eventLoop.exec();
    }

    for ( const auto& connection : connections )
    {
        QObject::disconnect( connection );
    }
}

//--------------------------------------------------------------------------------------------------
/// Download one blob and return its contents. The download and the wait for it run on the transfer thread.
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::downloadBlobBlocking( const QString& blobId )
{
    const auto contentsByBlobId = downloadBlobsBlocking( { blobId } );

    if ( auto it = contentsByBlobId.find( blobId ); it != contentsByBlobId.end() ) return it->second;

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Issue a GET and return the response body, waiting on the transfer thread. Returns an empty array when
/// the request fails. This is the primitive the data specific requests are built from.
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::getBlocking( const QString& url, const QString& progressText )
{
    requestTokenBlocking();

    QByteArray body;

    runOnTransferThreadBlocking(
        [&]()
        {
            QNetworkRequest networkRequest;
            networkRequest.setUrl( QUrl( url ) );
            addStandardHeader( networkRequest, token(), RiaCloudDefines::contentTypeJson() );

            auto reply = networkAccessManager()->get( networkRequest );

            QEventLoop eventLoop;
            QTimer     timer;
            timer.setSingleShot( true );
            QObject::connect( &timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit );
            QObject::connect( reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit );

            timer.start( RiaSumoDefines::requestTimeoutMillis() );
            eventLoop.exec();

            body = replyBody( reply, url );
        },
        progressText );

    return body;
}

//--------------------------------------------------------------------------------------------------
/// The REST API returns a blob id as a plain string, quoted by FastAPI.
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::blobIdFromBody( const QByteArray& body )
{
    if ( body.isEmpty() ) return {};

    QString blobId = QString::fromUtf8( body ).trimmed();

    if ( blobId.startsWith( '"' ) && blobId.endsWith( '"' ) )
    {
        blobId = blobId.mid( 1, blobId.length() - 2 );
    }

    return blobId;
}

//--------------------------------------------------------------------------------------------------
/// Read the body off a finished reply. The reply is consumed and scheduled for deletion.
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::replyBody( QNetworkReply* reply, const QString& url )
{
    if ( !reply ) return {};

    const bool failed = !reply->isFinished() || reply->error() != QNetworkReply::NoError;
    QByteArray body   = failed ? QByteArray() : reply->readAll();

    if ( failed )
    {
        RiaLogging::error( std::format( "Request failed: '{}': {}", url.toStdString(), reply->errorString().toStdString() ) );
    }

    reply->deleteLater();

    return body;
}

//--------------------------------------------------------------------------------------------------
/// Entry point for callers on any thread: hand the work to the transfer thread and wait for it there, so
/// the calling thread dispatches no events while the transfers are in flight. See downloadBlobs for what
/// the transfers actually are.
//--------------------------------------------------------------------------------------------------
std::map<QString, QByteArray> RiaSumoConnector::downloadBlobsBlocking( const std::vector<QString>& blobIds )
{
    if ( blobIds.empty() ) return {};

    requestTokenBlocking();

    std::map<QString, QByteArray> contentsByBlobId;

    runOnTransferThreadBlocking( [&]() { contentsByBlobId = downloadBlobs( blobIds ); },
                                 QString( "Downloading %1 file(s) from Sumo" ).arg( blobIds.size() ) );

    return contentsByBlobId;
}

//--------------------------------------------------------------------------------------------------
/// Download several blobs and return their contents by blob id. Getting a blob takes two round trips, one
/// for the pre-signed URI and one for the data itself, so both are done as a group: all the access info
/// requests are issued and waited for together, then all the transfers. A blob that fails is left out of
/// the returned map.
///
/// Always called on the transfer thread, where the event loops it waits on dispatch no GUI events.
//--------------------------------------------------------------------------------------------------
std::map<QString, QByteArray> RiaSumoConnector::downloadBlobs( const std::vector<QString>& blobIds )
{
    std::map<QString, QByteArray> contentsByBlobId;
    if ( blobIds.empty() ) return contentsByBlobId;

    // Phase 1: ask for the pre-signed URI of every blob.
    std::vector<QNetworkReply*> accessInfoReplies;
    for ( const auto& blobId : blobIds )
    {
        QString url = QString( "%1/blobs/%2/sas_token_and_blob_base_uri" ).arg( server() ).arg( blobId );

        QNetworkRequest networkRequest;
        networkRequest.setUrl( QUrl( url ) );
        addStandardHeader( networkRequest, token(), RiaCloudDefines::contentTypeJson() );

        accessInfoReplies.push_back( networkAccessManager()->get( networkRequest ) );
    }

    waitForRepliesToFinish( accessInfoReplies );

    std::vector<QString> sasUris;
    for ( size_t i = 0; i < accessInfoReplies.size(); i++ )
    {
        sasUris.push_back( sasUriFromReply( accessInfoReplies[i], blobIds[i] ) );
    }

    // Phase 2: transfer the blobs themselves.
    std::vector<QNetworkReply*> blobReplies;
    std::vector<size_t>         blobIndices; // index into blobIds for each reply
    for ( size_t i = 0; i < sasUris.size(); i++ )
    {
        if ( sasUris[i].isEmpty() ) continue;

        RiaLogging::debug( std::format( "Requesting blob. Id: {} SAS URI: {}", blobIds[i], sasUris[i] ) );

        QNetworkRequest networkRequest;
        networkRequest.setUrl( sasUris[i] );

        // The pre-signed SAS URI carries its own credential (signature in the query string), so no
        // Authorization header is added here. Do NOT forward the bearer token to the storage host. Redirect
        // policy is set explicitly so behaviour is not Qt-version dependent.
        networkRequest.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );

        blobReplies.push_back( networkAccessManager()->get( networkRequest ) );
        blobIndices.push_back( i );
    }

    waitForRepliesToFinish( blobReplies );

    for ( size_t i = 0; i < blobReplies.size(); i++ )
    {
        const size_t blobIndex = blobIndices[i];

        QByteArray contents = blobContentsFromReply( blobReplies[i], sasUris[blobIndex] );
        if ( !contents.isEmpty() )
        {
            contentsByBlobId[blobIds[blobIndex]] = contents;
        }
    }

    return contentsByBlobId;
}

//--------------------------------------------------------------------------------------------------
/// Read the pre-signed download URI off a finished blob access info reply. The reply is consumed and
/// scheduled for deletion. Returns an empty string on failure.
//--------------------------------------------------------------------------------------------------
QString RiaSumoConnector::sasUriFromReply( QNetworkReply* reply, const QString& blobId )
{
    if ( !reply ) return {};

    reply->deleteLater();

    if ( !reply->isFinished() || reply->error() != QNetworkReply::NoError )
    {
        RiaLogging::error(
            std::format( "Requesting access info for blob '{}' failed: {}", blobId.toStdString(), reply->errorString().toStdString() ) );
        return {};
    }

    // The backend returns BlobAccessInfo as JSON: { "sasToken": "...", "blobStoreBaseUri": "..." }
    const QByteArray    contents = reply->readAll();
    QJsonParseError     parseError;
    const QJsonDocument doc = QJsonDocument::fromJson( contents, &parseError );
    if ( parseError.error != QJsonParseError::NoError || !doc.isObject() )
    {
        RiaLogging::error( std::format( "Could not parse blob access info response as JSON: {}", parseError.errorString().toStdString() ) );
        return {};
    }

    const QJsonObject obj         = doc.object();
    const QString     sasToken    = obj.value( "sasToken" ).toString();
    const QString     blobBaseUri = obj.value( "blobStoreBaseUri" ).toString();
    if ( blobBaseUri.isEmpty() )
    {
        RiaLogging::error( "Blob access info response did not contain a blobStoreBaseUri." );
        return {};
    }

    return constructSasUri( blobBaseUri, blobId, sasToken );
}

//--------------------------------------------------------------------------------------------------
/// Read the blob contents off a finished transfer reply. The reply is consumed and scheduled for deletion.
/// Returns an empty array on failure.
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoConnector::blobContentsFromReply( QNetworkReply* reply, const QString& sasUri )
{
    if ( !reply ) return {};

    reply->deleteLater();

    if ( !reply->isFinished() || reply->error() != QNetworkReply::NoError )
    {
        RiaLogging::error( ( "Download failed: " + sasUri + " failed." + reply->errorString() ).toStdString() );
        return {};
    }

    auto statusCode     = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    auto contentLength  = reply->header( QNetworkRequest::ContentLengthHeader ).toLongLong();
    auto bytesAvailable = reply->bytesAvailable();

    RiaLogging::debug( std::format( "Response: status={}, content-length={}, bytes-available={}", statusCode, contentLength, bytesAvailable ) );

    auto contents = reply->readAll();

    RiaLogging::debug( std::format( "Read {} bytes from reply", contents.size() ) );

    // Guard against a silently truncated transfer: a dropped connection on a chunked response can still
    // report NoError. If the server told us how many bytes to expect and we got fewer, treat it as a failure.
    if ( contentLength > 0 && contents.size() != contentLength )
    {
        RiaLogging::error( std::format( "Download truncated: expected {} bytes, got {}.", contentLength, contents.size() ) );
        return {};
    }

    RiaLogging::debug( ( "Received data from : " + sasUri ).toStdString() );

    return contents;
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
void RiaSumoConnector::addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& contentType )
{
    networkRequest.setHeader( QNetworkRequest::ContentTypeHeader, contentType );
    networkRequest.setRawHeader( "Authorization", "Bearer " + token.toUtf8() );
}
