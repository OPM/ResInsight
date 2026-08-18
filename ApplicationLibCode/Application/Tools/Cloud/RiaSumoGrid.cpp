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

#include "RiaSumoGrid.h"

#include "RiaCloudDefines.h"
#include "RiaLogging.h"
#include "RiaSumoConnector.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoGrid::RiaSumoGrid( RiaSumoConnector& connector )
    : m_connector( connector )
    , m_blobCache( RiaSumoDefines::gridPropertyCacheLimitBytes() )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoGridInfo> RiaSumoGrid::gridInfo( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    const QString url =
        QString( "%1/cases/%2/ensembles/%3/grid_info_list" ).arg( m_connector.server() ).arg( caseId.get() ).arg( encodedEnsembleName );

    return parseGridInfo( m_connector.getBlocking( url ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoGrid::gridData( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    const QString blobId = gridBlobId( caseId, ensembleName, gridName, realization );
    if ( blobId.isEmpty() ) return {};

    return m_connector.downloadBlobBlocking( blobId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::gridBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedGridName     = QUrl::toPercentEncoding( gridName );

    const QString url = QString( "%1/cases/%2/ensembles/%3/grids/%4/realizations/%5/blob_id" )
                            .arg( m_connector.server() )
                            .arg( caseId.get() )
                            .arg( encodedEnsembleName )
                            .arg( encodedGridName )
                            .arg( realization );

    return blobIdFromBody( m_connector.getBlocking( url ), gridName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoGridPropertyInfo>
    RiaSumoGrid::propertyInfo( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedGridName     = QUrl::toPercentEncoding( gridName );

    const QString url = QString( "%1/cases/%2/ensembles/%3/grids/%4/realizations/%5/property_info_list" )
                            .arg( m_connector.server() )
                            .arg( caseId.get() )
                            .arg( encodedEnsembleName )
                            .arg( encodedGridName )
                            .arg( realization );

    return parsePropertyInfo( m_connector.getBlocking( url ) );
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
    // Serve from cache when possible. Keyed by the full property identity (not the blob id), a repeat request
    // is answered without even asking Sumo for the blob id. This avoids re-downloading every time step when a
    // property's global legend range is computed, and again when it is displayed.
    const QString key = cacheKey( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );
    if ( auto cachedContents = m_blobCache.lookup( key ); !cachedContents.isEmpty() )
    {
        return cachedContents;
    }

    const QString blobId = propertyBlobId( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );
    if ( blobId.isEmpty() ) return {};

    QByteArray contents = m_connector.downloadBlobBlocking( blobId );

    m_blobCache.insert( key, contents );

    return contents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoGrid::prefetchPropertyData( const SumoCaseId&           caseId,
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
        const QString key = cacheKey( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );

        if ( std::ranges::find( cacheKeys, key ) != cacheKeys.end() ) continue;
        if ( m_blobCache.contains( key ) ) continue;

        cacheKeys.push_back( key );
        timestampsToFetch.push_back( isoDateOrInterval );
    }

    if ( timestampsToFetch.size() < 2 ) return; // nothing to gain over the single time step path

    m_connector.runOnTransferThreadBlocking(
        [&]() { fetchPropertyBatch( caseId, ensembleName, gridName, realization, propertyName, timestampsToFetch, cacheKeys ); } );
}

//--------------------------------------------------------------------------------------------------
/// Resolve the blob ids of a batch of time steps, download the blobs, and put them in the cache. Always
/// called on the transfer thread, where the event loops it waits on dispatch no GUI events.
//--------------------------------------------------------------------------------------------------
void RiaSumoGrid::fetchPropertyBatch( const SumoCaseId&           caseId,
                                      const QString&              ensembleName,
                                      const QString&              gridName,
                                      int                         realization,
                                      const QString&              propertyName,
                                      const std::vector<QString>& timestampsToFetch,
                                      const std::vector<QString>& cacheKeys )
{
    // Phase 1: resolve all blob ids concurrently.
    std::vector<QNetworkReply*> blobIdReplies;
    for ( const auto& isoDateOrInterval : timestampsToFetch )
    {
        blobIdReplies.push_back( makePropertyBlobIdRequest( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval ) );
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

    const auto contentsByBlobId = m_connector.downloadBlobs( blobIdsToDownload );

    // Cache what arrived. Anything missing failed to download; the per time step path fetches it again later.
    for ( size_t i = 0; i < blobIds.size(); i++ )
    {
        if ( blobIds[i].isEmpty() ) continue;

        if ( auto it = contentsByBlobId.find( blobIds[i] ); it != contentsByBlobId.end() )
        {
            m_blobCache.insert( cacheKeys[i], it->second );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::propertyBlobIdUrl( const SumoCaseId& caseId,
                                        const QString&    ensembleName,
                                        const QString&    gridName,
                                        int               realization,
                                        const QString&    propertyName,
                                        const QString&    isoDateOrInterval ) const
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedGridName     = QUrl::toPercentEncoding( gridName );
    const QString encodedPropertyName = QUrl::toPercentEncoding( propertyName );

    QString url = QString( "%1/cases/%2/ensembles/%3/grids/%4/realizations/%5/properties/%6/blob_id" )
                      .arg( m_connector.server() )
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

    return url;
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
    const QString url = propertyBlobIdUrl( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );

    return blobIdFromBody( m_connector.getBlocking( url ), propertyName );
}

//--------------------------------------------------------------------------------------------------
/// Issue the blob id request for one grid property time step. The reply is returned unfinished, so the
/// caller decides how to wait for it: one at a time, or several at once when prefetching.
//--------------------------------------------------------------------------------------------------
QNetworkReply* RiaSumoGrid::makePropertyBlobIdRequest( const SumoCaseId& caseId,
                                                       const QString&    ensembleName,
                                                       const QString&    gridName,
                                                       int               realization,
                                                       const QString&    propertyName,
                                                       const QString&    isoDateOrInterval )
{
    const QString url = propertyBlobIdUrl( caseId, ensembleName, gridName, realization, propertyName, isoDateOrInterval );

    QNetworkRequest networkRequest;
    networkRequest.setUrl( QUrl( url ) );
    m_connector.addStandardHeader( networkRequest, m_connector.token(), RiaCloudDefines::contentTypeJson() );

    return m_connector.networkAccessManager()->get( networkRequest );
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

    return blobIdFromBody( body, propertyName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::blobIdFromBody( const QByteArray& body, const QString& name )
{
    const QString blobId = RiaSumoConnector::blobIdFromBody( body );
    if ( blobId.isEmpty() ) return {};

    RiaLogging::debug( std::format( "Received blob ID for vector '{}': {}", name.toStdString(), blobId.toStdString() ) );

    return blobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoGrid::cacheKey( const SumoCaseId& caseId,
                               const QString&    ensembleName,
                               const QString&    gridName,
                               int               realization,
                               const QString&    propertyName,
                               const QString&    isoDateOrInterval )
{
    return QString( "%1|%2|%3|%4|%5|%6" ).arg( caseId.get(), ensembleName, gridName ).arg( realization ).arg( propertyName, isoDateOrInterval );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoGridInfo> RiaSumoGrid::parseGridInfo( const QByteArray& body )
{
    std::vector<SumoGridInfo> gridInfos;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
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

        gridInfos.push_back( gridInfo );
    }

    RiaLogging::debug( std::format( "Grid info count : {}", gridInfos.size() ) );

    return gridInfos;
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
