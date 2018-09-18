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

// Json parser based on code example found on:
// http://stackoverflow.com/questions/4169988/easiest-way-to-parse-json-in-qt-4-7

//#define IMPL_DUMP_TO_FILE

#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtScript/QScriptValue>

#if IMPL_DUMP_TO_FILE
#include <vector>
#include <cvfVector3.h>
#endif

class QScriptEngine;

// Encapsulate the JSON code in a namespace to avoid issues with JSON classes used in opm-parser
namespace ResInsightInternalJson {

class JsonReader
{
public:
    QMap<QString, QVariant> decodeFile(QString filePath);

#if IMPL_DUMP_TO_FILE
    void dumpToFile(std::vector<cvf::Vec3d>& points, QString filePath);
#endif
};

class Json
{
public:
    Json() {};
    QString encode(const QMap<QString, QVariant>& map, bool prettify);
    QMap<QString, QVariant> decode(const QString &jsonStr);

private:
    QScriptValue encodeInner(const QMap<QString,QVariant> &map, QScriptEngine* engine);
    QMap<QString, QVariant> decodeInner(QScriptValue object);
    QList<QVariant> decodeInnerToList(QScriptValue arrayValue);
};


} // end ResInsightInternalJson