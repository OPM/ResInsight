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
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>

namespace ResInsightInternalJson
{
QMap<QString, QVariant> JsonReader::decodeFile( QString filePath )
{
    QFile file;
    file.setFileName( filePath );
    file.open( QIODevice::ReadOnly );
    QByteArray byteArray = file.readAll();
    file.close();
    return Json::decode( byteArray );
}

QString Json::encode( const QMap<QString, QVariant>& map, bool prettify )
{
    QJsonDocument doc( QJsonObject::fromVariantMap( map ) );

    QJsonDocument::JsonFormat format = prettify ? QJsonDocument::JsonFormat::Indented : QJsonDocument::JsonFormat::Compact;
    return doc.toJson( format );
}

QMap<QString, QVariant> Json::decode( const QString& jsonStr )
{
    return Json::decode( jsonStr.toUtf8() );
}

QMap<QString, QVariant> Json::decode( const QByteArray& byteArray )
{
    return QJsonDocument::fromJson( byteArray ).object().toVariantMap();
}

} // namespace ResInsightInternalJson
