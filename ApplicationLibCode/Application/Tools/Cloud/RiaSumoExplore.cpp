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

#include "RiaSumoExplore.h"

#include "RiaLogging.h"
#include "RiaQStringFormatter.h"
#include "RiaSumoConnector.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoExplore::RiaSumoExplore( RiaSumoConnector& connector )
    : m_connector( connector )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoAsset> RiaSumoExplore::assets()
{
    const QString url = QString( "%1/assets" ).arg( m_connector.server() );

    return parseAssets( m_connector.getBlocking( url, "Loading assets from Sumo" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoCase> RiaSumoExplore::cases( const QString& assetName )
{
    const QString url = QString( "%1/cases?asset_name=%2" ).arg( m_connector.server() ).arg( QString( QUrl::toPercentEncoding( assetName ) ) );

    return parseCases( m_connector.getBlocking( url, QString( "Loading the cases of %1 from Sumo" ).arg( assetName ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoExplore::ensembleNames( const SumoCaseId& caseId )
{
    const QString url = QString( "%1/cases/%2/ensembles" ).arg( m_connector.server() ).arg( caseId.get() );

    return parseEnsembleNames( m_connector.getBlocking( url, "Loading ensembles from Sumo" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoExplore::realizationIds( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    const QString url =
        QString( "%1/cases/%2/ensembles/%3/realizations" ).arg( m_connector.server() ).arg( caseId.get() ).arg( encodedEnsembleName );

    return parseRealizationIds( m_connector.getBlocking( url, "Loading realizations from Sumo" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoAsset> RiaSumoExplore::parseAssets( const QByteArray& body )
{
    std::vector<SumoAsset> assets;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    // This json is an array of AssetInfo
    for ( const QJsonValue& assetInfo : jsonArray )
    {
        QString assetName = assetInfo["name"].toString();
        assets.push_back( SumoAsset{ SumoAssetId( "" ), "", assetName } );
    }

    for ( const auto& asset : assets )
    {
        RiaLogging::debug( std::format( "Asset: {}", asset.name ) );
    }

    return assets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoCase> RiaSumoExplore::parseCases( const QByteArray& body )
{
    std::vector<SumoCase> cases;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        QJsonObject caseObj = value.toObject();

        QString id   = caseObj["id"].toString();
        QString kind = "";
        QString name = caseObj["name"].toString();
        cases.push_back( SumoCase{ SumoCaseId( id ), kind, name } );
    }

    RiaLogging::debug( std::format( "Case count : {}", cases.size() ) );

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoExplore::parseEnsembleNames( const QByteArray& body )
{
    std::vector<QString> ensembleNames;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        QJsonObject ensembleObj = value.toObject();
        ensembleNames.push_back( ensembleObj["name"].toString() );
    }

    RiaLogging::debug( std::format( "Ensemble count : {}", ensembleNames.size() ) );

    return ensembleNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSumoExplore::parseRealizationIds( const QByteArray& body )
{
    std::vector<QString> realizationIds;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        realizationIds.push_back( QString::number( value.toInt() ) );
    }

    return realizationIds;
}
