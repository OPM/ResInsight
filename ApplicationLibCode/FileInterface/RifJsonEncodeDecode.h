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

//#define IMPL_DUMP_TO_FILE

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>

#if IMPL_DUMP_TO_FILE
#include <cvfVector3.h>
#include <vector>
#endif

// Encapsulate the JSON code in a namespace to avoid issues with JSON classes used in opm-parser
namespace ResInsightInternalJson
{
class JsonReader
{
public:
    QMap<QString, QVariant> decodeFile( QString filePath );

#if IMPL_DUMP_TO_FILE
    void dumpToFile( std::vector<cvf::Vec3d>& points, QString filePath );
#endif
};

class Json
{
public:
    static QString                 encode( const QMap<QString, QVariant>& map, bool prettify );
    static QMap<QString, QVariant> decode( const QString& jsonStr );
    static QMap<QString, QVariant> decode( const QByteArray& byteArray );
};

} // namespace ResInsightInternalJson
