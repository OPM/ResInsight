/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RiaSumoSummary.h"

#include "RiaCloudDefines.h"
#include "RiaLogging.h"
#include "RiaSumoConnector.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoSummary::RiaSumoSummary( RiaSumoConnector& connector )
    : m_connector( connector )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoSummary::vectorNames( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    const QString path = QString( "/cases/%1/ensembles/%2/vector_list" ).arg( caseId.get() ).arg( encodedEnsembleName );

    return parseVectorNames( m_connector.getBlocking( path ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoSummary::vectorData( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    const auto contentsByVectorName = vectorData( caseId, ensembleName, std::vector<QString>{ vectorName } );

    if ( auto it = contentsByVectorName.find( vectorName ); it != contentsByVectorName.end() ) return it->second;

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Fetch several summary vectors at the same time. The blob id requests are issued together and waited
/// for as a group, and so are the transfers, turning 2N sequential round trips into 2 batched ones.
///
/// This matters more than the round trip count alone: a vector that has not been aggregated yet is
/// produced on demand by the request that asks for it, so fetching serially costs the sum of those
/// aggregations while fetching together costs roughly the slowest one.
//--------------------------------------------------------------------------------------------------
std::map<QString, QByteArray>
    RiaSumoSummary::vectorData( const SumoCaseId& caseId, const QString& ensembleName, const std::vector<QString>& vectorNames )
{
    std::map<QString, QByteArray> contentsByVectorName;
    if ( vectorNames.empty() ) return contentsByVectorName;

    // Drop duplicates, so a vector is never requested twice in one batch.
    std::vector<QString> namesToFetch;
    for ( const auto& vectorName : vectorNames )
    {
        if ( vectorName.isEmpty() ) continue;
        if ( std::ranges::find( namesToFetch, vectorName ) != namesToFetch.end() ) continue;

        namesToFetch.push_back( vectorName );
    }

    if ( namesToFetch.empty() ) return contentsByVectorName;

    // The requests below are issued directly on the transfer thread, so the token and the address of the
    // service have to be in place before the work is handed over.
    m_connector.requestTokenBlocking();

    const QString baseUrl = m_connector.serviceBaseUrl();
    if ( baseUrl.isEmpty() ) return contentsByVectorName;

    m_connector.runOnTransferThreadBlocking(
        [&]()
        {
            // Phase 1: resolve all blob ids concurrently.
            std::vector<QNetworkReply*> blobIdReplies;
            for ( const auto& vectorName : namesToFetch )
            {
                blobIdReplies.push_back(
                    makeVectorBlobIdRequest( baseUrl, caseId, ensembleName, vectorName, m_connector.networkAccessManager() ) );
            }

            RiaSumoConnector::waitForRepliesToFinish( blobIdReplies );

            std::vector<QString> blobIds;
            for ( size_t i = 0; i < blobIdReplies.size(); i++ )
            {
                blobIds.push_back( blobIdFromReply( blobIdReplies[i], namesToFetch[i] ) );
            }

            // Phase 2: download all resolved blobs as one group.
            std::vector<QString> blobIdsToDownload;
            for ( const auto& blobId : blobIds )
            {
                if ( !blobId.isEmpty() ) blobIdsToDownload.push_back( blobId );
            }

            const auto contentsByBlobId = m_connector.downloadBlobs( baseUrl, blobIdsToDownload );

            // Anything missing failed; the caller falls back to fetching it on its own later.
            for ( size_t i = 0; i < blobIds.size(); i++ )
            {
                if ( blobIds[i].isEmpty() ) continue;

                if ( auto it = contentsByBlobId.find( blobIds[i] ); it != contentsByBlobId.end() )
                {
                    contentsByVectorName[namesToFetch[i]] = it->second;
                }
            }
        },
        QString( "Loading %1 summary vector(s) from Sumo" ).arg( namesToFetch.size() ) );

    return contentsByVectorName;
}

//--------------------------------------------------------------------------------------------------
/// Fetch several summary vectors without waiting for any of them. Every vector is requested at once and
/// onVectorReady is called for each one as it arrives, on the thread the connector lives on, so a caller can
/// show each vector the moment it is there instead of when the slowest one is.
///
/// Empty contents mean that vector failed. The callback is called exactly once per requested vector, so a
/// caller tracking what is still on its way can rely on all of them being accounted for.
//--------------------------------------------------------------------------------------------------
void RiaSumoSummary::vectorDataAsync( const SumoCaseId&                                               caseId,
                                      const QString&                                                  ensembleName,
                                      const std::vector<QString>&                                     vectorNames,
                                      const std::function<void( const QString&, const QByteArray& )>& onVectorReady,
                                      const void*                                                     cancelGroup )
{
    if ( vectorNames.empty() || !onVectorReady ) return;

    const QString baseUrl = m_connector.serviceBaseUrl();
    if ( baseUrl.isEmpty() ) return;

    m_connector.runOnTransferThread(
        [this, baseUrl, caseId, ensembleName, vectorNames, onVectorReady, cancelGroup]()
        {
            for ( const auto& vectorName : vectorNames )
            {
                auto deliver = [this, onVectorReady, vectorName]( const QByteArray& contents )
                {
                    m_connector.invokeOnConnectorThread( [onVectorReady, vectorName, contents]() { onVectorReady( vectorName, contents ); } );
                };

                auto blobIdReply =
                    makeVectorBlobIdRequest( baseUrl, caseId, ensembleName, vectorName, m_connector.backgroundNetworkAccessManager() );
                if ( !blobIdReply )
                {
                    deliver( {} );
                    continue;
                }

                m_connector.trackReply( cancelGroup, blobIdReply );

                // A vector that has not been aggregated yet is produced on demand by this request, which can
                // take a good while. Nothing is waiting on it, so it is given room to finish.
                RiaSumoConnector::abortIfNotFinishedWithin( blobIdReply, RiaSumoDefines::asyncRequestTimeoutMillis() );

                QObject::connect( blobIdReply,
                                  &QNetworkReply::finished,
                                  blobIdReply,
                                  [this, blobIdReply, vectorName, deliver, cancelGroup]()
                                  {
                                      const QString blobId = blobIdFromReply( blobIdReply, vectorName );
                                      if ( blobId.isEmpty() )
                                      {
                                          deliver( {} );
                                          return;
                                      }

                                      m_connector.downloadBlobAsync( blobId, deliver, RiaSumoDefines::requestTimeoutMillis(), cancelGroup );
                                  } );
            }
        } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoSummary::parameterData( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString blobId = parameterBlobId( caseId, ensembleName );
    if ( blobId.isEmpty() ) return {};

    return m_connector.downloadBlobBlocking( blobId, "ensemble parameters" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::vectorBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    const QString path = vectorBlobIdPath( caseId, ensembleName, vectorName );

    return logBlobId( RiaSumoConnector::blobIdFromBody( m_connector.getBlocking( path ) ), vectorName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::vectorBlobIdPath( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedVectorName   = QUrl::toPercentEncoding( vectorName );

    return QString( "/cases/%1/ensembles/%2/vectors/%3/blob_id" ).arg( caseId.get() ).arg( encodedEnsembleName ).arg( encodedVectorName );
}

//--------------------------------------------------------------------------------------------------
/// Issue the blob id request for one vector. The reply is returned unfinished, so the caller decides how
/// to wait for it: one at a time, or several at once when batching.
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoSummary::makeVectorBlobIdRequest( const QString&          baseUrl,
                                                        const SumoCaseId&       caseId,
                                                        const QString&          ensembleName,
                                                        const QString&          vectorName,
                                                        QNetworkAccessManager* networkManager )
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( baseUrl + vectorBlobIdPath( caseId, ensembleName, vectorName ) ) );
    m_connector.addStandardHeader( networkRequest, m_connector.transferToken(), RiaCloudDefines::contentTypeJson() );

    return networkManager->get( networkRequest );
}

//--------------------------------------------------------------------------------------------------
/// Read the blob id off a finished blob id reply. The reply is consumed and scheduled for deletion.
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::blobIdFromReply( QNetworkReply* reply, const QString& vectorName )
{
    if ( !reply ) return {};

    const bool failed = !reply->isFinished() || reply->error() != QNetworkReply::NoError;
    QByteArray body   = failed ? QByteArray() : reply->readAll();

    if ( failed )
    {
        RiaLogging::error(
            std::format( "Request blob ID failed for vector '{}': {}", vectorName.toStdString(), reply->errorString().toStdString() ) );
    }

    reply->deleteLater();

    return logBlobId( RiaSumoConnector::blobIdFromBody( body ), vectorName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::logBlobId( const QString& blobId, const QString& vectorName )
{
    if ( !blobId.isEmpty() )
    {
        RiaLogging::debug( std::format( "Received blob ID for vector '{}': {}", vectorName.toStdString(), blobId.toStdString() ) );
    }

    return blobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::parameterBlobIdPath( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    return QString( "/cases/%1/ensembles/%2/parameters/blob_id" ).arg( caseId.get() ).arg( encodedEnsembleName );
}

//--------------------------------------------------------------------------------------------------
/// Issue the blob id request for the ensemble parameters. The reply is returned unfinished, so the caller
/// decides how to wait for it.
//--------------------------------------------------------------------------------------------------
QNetworkReply*
    RiaSumoSummary::makeParameterBlobIdRequest( const QString& baseUrl, const SumoCaseId& caseId, const QString& ensembleName, QNetworkAccessManager* networkManager )
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( baseUrl + parameterBlobIdPath( caseId, ensembleName ) ) );
    m_connector.addStandardHeader( networkRequest, m_connector.transferToken(), RiaCloudDefines::contentTypeJson() );

    return networkManager->get( networkRequest );
}

//--------------------------------------------------------------------------------------------------
/// Fetch the ensemble parameters without waiting, calling onParametersReady on the thread the connector
/// lives on. Empty contents mean the request failed, and the callback is called exactly once.
//--------------------------------------------------------------------------------------------------
void RiaSumoSummary::parameterDataAsync( const SumoCaseId&                               caseId,
                                         const QString&                                  ensembleName,
                                         const std::function<void( const QByteArray& )>& onParametersReady,
                                         const void*                                     cancelGroup )
{
    if ( !onParametersReady ) return;

    const QString baseUrl = m_connector.serviceBaseUrl();
    if ( baseUrl.isEmpty() ) return;

    m_connector.runOnTransferThread(
        [this, baseUrl, caseId, ensembleName, onParametersReady, cancelGroup]()
        {
            auto deliver = [this, onParametersReady]( const QByteArray& contents )
            { m_connector.invokeOnConnectorThread( [onParametersReady, contents]() { onParametersReady( contents ); } ); };

            auto blobIdReply = makeParameterBlobIdRequest( baseUrl, caseId, ensembleName, m_connector.backgroundNetworkAccessManager() );
            if ( !blobIdReply )
            {
                deliver( {} );
                return;
            }

            m_connector.trackReply( cancelGroup, blobIdReply );

            RiaSumoConnector::abortIfNotFinishedWithin( blobIdReply, RiaSumoDefines::asyncRequestTimeoutMillis() );

            QObject::connect( blobIdReply,
                              &QNetworkReply::finished,
                              blobIdReply,
                              [this, blobIdReply, deliver, cancelGroup]()
                              {
                                  const QString blobId = blobIdFromReply( blobIdReply, "parameters" );
                                  if ( blobId.isEmpty() )
                                  {
                                      deliver( {} );
                                      return;
                                  }

                                  m_connector.downloadBlobAsync( blobId, deliver, RiaSumoDefines::requestTimeoutMillis(), cancelGroup );
                              } );
        } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::parameterBlobId( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString blobId = RiaSumoConnector::blobIdFromBody( m_connector.getBlocking( parameterBlobIdPath( caseId, ensembleName ) ) );

    if ( !blobId.isEmpty() )
    {
        RiaLogging::debug( std::format( "Received blob ID for parameters: {}", blobId.toStdString() ) );
    }

    return blobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoSummary::parseVectorNames( const QByteArray& body )
{
    std::vector<QString> vectorNames;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        QJsonObject vectorObj = value.toObject();
        vectorNames.push_back( vectorObj["name"].toString() );
    }

    return vectorNames;
}
