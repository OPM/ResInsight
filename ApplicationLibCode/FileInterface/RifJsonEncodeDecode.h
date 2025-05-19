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

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>

// Encapsulate the JSON code in a namespace to avoid issues with JSON classes used in opm-parser
namespace ResInsightInternalJson
{
class JsonReader
{
public:
    static QMap<QString, QVariant> decodeFile( QString filePath );

    // Get a variant list from a map
    static QVariantList getVariantList( const QMap<QString, QVariant>& map );

    static QString rootKeyText();

private:
    JsonReader() {};
};

class JsonWriter
{
public:
    static bool encodeFile( QString filePath, QMap<QString, QVariant> map );

private:
    JsonWriter() {};
};

class Json
{
public:
    static QString                 encode( const QMap<QString, QVariant>& map, bool prettify );
    static QMap<QString, QVariant> decode( const QString& jsonStr );
    static QMap<QString, QVariant> decode( const QByteArray& byteArray );
};

} // namespace ResInsightInternalJson
