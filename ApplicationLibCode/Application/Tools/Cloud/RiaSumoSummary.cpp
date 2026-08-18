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

    const QString url =
        QString( "%1/cases/%2/ensembles/%3/vector_list" ).arg( m_connector.server() ).arg( caseId.get() ).arg( encodedEnsembleName );

    return parseVectorNames( m_connector.getBlocking( url ) );
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

    m_connector.runOnTransferThreadBlocking(
        [&]()
        {
            // Phase 1: resolve all blob ids concurrently.
            std::vector<QNetworkReply*> blobIdReplies;
            for ( const auto& vectorName : namesToFetch )
            {
                blobIdReplies.push_back( makeVectorBlobIdRequest( caseId, ensembleName, vectorName ) );
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

            const auto contentsByBlobId = m_connector.downloadBlobs( blobIdsToDownload );

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
                                      const std::function<void( const QString&, const QByteArray& )>& onVectorReady )
{
    if ( vectorNames.empty() || !onVectorReady ) return;

    m_connector.runOnTransferThread(
        [this, caseId, ensembleName, vectorNames, onVectorReady]()
        {
            for ( const auto& vectorName : vectorNames )
            {
                auto deliver = [this, onVectorReady, vectorName]( const QByteArray& contents )
                {
                    m_connector.invokeOnConnectorThread( [onVectorReady, vectorName, contents]() { onVectorReady( vectorName, contents ); } );
                };

                auto blobIdReply = makeVectorBlobIdRequest( caseId, ensembleName, vectorName );
                if ( !blobIdReply )
                {
                    deliver( {} );
                    continue;
                }

                // A vector that has not been aggregated yet is produced on demand by this request, which can
                // take a good while. Nothing is waiting on it, so it is given room to finish.
                RiaSumoConnector::abortIfNotFinishedWithin( blobIdReply, RiaSumoDefines::asyncRequestTimeoutMillis() );

                QObject::connect( blobIdReply,
                                  &QNetworkReply::finished,
                                  blobIdReply,
                                  [this, blobIdReply, vectorName, deliver]()
                                  {
                                      const QString blobId = blobIdFromReply( blobIdReply, vectorName );
                                      if ( blobId.isEmpty() )
                                      {
                                          deliver( {} );
                                          return;
                                      }

                                      m_connector.downloadBlobAsync( blobId, deliver );
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

    return m_connector.downloadBlobBlocking( blobId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::vectorBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    const QString url = vectorBlobIdUrl( caseId, ensembleName, vectorName );

    return logBlobId( RiaSumoConnector::blobIdFromBody( m_connector.getBlocking( url ) ), vectorName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::vectorBlobIdUrl( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName ) const
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedVectorName   = QUrl::toPercentEncoding( vectorName );

    return QString( "%1/cases/%2/ensembles/%3/vectors/%4/blob_id" )
        .arg( m_connector.server() )
        .arg( caseId.get() )
        .arg( encodedEnsembleName )
        .arg( encodedVectorName );
}

//--------------------------------------------------------------------------------------------------
/// Issue the blob id request for one vector. The reply is returned unfinished, so the caller decides how
/// to wait for it: one at a time, or several at once when batching.
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoSummary::makeVectorBlobIdRequest( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName )
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( vectorBlobIdUrl( caseId, ensembleName, vectorName ) ) );
    m_connector.addStandardHeader( networkRequest, m_connector.token(), RiaCloudDefines::contentTypeJson() );

    return m_connector.networkAccessManager()->get( networkRequest );
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
QString RiaSumoSummary::parameterBlobIdUrl( const SumoCaseId& caseId, const QString& ensembleName ) const
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    return QString( "%1/cases/%2/ensembles/%3/parameters/blob_id" ).arg( m_connector.server() ).arg( caseId.get() ).arg( encodedEnsembleName );
}

//--------------------------------------------------------------------------------------------------
/// Issue the blob id request for the ensemble parameters. The reply is returned unfinished, so the caller
/// decides how to wait for it.
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoSummary::makeParameterBlobIdRequest( const SumoCaseId& caseId, const QString& ensembleName )
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( parameterBlobIdUrl( caseId, ensembleName ) ) );
    m_connector.addStandardHeader( networkRequest, m_connector.token(), RiaCloudDefines::contentTypeJson() );

    return m_connector.networkAccessManager()->get( networkRequest );
}

//--------------------------------------------------------------------------------------------------
/// Fetch the ensemble parameters without waiting, calling onParametersReady on the thread the connector
/// lives on. Empty contents mean the request failed, and the callback is called exactly once.
//--------------------------------------------------------------------------------------------------
void RiaSumoSummary::parameterDataAsync( const SumoCaseId&                               caseId,
                                         const QString&                                  ensembleName,
                                         const std::function<void( const QByteArray& )>& onParametersReady )
{
    if ( !onParametersReady ) return;

    m_connector.runOnTransferThread(
        [this, caseId, ensembleName, onParametersReady]()
        {
            auto deliver = [this, onParametersReady]( const QByteArray& contents )
            { m_connector.invokeOnConnectorThread( [onParametersReady, contents]() { onParametersReady( contents ); } ); };

            auto blobIdReply = makeParameterBlobIdRequest( caseId, ensembleName );
            if ( !blobIdReply )
            {
                deliver( {} );
                return;
            }

            RiaSumoConnector::abortIfNotFinishedWithin( blobIdReply, RiaSumoDefines::asyncRequestTimeoutMillis() );

            QObject::connect( blobIdReply,
                              &QNetworkReply::finished,
                              blobIdReply,
                              [this, blobIdReply, deliver]()
                              {
                                  const QString blobId = blobIdFromReply( blobIdReply, "parameters" );
                                  if ( blobId.isEmpty() )
                                  {
                                      deliver( {} );
                                      return;
                                  }

                                  m_connector.downloadBlobAsync( blobId, deliver );
                              } );
        } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::parameterBlobId( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString blobId = RiaSumoConnector::blobIdFromBody( m_connector.getBlocking( parameterBlobIdUrl( caseId, ensembleName ) ) );

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
