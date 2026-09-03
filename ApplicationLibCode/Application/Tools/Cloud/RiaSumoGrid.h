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

#include "RiaSumoDefines.h"

#include <QByteArray>
#include <QString>

#include <functional>
#include <map>
#include <vector>

class RiaSumoConnector;
class QNetworkReply;
class QNetworkAccessManager;

struct SumoGridDimensions
{
    int iCount = 0;
    int jCount = 0;
    int kCount = 0;

    bool isValid() const { return iCount > 0 && jCount > 0 && kCount > 0; }

    bool operator==( const SumoGridDimensions& ) const = default;
};

struct SumoGridRealizationInfo
{
    int                realization = -1;
    SumoGridDimensions dimensions;
};

//==================================================================================================
/// The realizations one grid of an ensemble exists for, each with the IJK dimensions Sumo reports. The
/// grid is identified by the request, so its name is not repeated here.
//==================================================================================================
struct SumoGridInfo
{
    std::vector<SumoGridRealizationInfo> realizationInfos;

    // The realizations this grid is available for.
    std::vector<int> realizationIds() const;

    // True when all the given realizations report the same valid IJK dimensions, which is what makes it
    // safe to share one grid between them. False when any of them is missing from this grid or reports
    // invalid dimensions, so the caller falls back to loading a grid per realization.
    bool hasIdenticalDimensions( const std::vector<int>& realizations ) const;
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

    // The names of the grids available in an ensemble.
    std::vector<QString> gridNames( const SumoCaseId& caseId, const QString& ensembleName );

    // The realizations one grid exists for, each with the IJK dimensions Sumo reports for it. Used by the
    // reservoir grid ensemble, which needs the dimensions to decide whether the realizations can share a grid.
    SumoGridInfo gridInfo( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName );

    // The grid geometry as a binary roff blob.
    QByteArray gridData( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    std::vector<SumoGridPropertyInfo>
        propertyInfo( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    // One time step of one grid property, as a binary roff blob.
    QByteArray propertyData( const SumoCaseId& caseId,
                             const QString&    ensembleName,
                             const QString&    gridName,
                             int               realization,
                             const QString&    propertyName,
                             const QString&    isoDateOrInterval );

    // Several time steps of one property, downloaded as one concurrent batch. The contents are keyed by
    // the time step they belong to, and a time step that failed to download is absent from the result.
    //
    // Nothing is retained here: the caller decodes the contents and keeps the values, see
    // RifReaderSumoGridProperty.
    std::map<QString, QByteArray> propertyDataBatch( const SumoCaseId&           caseId,
                                                     const QString&              ensembleName,
                                                     const QString&              gridName,
                                                     int                         realization,
                                                     const QString&              propertyName,
                                                     const std::vector<QString>& isoDatesOrIntervals );

    // The async twin of propertyDataBatch. All time steps are requested at once and onTimeStepReady is called
    // for each as it arrives, on the connector thread, exactly once per requested step. Empty contents mean
    // that step failed. Transfers get a generous deadline, see
    // RiaSumoDefines::gridPropertyTransferTimeoutMillis.
    //
    // cancelGroup is passed on to RiaSumoConnector::downloadBlobAsync, see its documentation.
    void propertyDataBatchAsync( const SumoCaseId&                                               caseId,
                                 const QString&                                                  ensembleName,
                                 const QString&                                                  gridName,
                                 int                                                             realization,
                                 const QString&                                                  propertyName,
                                 const std::vector<QString>&                                     isoDatesOrIntervals,
                                 const std::function<void( const QString&, const QByteArray& )>& onTimeStepReady,
                                 const void*                                                     cancelGroup = nullptr );

private:
    QString gridBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    QString propertyBlobId( const SumoCaseId& caseId,
                            const QString&    ensembleName,
                            const QString&    gridName,
                            int               realization,
                            const QString&    propertyName,
                            const QString&    isoDateOrInterval );

    // Runs on the transfer thread, so the base URL is resolved by the caller and passed in. networkManager
    // lets the caller choose the connection pool: the shared one for a blocking fetch waited on directly, or
    // the background one for a prefetch batch, see RiaSumoConnector::backgroundNetworkAccessManager.
    QNetworkReply* makePropertyBlobIdRequest( const QString&          baseUrl,
                                              const SumoCaseId&       caseId,
                                              const QString&          ensembleName,
                                              const QString&          gridName,
                                              int                     realization,
                                              const QString&          propertyName,
                                              const QString&          isoDateOrInterval,
                                              QNetworkAccessManager* networkManager );

    std::map<QString, QByteArray> fetchPropertyBatch( const QString&              baseUrl,
                                                      const SumoCaseId&           caseId,
                                                      const QString&              ensembleName,
                                                      const QString&              gridName,
                                                      int                         realization,
                                                      const QString&              propertyName,
                                                      const std::vector<QString>& timestampsToFetch );

    static QString propertyBlobIdPath( const SumoCaseId& caseId,
                                       const QString&    ensembleName,
                                       const QString&    gridName,
                                       int               realization,
                                       const QString&    propertyName,
                                       const QString&    isoDateOrInterval );

    // 'kind' names what the blob holds, e.g. "grid" or "grid property", for the debug log.
    static QString blobIdFromBody( const QByteArray& body, const QString& kind, const QString& name );
    static QString blobIdFromReply( QNetworkReply* reply, const QString& propertyName );

    static std::vector<QString>              parseGridNames( const QByteArray& body );
    static SumoGridInfo                      parseGridInfo( const QByteArray& body );
    static std::vector<SumoGridPropertyInfo> parsePropertyInfo( const QByteArray& body );

private:
    RiaSumoConnector& m_connector;
};
