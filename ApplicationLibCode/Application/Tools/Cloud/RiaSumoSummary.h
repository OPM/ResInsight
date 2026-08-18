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

#include <vector>

class RiaSumoConnector;

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

    // The ensemble parameters, as a parquet blob.
    QByteArray parameterData( const SumoCaseId& caseId, const QString& ensembleName );

    QString vectorBlobId( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );
    QString parameterBlobId( const SumoCaseId& caseId, const QString& ensembleName );

private:
    static std::vector<QString> parseVectorNames( const QByteArray& body );

private:
    RiaSumoConnector& m_connector;
};
