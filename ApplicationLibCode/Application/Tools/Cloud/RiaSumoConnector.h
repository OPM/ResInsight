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

#pragma once

#include "RiaCloudConnector.h"
#include "RiaSumoDefines.h"
#include "RiaSumoExplore.h"
#include "RiaSumoGrid.h"
#include "RiaSumoSummary.h"

#include <QByteArray>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>

#include <functional>
#include <list>
#include <map>

class QEventLoop;
class QThread;

using SumoObjectId = QString;

//==================================================================================================
///
//==================================================================================================
class RiaSumoConnector : public RiaCloudConnector
{
    Q_OBJECT
public:
    // The Sumo data is served by a server whose address is not known when the connector is constructed,
    // and which can change while the connector is alive. It is therefore supplied as a callable that is
    // invoked for every request, rather than as a fixed address.
    RiaSumoConnector( QObject*                 parent,
                      std::function<QString()> serverUrlProvider,
                      const QString&           authority,
                      const QString&           scopes,
                      const QString&           clientId,
                      unsigned int             port );
    ~RiaSumoConnector() override;

    QString server() const override;

    // Download blobs by id and return their contents. Getting a blob takes two round trips, one for the
    // pre-signed URI and one for the data, and a batch does each of those as one concurrent group.
    // The description names what is being fetched in the progress dialog, e.g. "grid DROGON realization 1".
    // Falls back to a plain file count when it is empty.
    QByteArray                    downloadBlobBlocking( const QString& blobId, const QString& description = {} );
    std::map<QString, QByteArray> downloadBlobsBlocking( const std::vector<QString>& blobIds, const QString& description = {} );

    // What Sumo holds: assets, cases, ensembles and realizations.
    RiaSumoExplore& explore();

    // The grid data of a case. Owned here so its blob cache lives as long as the connection.
    RiaSumoGrid& grid();

    // The summary data of a case.
    RiaSumoSummary& summary();

    // Transport used by the data specific delegates. Every request goes through the transfer thread, so
    // the calling thread waits without dispatching events.
    // Issue a GET against the local cloud API service and return the response body. Takes the path only,
    // e.g. "/cases/{id}/ensembles": the base URL is prepended here, once the service is known to answer.
    // Composing the full URL at the call site would capture the address before the service has one.
    QByteArray getBlocking( const QString& path, const QString& progressText = {} );

    // The base URL of the local cloud API service, empty when it could not be made ready. May block while
    // the service starts, so call it from the calling thread and not from the transfer thread.
    QString serviceBaseUrl();

    // The REST API returns a blob id as a plain string, quoted by FastAPI.
    static QString blobIdFromBody( const QByteArray& body );

    void addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& contentType );

    // Run work on the transfer thread and wait for it. Pass progressText to show the standard progress dialog
    // while waiting, worth doing for the transfers slow enough to be noticed and not for the small requests
    // that would only make it flash.
    void runOnTransferThreadBlocking( const std::function<void()>& work, const QString& progressText = {} );

    // Run work on the transfer thread without waiting for it. The async data paths use this: the result is
    // delivered by a callback rather than by returning, so the calling thread carries on immediately.
    void runOnTransferThread( const std::function<void()>& work );

    // Hand a call back to the thread the connector lives on, the one owning the user interface. Results of
    // async work are delivered through this, so a caller never has its data handed to it on another thread.
    void invokeOnConnectorThread( const std::function<void()>& work );

    // Download one blob, calling onFinished with its contents. Call on the transfer thread, where onFinished
    // is called as well. Empty contents mean the transfer failed.
    void downloadBlobAsync( const QString& blobId, const std::function<void( const QByteArray& )>& onFinished );

    // Abort a reply that has not finished in time, so an async chain reports a failure instead of hanging and
    // leaving whoever waits for the data waiting forever.
    static void abortIfNotFinishedWithin( QNetworkReply* reply, int timeoutMillis );

    // The network manager belonging to the calling thread: the transfer thread manager when called from
    // there, otherwise the one owned by RiaCloudConnector on the GUI thread.
    QNetworkAccessManager* networkAccessManager();

    // The token for a request issued from the transfer thread. token() reads objects owned by another thread.
    QString transferToken() const;

    static void waitForRepliesToFinish( const std::vector<QNetworkReply*>& replies );

    // Issue and collect the two round trips a blob transfer needs. Called on the transfer thread.
    // Runs on the transfer thread, so the base URL is resolved by the caller and passed in: waiting for the
    // service must not happen here.
    std::map<QString, QByteArray> downloadBlobs( const QString& baseUrl, const std::vector<QString>& blobIds );

public slots:
    void requestFailed( const QAbstractOAuth::Error error );

private:
    // Call on the thread owning the authentication objects, before work is handed over.
    void cacheTransferToken();

    static QString constructSasUri( const QString& blobStoreBaseUri, const QString& blobId, const QString& sasToken );

    QString           sasUriFromReply( QNetworkReply* reply, const QString& blobId );
    static QByteArray blobContentsFromReply( QNetworkReply* reply, const QString& sasUri );
    static QByteArray replyBody( QNetworkReply* reply, const QString& url );

private:
    std::function<QString()> m_serverUrlProvider;

    RiaSumoExplore m_explore;
    RiaSumoGrid    m_grid;
    RiaSumoSummary m_summary;

    // Transfers run on their own thread so the calling thread can wait without dispatching events. Waiting on
    // a nested event loop on the GUI thread let the view update code re-enter a load that was still running,
    // and the same grid property was downloaded twice. Authentication stays on the GUI thread: the OAuth flow
    // opens a browser and its objects live there.
    QThread*               m_transferThread               = nullptr;
    QObject*               m_transferContext              = nullptr; // lives on the transfer thread
    QNetworkAccessManager* m_transferNetworkAccessManager = nullptr; // created on the transfer thread

    // Written on the thread handing work over, read on the transfer thread running it.
    mutable QMutex m_transferTokenMutex;
    QString        m_transferToken;
};
