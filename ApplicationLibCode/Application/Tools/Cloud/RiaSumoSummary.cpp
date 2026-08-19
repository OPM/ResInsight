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

#include "RiaLogging.h"
#include "RiaSumoConnector.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

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
    const QString blobId = vectorBlobId( caseId, ensembleName, vectorName );
    if ( blobId.isEmpty() ) return {};

    return m_connector.downloadBlobBlocking( blobId );
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
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString encodedVectorName   = QUrl::toPercentEncoding( vectorName );

    const QString url = QString( "%1/cases/%2/ensembles/%3/vectors/%4/blob_id" )
                            .arg( m_connector.server() )
                            .arg( caseId.get() )
                            .arg( encodedEnsembleName )
                            .arg( encodedVectorName );

    const QString blobId = RiaSumoConnector::blobIdFromBody( m_connector.getBlocking( url ) );

    if ( !blobId.isEmpty() )
    {
        RiaLogging::debug( std::format( "Received blob ID for vector '{}': {}", vectorName.toStdString(), blobId.toStdString() ) );
    }

    return blobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoSummary::parameterBlobId( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    const QString url =
        QString( "%1/cases/%2/ensembles/%3/parameters/blob_id" ).arg( m_connector.server() ).arg( caseId.get() ).arg( encodedEnsembleName );

    const QString blobId = RiaSumoConnector::blobIdFromBody( m_connector.getBlocking( url ) );

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
