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
    QByteArray                    downloadBlobBlocking( const QString& blobId );
    std::map<QString, QByteArray> downloadBlobsBlocking( const std::vector<QString>& blobIds );

    // What Sumo holds: assets, cases, ensembles and realizations.
    RiaSumoExplore& explore();

    // The grid data of a case. Owned here so its blob cache lives as long as the connection.
    RiaSumoGrid& grid();

    // The summary data of a case.
    RiaSumoSummary& summary();

    // Transport used by the data specific delegates. Every request goes through the transfer thread, so
    // the calling thread waits without dispatching events.
    QByteArray getBlocking( const QString& url );

    // The REST API returns a blob id as a plain string, quoted by FastAPI.
    static QString blobIdFromBody( const QByteArray& body );

    void addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& contentType );
    void runOnTransferThreadBlocking( const std::function<void()>& work );

    // The network manager belonging to the calling thread: the transfer thread manager when called from
    // there, otherwise the one owned by RiaCloudConnector on the GUI thread.
    QNetworkAccessManager* networkAccessManager();

    static void waitForRepliesToFinish( const std::vector<QNetworkReply*>& replies );

    // Issue and collect the two round trips a blob transfer needs. Called on the transfer thread.
    std::map<QString, QByteArray> downloadBlobs( const std::vector<QString>& blobIds );

public slots:
    void requestFailed( const QAbstractOAuth::Error error );

private:
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
};
