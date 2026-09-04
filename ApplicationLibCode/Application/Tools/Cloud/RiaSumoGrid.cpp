/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RiaSumoGrid.h"

#include "RiaCloudDefines.h"
#include "RiaLogging.h"
#include "RiaSumoConnector.h"
#include "RiaSumoDefines.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <optional>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> SumoGridInfo::realizationIds() const
{
    std::vector<int> ids;
    ids.reserve( realizationInfos.size() );

    for ( const auto& realizationInfo : realizationInfos )
    {
        ids.push_back( realizationInfo.realization );
    }

    return ids;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool SumoGridInfo::hasIdenticalDimensions( const std::vector<int>& realizations ) const
{
    if ( realizations.empty() ) return false;

    std::optional<SumoGridDimensions> referenceDimensions;

    for ( int realization : realizations )
    {
        auto it = std::ranges::find( realizationInfos, realization, &SumoGridRealizationInfo::realization );

        // A realization this grid does not have, or one the server reported no dimensions for, makes the
        // comparison inconclusive. Report not identical, so a grid is loaded per realization.
        if ( it == realizationInfos.end() || !it->dimensions.isValid() ) return false;

        if ( !referenceDimensions )
        {
            referenceDimensions = it->dimensions;
        }
        else if ( it->dimensions != *referenceDimensions )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoGrid::RiaSumoGrid( RiaSumoConnector& connector )
    : m_connector( connector )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoGrid::gridNames( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    const QString path = QString( "/cases/%1/ensembles/%2/grid_names" ).arg( caseId.get() ).arg( encodedEnsembleName );

    return parseGridNames( m_connector.getBlocking( path ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SumoGridInfo RiaSumoGrid::gridInfo( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedGridName     = QUrl::toPercentEncoding( gridName );

    const QString path = QString( "/cases/%1/ensembles/%2/grid_info/%3" ).arg( caseId.get() ).arg( encodedEnsembleName ).arg( encodedGridName );

    auto gridInfo = parseGridInfo( m_connector.getBlocking( path ) );

    // An empty result also means the request failed: the response body is discarded on any HTTP error.
    RiaLogging::debug( std::format( "Grid '{}' realization count : {}", gridName.toStdString(), gridInfo.realizationInfos.size() ) );

    return gridInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoGrid::gridData( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    const QString blobId = gridBlobId( caseId, ensembleName, gridName, realization );
    if ( blobId.isEmpty() ) return {};

    QByteArray contents = m_connector.downloadBlobBlocking( blobId, QString( "grid %1 realization %2" ).arg( gridName ).arg( realization ) );

    // Every realization downloads its own grid, also in shared grid mode, as the roff blob is the only
    // source of the active cells of a realization. Sharing a grid saves memory, not transfers.
    RiaLogging::debug( std::format( "Downloaded grid '{}' realization {}: {} bytes", gridName.toStdString(), realization, contents.size() ) );

    return contents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::gridBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedGridName     = QUrl::toPercentEncoding( gridName );

    const QString path = QString( "/cases/%1/ensembles/%2/grids/%3/realizations/%4/blob_id" )
                             .arg( caseId.get() )
                             .arg( encodedEnsembleName )
                             .arg( encodedGridName )
                             .arg( realization );

    return blobIdFromBody( m_connector.getBlocking( path ), "grid", gridName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoGridPropertyInfo>
    RiaSumoGrid::propertyInfo( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedGridName     = QUrl::toPercentEncoding( gridName );

    const QString path = QString( "/cases/%1/ensembles/%2/grids/%3/realizations/%4/property_info_list" )
                             .arg( caseId.get() )
                             .arg( encodedEnsembleName )
                             .arg( encodedGridName )
                             .arg( realization );

    return parsePropertyInfo( m_connector.getBlocking( path ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoGrid::propertyData( const SumoCaseId& caseId,
                                      const QString&    ensembleName,
                                      const QString&    gridName,
                                      int               realization,
                                      const QString&    propertyName,
                                      const QString&    isoDateOrInterval )
{
    const QString blobId = propertyBlobId( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );
    if ( blobId.isEmpty() ) return {};

    const QString description = isoDateOrInterval.isEmpty()
                                    ? QString( "%1 realization %2" ).arg( propertyName ).arg( realization )
                                    : QString( "%1 %2 realization %3" ).arg( propertyName, isoDateOrInterval ).arg( realization );

    return m_connector.downloadBlobBlocking( blobId, description );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QByteArray> RiaSumoGrid::propertyDataBatch( const SumoCaseId&           caseId,
                                                              const QString&              ensembleName,
                                                              const QString&              gridName,
                                                              int                         realization,
                                                              const QString&              propertyName,
                                                              const std::vector<QString>& isoDatesOrIntervals )
{
    // Drop duplicates so a time step is never requested twice in the same batch.
    std::vector<QString> timestampsToFetch;
    for ( const auto& isoDateOrInterval : isoDatesOrIntervals )
    {
        if ( std::ranges::find( timestampsToFetch, isoDateOrInterval ) != timestampsToFetch.end() ) continue;

        timestampsToFetch.push_back( isoDateOrInterval );
    }

    if ( timestampsToFetch.empty() ) return {};

    // The requests below are issued directly on the transfer thread, so the token and the address of the
    // service have to be in place before the work is handed over.
    m_connector.requestTokenBlocking();

    const QString baseUrl = m_connector.serviceBaseUrl();
    if ( baseUrl.isEmpty() ) return {};

    std::map<QString, QByteArray> contentsByTimestamp;

    m_connector.runOnTransferThreadBlocking(
        [&]()
        {
            contentsByTimestamp = fetchPropertyBatch( baseUrl, caseId, ensembleName, gridName, realization, propertyName, timestampsToFetch );
        },
        QString( "Loading %1 time step(s) of %2 from Sumo" ).arg( timestampsToFetch.size() ).arg( propertyName ) );

    return contentsByTimestamp;
}

//--------------------------------------------------------------------------------------------------
/// The async twin of propertyDataBatch, built on the same blob id request. Every time step is requested at
/// once and delivered on the connector thread as it arrives, so nothing waits for the slowest one.
//--------------------------------------------------------------------------------------------------
void RiaSumoGrid::propertyDataBatchAsync( const SumoCaseId&                                               caseId,
                                          const QString&                                                  ensembleName,
                                          const QString&                                                  gridName,
                                          int                                                             realization,
                                          const QString&                                                  propertyName,
                                          const std::vector<QString>&                                     isoDatesOrIntervals,
                                          const std::function<void( const QString&, const QByteArray& )>& onTimeStepReady,
                                          const void*                                                     cancelGroup )
{
    if ( isoDatesOrIntervals.empty() || !onTimeStepReady ) return;

    // Resolved here and not on the transfer thread: requesting the token is what starts the service, and
    // waiting for it uses the network access manager of the calling thread.
    const QString baseUrl = m_connector.serviceBaseUrl();
    if ( baseUrl.isEmpty() )
    {
        // Report every time step as failed rather than returning silently, so the caller is not left waiting
        // for replies that will never come.
        for ( const auto& isoDateOrInterval : isoDatesOrIntervals )
        {
            m_connector.invokeOnConnectorThread( [onTimeStepReady, isoDateOrInterval]() { onTimeStepReady( isoDateOrInterval, {} ); } );
        }
        return;
    }

    m_connector.runOnTransferThread(
        [this, baseUrl, caseId, ensembleName, gridName, realization, propertyName, isoDatesOrIntervals, onTimeStepReady, cancelGroup]()
        {
            for ( const auto& isoDateOrInterval : isoDatesOrIntervals )
            {
                auto deliver = [this, onTimeStepReady, isoDateOrInterval]( const QByteArray& contents )
                {
                    m_connector.invokeOnConnectorThread( [onTimeStepReady, isoDateOrInterval, contents]()
                                                         { onTimeStepReady( isoDateOrInterval, contents ); } );
                };

                auto blobIdReply = makePropertyBlobIdRequest( baseUrl,
                                                              caseId,
                                                              ensembleName,
                                                              gridName,
                                                              realization,
                                                              propertyName,
                                                              isoDateOrInterval,
                                                              m_connector.backgroundNetworkAccessManager(),
                                                              cancelGroup );
                if ( !blobIdReply )
                {
                    deliver( {} );
                    continue;
                }

                // Shorter than the transfer it precedes: this one moves almost no data, so minutes really
                // does mean broken.
                RiaSumoConnector::abortIfNotFinishedWithin( blobIdReply, RiaSumoDefines::blobLookupTimeoutMillis() );

                QObject::connect( blobIdReply,
                                  &QNetworkReply::finished,
                                  blobIdReply,
                                  [this, blobIdReply, propertyName, deliver, cancelGroup]()
                                  {
                                      const QString blobId = blobIdFromReply( blobIdReply, propertyName );
                                      if ( blobId.isEmpty() )
                                      {
                                          deliver( {} );
                                          return;
                                      }

                                      m_connector.downloadBlobAsync( blobId,
                                                                     deliver,
                                                                     RiaSumoDefines::gridPropertyTransferTimeoutMillis(),
                                                                     cancelGroup );
                                  } );
            }
        } );
}

//--------------------------------------------------------------------------------------------------
/// Resolve the blob ids of a batch of time steps and download the blobs. Always called on the transfer
/// thread, where the event loops it waits on dispatch no GUI events. The contents are returned rather
/// than retained, so the caller decides what to keep.
//--------------------------------------------------------------------------------------------------
std::map<QString, QByteArray> RiaSumoGrid::fetchPropertyBatch( const QString&              baseUrl,
                                                               const SumoCaseId&           caseId,
                                                               const QString&              ensembleName,
                                                               const QString&              gridName,
                                                               int                         realization,
                                                               const QString&              propertyName,
                                                               const std::vector<QString>& timestampsToFetch )
{
    // Phase 1: resolve all blob ids concurrently.
    std::vector<QNetworkReply*> blobIdReplies;
    for ( const auto& isoDateOrInterval : timestampsToFetch )
    {
        blobIdReplies.push_back( makePropertyBlobIdRequest( baseUrl,
                                                            caseId,
                                                            ensembleName,
                                                            gridName,
                                                            realization,
                                                            propertyName,
                                                            isoDateOrInterval,
                                                            m_connector.networkAccessManager() ) );
    }

    RiaSumoConnector::waitForRepliesToFinish( blobIdReplies );

    std::vector<QString> blobIds;
    for ( auto reply : blobIdReplies )
    {
        blobIds.push_back( blobIdFromReply( reply, propertyName ) );
    }

    // Phase 2: download all resolved blobs as one group.
    std::vector<QString> blobIdsToDownload;
    for ( const auto& blobId : blobIds )
    {
        if ( !blobId.isEmpty() ) blobIdsToDownload.push_back( blobId );
    }

    const auto contentsByBlobId = m_connector.downloadBlobs( baseUrl, blobIdsToDownload );

    // Anything missing failed to download, and is left out so the caller can fall back to a single request.
    std::map<QString, QByteArray> contentsByTimestamp;
    for ( size_t i = 0; i < blobIds.size(); i++ )
    {
        if ( blobIds[i].isEmpty() ) continue;

        if ( auto it = contentsByBlobId.find( blobIds[i] ); it != contentsByBlobId.end() )
        {
            contentsByTimestamp[timestampsToFetch[i]] = it->second;
        }
    }

    return contentsByTimestamp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::propertyBlobIdPath( const SumoCaseId& caseId,
                                         const QString&    ensembleName,
                                         const QString&    gridName,
                                         int               realization,
                                         const QString&    propertyName,
                                         const QString&    isoDateOrInterval )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedGridName     = QUrl::toPercentEncoding( gridName );
    const QString encodedPropertyName = QUrl::toPercentEncoding( propertyName );

    QString path = QString( "/cases/%1/ensembles/%2/grids/%3/realizations/%4/properties/%5/blob_id" )
                       .arg( caseId.get() )
                       .arg( encodedEnsembleName )
                       .arg( encodedGridName )
                       .arg( realization )
                       .arg( encodedPropertyName );

    // The timestamp/interval is an optional query parameter; omit it for static properties.
    if ( !isoDateOrInterval.isEmpty() )
    {
        path += QString( "?property_iso_date_or_interval=%1" ).arg( QString( QUrl::toPercentEncoding( isoDateOrInterval ) ) );
    }

    return path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::propertyBlobId( const SumoCaseId& caseId,
                                     const QString&    ensembleName,
                                     const QString&    gridName,
                                     int               realization,
                                     const QString&    propertyName,
                                     const QString&    isoDateOrInterval )
{
    const QString path = propertyBlobIdPath( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );

    return blobIdFromBody( m_connector.getBlocking( path ), "grid property", propertyName );
}

//--------------------------------------------------------------------------------------------------
/// Issue the blob id request for one grid property time step. The reply is returned unfinished, so the
/// caller decides how to wait for it: one at a time, or several at once when prefetching. networkManager
/// picks the connection pool, see the header comment.
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoGrid::makePropertyBlobIdRequest( const QString&          baseUrl,
                                                       const SumoCaseId&       caseId,
                                                       const QString&          ensembleName,
                                                       const QString&          gridName,
                                                       int                     realization,
                                                       const QString&          propertyName,
                                                       const QString&          isoDateOrInterval,
                                                       QNetworkAccessManager* networkManager,
                                                       const void*             cancelGroup )
{
    const QString url = baseUrl + propertyBlobIdPath( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );

    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( url ) );
    m_connector.addStandardHeader( networkRequest, m_connector.transferToken(), RiaCloudDefines::contentTypeJson() );

    return m_connector.getAndTrackReply( networkManager, networkRequest, cancelGroup );
}

//--------------------------------------------------------------------------------------------------
/// Read the blob id off a finished blob id reply. The reply is consumed and scheduled for deletion.
///
/// Waiting for one specific reply, rather than for a signal shared by all blob id requests, is what makes
/// the mapping correct: a still-pending reply from an earlier property's request can no longer satisfy
/// this wait and hand us its blob id, which previously caused e.g. SWAT to be served the SWCR blob.
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::blobIdFromReply( QNetworkReply* reply, const QString& propertyName )
{
    if ( !reply ) return {};

    const bool failed = !reply->isFinished() || reply->error() != QNetworkReply::NoError;
    QByteArray body   = failed ? QByteArray() : reply->readAll();

    if ( failed )
    {
        RiaLogging::error( std::format( "Request grid property blob ID failed: '{}'", reply->errorString().toStdString() ) );
    }

    reply->deleteLater();

    return blobIdFromBody( body, "grid property", propertyName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::blobIdFromBody( const QByteArray& body, const QString& kind, const QString& name )
{
    const QString blobId = RiaSumoConnector::blobIdFromBody( body );
    if ( blobId.isEmpty() ) return {};

    RiaLogging::debug( std::format( "Received blob ID for {} '{}': {}", kind.toStdString(), name.toStdString(), blobId.toStdString() ) );

    return blobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoGrid::parseGridNames( const QByteArray& body )
{
    std::vector<QString> gridNames;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        QString gridName = value.toString();

        gridNames.push_back( gridName );
    }

    RiaLogging::debug( std::format( "Grid names count : {}", gridNames.size() ) );

    return gridNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SumoGridInfo RiaSumoGrid::parseGridInfo( const QByteArray& body )
{
    SumoGridInfo gridInfo;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        const QJsonObject realizationObj = value.toObject();

        // A missing or null dimensions object leaves the counts at zero, which isValid() rejects, so a
        // realization Sumo has no dimensions for falls back to a grid of its own.
        const QJsonObject dimensionsObj = realizationObj["dimensions"].toObject();

        SumoGridRealizationInfo realizationInfo;
        realizationInfo.realization       = realizationObj["realization"].toInt( -1 );
        realizationInfo.dimensions.iCount = dimensionsObj["iCount"].toInt();
        realizationInfo.dimensions.jCount = dimensionsObj["jCount"].toInt();
        realizationInfo.dimensions.kCount = dimensionsObj["kCount"].toInt();

        gridInfo.realizationInfos.push_back( realizationInfo );
    }

    return gridInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoGridPropertyInfo> RiaSumoGrid::parsePropertyInfo( const QByteArray& body )
{
    std::vector<SumoGridPropertyInfo> propertyInfos;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        QJsonObject propertyObj = value.toObject();

        SumoGridPropertyInfo propertyInfo;
        propertyInfo.name = propertyObj["propertyName"].toString();

        // isoDateOrInterval is null for static properties.
        const auto isoValue = propertyObj["isoDateOrInterval"];
        if ( !isoValue.isNull() ) propertyInfo.isoDateOrInterval = isoValue.toString();

        propertyInfos.push_back( propertyInfo );
    }

    RiaLogging::debug( std::format( "Grid property info count : {}", propertyInfos.size() ) );

    return propertyInfos;
}
