/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RiaQuantityInfoTools.h"

#include "RiaLogging.h"

#include "RiuSummaryQuantityNameInfoProvider.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

#include <unordered_map>

namespace RiaQuantityInfoTools::internal
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void writeToFile( const QString& filename, const std::unordered_map<std::string, std::pair<std::string, std::string>>& map )
{
    QJsonObject jsonObj;

    for ( const auto& item : map )
    {
        QJsonObject itemObj;
        itemObj["category"]                           = QString::fromStdString( item.second.first );
        itemObj["description"]                        = QString::fromStdString( item.second.second );
        jsonObj[QString::fromStdString( item.first )] = itemObj;
    }

    QJsonDocument jsonDoc( jsonObj );
    QFile         file( filename );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        RiaLogging::error( "Couldn't open file : " + filename );
        return;
    }
    file.write( jsonDoc.toJson() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unordered_map<std::string, std::pair<std::string, std::string>> importFromFile( const QString& filename )
{
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        RiaLogging::error( "Couldn't open file : " + filename );
        return {};
    }

    QByteArray    data = file.readAll();
    QJsonDocument jsonDoc( QJsonDocument::fromJson( data ) );

    if ( !jsonDoc.isObject() )
    {
        RiaLogging::error( "Invalid JSON format in : " + filename );
        return {};
    }

    std::unordered_map<std::string, std::pair<std::string, std::string>> map;

    QJsonObject jsonObj = jsonDoc.object();
    for ( auto it = jsonObj.begin(); it != jsonObj.end(); ++it )
    {
        auto        key   = it.key().toStdString();
        QJsonObject value = it.value().toObject();

        auto category    = value["category"].toString().toStdString();
        auto description = value["description"].toString().toStdString();

        map.insert( { key, { category, description } } );
    }

    return map;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void importKeywords( const QString& keywordEclipseFilePath, const QString& keyword6XFilePath )
{
    auto quantityInfos = internal::importFromFile( keywordEclipseFilePath );
    auto info6x        = internal::importFromFile( keyword6XFilePath );

    for ( const auto& other : info6x )
    {
        if ( !quantityInfos.contains( other.first ) )
        {
            quantityInfos.insert( other );
        }
    }

    RiuSummaryQuantityNameInfoProvider::instance()->setQuantityInfos( quantityInfos );
}

} // namespace RiaQuantityInfoTools::internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaQuantityInfoTools::initializeSummaryKeywords()
{
    QString keywordEclipseFilePath = ":keywords/keyword-description/keywords_eclipse.json";
    QString keyword6XFilePath      = ":keywords/keyword-description/keywords_6x.json";

    RiaQuantityInfoTools::internal::importKeywords( keywordEclipseFilePath, keyword6XFilePath );
}
