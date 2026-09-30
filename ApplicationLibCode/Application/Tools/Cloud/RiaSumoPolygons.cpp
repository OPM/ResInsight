/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaSumoPolygons.h"

#include "RiaLogging.h"
#include "RiaSumoConnector.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

#include <format>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoPolygons::RiaSumoPolygons( RiaSumoConnector& connector )
    : m_connector( connector )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SumoPolygonDirectory RiaSumoPolygons::polygonResultDirectory( const SumoCaseId& caseId, const QString& ensembleName )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );
    const QString path = QString( "/cases/%1/ensembles/%2/polygon_result_directory" ).arg( caseId.get() ).arg( encodedEnsembleName );

    return parseDirectory( m_connector.getBlocking( path, "Loading polygon result directory from Sumo" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SumoPolygonData> RiaSumoPolygons::polygonsData( const SumoCaseId& caseId,
                                                            const QString&    ensembleName,
                                                            int               realization,
                                                            SumoPolygonResult polygonResult,
                                                            const QString&    name,
                                                            const QString&    contactType )
{
    const QString encodedEnsembleName = QUrl::toPercentEncoding( ensembleName );

    QUrlQuery query;
    query.addQueryItem( "realization", QString::number( realization ) );
    query.addQueryItem( "polygon_result", polygonResultKey( polygonResult ) );
    if ( polygonResult != SumoPolygonResult::FieldOutline && !name.isEmpty() )
    {
        query.addQueryItem( "name", name );
    }
    if ( polygonResult == SumoPolygonResult::FluidContactOutline && !contactType.isEmpty() )
    {
        query.addQueryItem( "contact_type", contactType );
    }

    const QString path =
        QString( "/cases/%1/ensembles/%2/polygons_data?%3" ).arg( caseId.get() ).arg( encodedEnsembleName ).arg( query.toString( QUrl::FullyEncoded ) );

    return parsePolygonsData( m_connector.getBlocking( path, "Loading polygon data from Sumo" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSumoPolygons::polygonResultKey( SumoPolygonResult polygonResult )
{
    switch ( polygonResult )
    {
        case SumoPolygonResult::FieldOutline:
            return "field_outline";
        case SumoPolygonResult::StructureDepthFaultLines:
            return "structure_depth_fault_lines";
        case SumoPolygonResult::FluidContactOutline:
            return "fluid_contact_outline";
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Parses the polygon_result_directory response: a JSON object keyed by category, each value a list of
/// { name } or { name, contactType } objects. A category with no matches is omitted by the API.
//--------------------------------------------------------------------------------------------------
SumoPolygonDirectory RiaSumoPolygons::parseDirectory( const QByteArray& body )
{
    SumoPolygonDirectory directory;

    QJsonDocument doc = QJsonDocument::fromJson( body );
    QJsonObject   obj = doc.object();

    auto parseMetaArray = []( const QJsonArray& array )
    {
        std::vector<SumoPolygonMeta> result;
        for ( const QJsonValue& value : array )
        {
            QJsonObject metaObj = value.toObject();
            result.push_back( SumoPolygonMeta{ metaObj["name"].toString(), metaObj["contactType"].toString() } );
        }
        return result;
    };

    directory.fieldOutline             = parseMetaArray( obj[polygonResultKey( SumoPolygonResult::FieldOutline )].toArray() );
    directory.structureDepthFaultLines = parseMetaArray( obj[polygonResultKey( SumoPolygonResult::StructureDepthFaultLines )].toArray() );
    directory.fluidContactOutline      = parseMetaArray( obj[polygonResultKey( SumoPolygonResult::FluidContactOutline )].toArray() );

    return directory;
}

//--------------------------------------------------------------------------------------------------
/// Parses the polygons_data response: a JSON array of { xUtmEArr, yUtmNArray, zTvdSSArray, polyId, name }.
//--------------------------------------------------------------------------------------------------
std::vector<SumoPolygonData> RiaSumoPolygons::parsePolygonsData( const QByteArray& body )
{
    std::vector<SumoPolygonData> polygonDataList;

    QJsonDocument doc       = QJsonDocument::fromJson( body );
    QJsonArray    jsonArray = doc.array();

    for ( const QJsonValue& value : jsonArray )
    {
        QJsonObject obj = value.toObject();

        SumoPolygonData data;
        data.name = obj["name"].toString();

        // polyId can be an int or a string on the wire; keep it as a string either way.
        const QJsonValue polyIdValue = obj["polyId"];
        data.polyId                  = polyIdValue.isString() ? polyIdValue.toString() : QString::number( polyIdValue.toInt() );

        for ( const QJsonValue& x : obj["xUtmEArr"].toArray() )
            data.xArr.push_back( x.toDouble() );
        for ( const QJsonValue& y : obj["yUtmNArray"].toArray() )
            data.yArr.push_back( y.toDouble() );
        for ( const QJsonValue& z : obj["zTvdSSArray"].toArray() )
            data.zArr.push_back( z.toDouble() );

        polygonDataList.push_back( data );
    }

    RiaLogging::debug( std::format( "Polygon data count : {}", polygonDataList.size() ) );

    return polygonDataList;
}
