/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2013 Statoil ASA, Ceetron AS
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

#include "RifJsonEncodeDecode.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtCore/QTextStream>

namespace ResInsightInternalJson
{
QMap<QString, QVariant> JsonReader::decodeFile( QString filePath )
{
    QFile file;
    file.setFileName( filePath );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray byteArray = file.readAll();
        file.close();
        return Json::decode( byteArray );
    }

    return QMap<QString, QVariant>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariantList JsonReader::getVariantList( const QMap<QString, QVariant>& map )
{
    // Assume a "root" key with a variant list
    return map[JsonReader::rootKeyText()].toList();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString JsonReader::rootKeyText()
{
    return "root";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool JsonWriter::encodeFile( QString filePath, QMap<QString, QVariant> map )
{
    QFile file;
    file.setFileName( filePath );
    if ( file.open( QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate ) )
    {
        QString     content = Json::encode( map, true );
        QTextStream out( &file );
        out << content;
        file.close();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Json::encode( const QMap<QString, QVariant>& map, bool prettify )
{
    QJsonDocument doc( QJsonObject::fromVariantMap( map ) );

    QJsonDocument::JsonFormat format = prettify ? QJsonDocument::JsonFormat::Indented : QJsonDocument::JsonFormat::Compact;
    return doc.toJson( format );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Json::encode( const QVariantList& list, bool prettify )
{
    QJsonDocument doc( QJsonArray::fromVariantList( list ) );

    QJsonDocument::JsonFormat format = prettify ? QJsonDocument::JsonFormat::Indented : QJsonDocument::JsonFormat::Compact;
    return doc.toJson( format );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMap<QString, QVariant> Json::decode( const QString& jsonStr )
{
    return Json::decode( jsonStr.toUtf8() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMap<QString, QVariant> Json::decode( const QByteArray& byteArray )
{
    auto document = QJsonDocument::fromJson( byteArray );
    if ( document.isObject() ) return QJsonDocument::fromJson( byteArray ).object().toVariantMap();

    // Return a variant map with "root" as key and a variant list if the document contains an array
    if ( document.isArray() )
    {
        auto variantList = document.array().toVariantList();

        QMap<QString, QVariant> map;

        map[JsonReader::rootKeyText()] = variantList;
        return map;
    }

    return {};
}

} // namespace ResInsightInternalJson
