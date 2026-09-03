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

//==================================================================================================
/// The summary data of a Sumo case: the vectors an ensemble has, their values, and the ensemble
/// parameters. Requests are made through RiaSumoConnector, which owns the connection and does the
/// transfers, and every call returns its result rather than leaving it in shared state.
//==================================================================================================
class RiaSumoSummary
{
public:
    explicit RiaSumoSummary( RiaSumoConnector& connector );

    std::vector<QString> vectorNames( const SumoCaseId& caseId, const QString& ensembleName );

    // The values of one summary vector, for all realizations, as a parquet blob.
    QByteArray vectorData( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );

    // The same for several vectors at once, returned by vector name. The blob id requests are issued as one
    // concurrent group and so are the transfers, which matters because a vector that has not been aggregated
    // yet is produced on demand by the request asking for it.
    std::map<QString, QByteArray> vectorData( const SumoCaseId& caseId, const QString& ensembleName, const std::vector<QString>& vectorNames );

    // The same again, but without waiting: all vectors are requested at once and onVectorReady is called for
    // each as it arrives, on the thread the connector lives on. Empty contents mean that vector failed, and
    // the callback is called exactly once per requested vector.
    //
    // cancelGroup is passed on to RiaSumoConnector::downloadBlobAsync, see its documentation.
    void vectorDataAsync( const SumoCaseId&                                               caseId,
                          const QString&                                                  ensembleName,
                          const std::vector<QString>&                                     vectorNames,
                          const std::function<void( const QString&, const QByteArray& )>& onVectorReady,
                          const void*                                                     cancelGroup = nullptr );

    // The ensemble parameters, as a parquet blob.
    QByteArray parameterData( const SumoCaseId& caseId, const QString& ensembleName );

    // The same without waiting. Like the vectors, the parameters are aggregated on demand by the service, so
    // the first request for them can take a while and is not something to hold the user interface for.
    void parameterDataAsync( const SumoCaseId&                               caseId,
                             const QString&                                  ensembleName,
                             const std::function<void( const QByteArray& )>& onParametersReady,
                             const void*                                     cancelGroup = nullptr );

    QString vectorBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );
    QString parameterBlobId( const SumoCaseId& caseId, const QString& ensembleName );

private:
    static QString vectorBlobIdPath( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );
    static QString parameterBlobIdPath( const SumoCaseId& caseId, const QString& ensembleName );

    // Run on the transfer thread, so the base URL is resolved by the caller and passed in. networkManager
    // lets the caller choose the connection pool: the shared one for a blocking fetch waited on directly, or
    // the background one for a prefetch batch, see RiaSumoConnector::backgroundNetworkAccessManager.
    QNetworkReply*
        makeParameterBlobIdRequest( const QString& baseUrl, const SumoCaseId& caseId, const QString& ensembleName, QNetworkAccessManager* networkManager );
    QNetworkReply* makeVectorBlobIdRequest( const QString&          baseUrl,
                                            const SumoCaseId&       caseId,
                                            const QString&          ensembleName,
                                            const QString&          vectorName,
                                            QNetworkAccessManager* networkManager );
    static QString blobIdFromReply( QNetworkReply* reply, const QString& vectorName );
    static QString logBlobId( const QString& blobId, const QString& vectorName );

    static std::vector<QString> parseVectorNames( const QByteArray& body );

private:
    RiaSumoConnector& m_connector;
};
