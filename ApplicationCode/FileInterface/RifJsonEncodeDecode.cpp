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

// Json parser based on code example found on:
// http://stackoverflow.com/questions/4169988/easiest-way-to-parse-json-in-qt-4-7

#include "RifJsonEncodeDecode.h"

#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>

namespace ResInsightInternalJson {

QMap<QString, QVariant> JsonReader::decodeFile(QString filePath)
{
    QFile file;
    file.setFileName(filePath);
    file.open(QIODevice::ReadOnly);
    QByteArray byteArray = file.readAll();
    file.close();
    QString jsonString(byteArray);
    Json json;
    return json.decode(jsonString);    
}


#if IMPL_DUMP_TO_FILE
void JsonReader::dumpToFile(std::vector<cvf::Vec3d>& points, QString filePath)
{
    QFile file;
    file.setFileName(filePath);
    file.open(QIODevice::WriteOnly);
    for (size_t idx = 0; idx < points.size(); idx++)
    {
        cvf::Vec3d point = points[idx];
        QString string;
        string.sprintf("(%0.10e, %0.10e, %0.10e)\n", point.x(), point.y(), point.z());
        QByteArray byteArray(string.toLatin1());
        file.write(byteArray);
    }
    file.close();
}
#endif


QString Json::encode(const QMap<QString,QVariant>& map, bool prettify)
{
    QScriptEngine engine;
    if (prettify)
    {
        engine.evaluate("function toString() { return JSON.stringify(this, null, '  ') }");
    }
    else
    {
        engine.evaluate("function toString() { return JSON.stringify(this) }");
    }

    QScriptValue toString = engine.globalObject().property("toString");
    QScriptValue obj = encodeInner(map, &engine);
    return toString.call(obj).toString();

}

QMap<QString, QVariant> Json::decode(const QString &jsonStr)
{
    QScriptValue object;
    QScriptEngine engine;
    object = engine.evaluate("(" + jsonStr + ")");
    return decodeInner(object);
}

QScriptValue Json::encodeInner(const QMap<QString,QVariant> &map, QScriptEngine* engine)
{
    QScriptValue obj = engine->newObject();
    QMapIterator<QString, QVariant> i(map);
    while (i.hasNext()) {
        i.next();
        if (i.value().type() == QVariant::String)
            obj.setProperty(i.key(), i.value().toString());
        else if (i.value().type() == QVariant::Int)
            obj.setProperty(i.key(), i.value().toInt());
        else if (i.value().type() == QVariant::Double)
            obj.setProperty(i.key(), i.value().toDouble());
        else if (i.value().type() == QVariant::List)
            obj.setProperty(i.key(), qScriptValueFromSequence(engine, i.value().toList()));
        else if (i.value().type() == QVariant::Map)
            obj.setProperty(i.key(), encodeInner(i.value().toMap(), engine));
    }
    return obj;
}


QMap<QString, QVariant> Json::decodeInner(QScriptValue object)
{
    QMap<QString, QVariant> map;
    QScriptValueIterator it(object);
    while (it.hasNext()) {
        it.next();
        if (it.value().isArray())
            map.insert(it.name(),QVariant(decodeInnerToList(it.value())));
        else if (it.value().isNumber())
            map.insert(it.name(),QVariant(it.value().toNumber()));
        else if (it.value().isString())
            map.insert(it.name(),QVariant(it.value().toString()));
        else if (it.value().isNull())
            map.insert(it.name(),QVariant());
        else if(it.value().isObject())
            map.insert(it.name(),QVariant(decodeInner(it.value())));
    }
    return map;
}

QList<QVariant> Json::decodeInnerToList(QScriptValue arrayValue)
{
    QList<QVariant> list;
    QScriptValueIterator it(arrayValue);
    while (it.hasNext()) {
        it.next();
        if (it.name() == "length")
            continue;

        if (it.value().isArray())
            list.append(QVariant(decodeInnerToList(it.value())));
        else if (it.value().isNumber())
            list.append(QVariant(it.value().toNumber()));
        else if (it.value().isString())
            list.append(QVariant(it.value().toString()));
        else if (it.value().isNull())
            list.append(QVariant());
        else if(it.value().isObject())
            list.append(QVariant(decodeInner(it.value())));
    }
    return list;
}

} // end ResInsightInternalJson