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

#pragma once

#include "RiaSumoBlobCache.h"
#include "RiaSumoDefines.h"

#include <QByteArray>
#include <QString>

#include <vector>

class RiaSumoConnector;
class QNetworkReply;

struct SumoGridInfo
{
    QString          name;
    std::vector<int> realizations;
};

struct SumoGridPropertyInfo
{
    QString name;

    // Empty for a static property. For a dynamic property this is either a single timestamp ("2018-01-01")
    // or an interval ("2018-01-01/2019-01-01"). ResInsight currently only supports the single-timestamp form.
    QString isoDateOrInterval;
};

//==================================================================================================
/// The grid data of a Sumo case: the grids of an ensemble, the grid geometry itself, and the grid
/// properties. Requests are made through RiaSumoConnector, which owns the connection and does the
/// transfers, and every call returns its result rather than leaving it in shared state.
//==================================================================================================
class RiaSumoGrid
{
public:
    explicit RiaSumoGrid( RiaSumoConnector& connector );

    std::vector<SumoGridInfo> gridInfo( const SumoCaseId& caseId, const QString& ensembleName );

    // The grid geometry as a binary roff blob.
    QByteArray gridData( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    std::vector<SumoGridPropertyInfo>
        propertyInfo( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    // One time step of one grid property, as a binary roff blob. Served from the blob cache when possible.
    QByteArray propertyData( const SumoCaseId& caseId,
                             const QString&    ensembleName,
                             const QString&    gridName,
                             int               realization,
                             const QString&    propertyName,
                             const QString&    isoDateOrInterval );

    // Download several time steps of one property concurrently and put them in the blob cache, so the
    // per time step requests that follow are served without going to Sumo. Cached entries are skipped.
    void prefetchPropertyData( const SumoCaseId&           caseId,
                               const QString&              ensembleName,
                               const QString&              gridName,
                               int                         realization,
                               const QString&              propertyName,
                               const std::vector<QString>& isoDatesOrIntervals );

private:
    QString gridBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    QString propertyBlobId( const SumoCaseId& caseId,
                            const QString&    ensembleName,
                            const QString&    gridName,
                            int               realization,
                            const QString&    propertyName,
                            const QString&    isoDateOrInterval );

    QNetworkReply* makePropertyBlobIdRequest( const SumoCaseId& caseId,
                                              const QString&    ensembleName,
                                              const QString&    gridName,
                                              int               realization,
                                              const QString&    propertyName,
                                              const QString&    isoDateOrInterval );

    void fetchPropertyBatch( const SumoCaseId&           caseId,
                             const QString&              ensembleName,
                             const QString&              gridName,
                             int                         realization,
                             const QString&              propertyName,
                             const std::vector<QString>& timestampsToFetch,
                             const std::vector<QString>& cacheKeys );

    QString propertyBlobIdUrl( const SumoCaseId& caseId,
                               const QString&    ensembleName,
                               const QString&    gridName,
                               int               realization,
                               const QString&    propertyName,
                               const QString&    isoDateOrInterval ) const;

    // The full identity of one grid property time step, used as blob cache key.
    static QString cacheKey( const SumoCaseId& caseId,
                             const QString&    ensembleName,
                             const QString&    gridName,
                             int               realization,
                             const QString&    propertyName,
                             const QString&    isoDateOrInterval );

    static QString blobIdFromBody( const QByteArray& body, const QString& name );
    static QString blobIdFromReply( QNetworkReply* reply, const QString& propertyName );

    static std::vector<SumoGridInfo>         parseGridInfo( const QByteArray& body );
    static std::vector<SumoGridPropertyInfo> parsePropertyInfo( const QByteArray& body );

private:
    RiaSumoConnector& m_connector;

    // Downloaded grid property blobs, keyed by the full property identity, see cacheKey.
    RiaSumoBlobCache m_blobCache;
};
