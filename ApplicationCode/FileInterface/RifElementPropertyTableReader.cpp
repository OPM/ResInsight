/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RifElementPropertyTableReader.h"

#include "RiaLogging.h"
#include "RiuMainWindow.h"

#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <cctype>
#include <string>


//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
static QFile* openFile(const QString &fileName);
static void closeFile(QFile *file);
static QStringList splitLineAndTrim(const QString& line, const QString& separator);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ElementPropertyMetadata RifElementPropertyTableReader::readMetadata(const QString& fileName)
{
    ElementPropertyMetadata metadata;
    QFile*                  file = openFile(fileName);

    if (file)
    {
        QTextStream     stream(file);
        bool            metadataBlockFound = false;
        int             maxLinesToRead = 50;
        int             lineNo = 0;

        while (lineNo < maxLinesToRead)
        {
            QString line = stream.readLine();
            lineNo++;

            if (line.toUpper().startsWith("*DISTRIBUTION TABLE"))
            {
                metadataBlockFound = true;
                continue;
            }

            if (!metadataBlockFound) continue;

            QStringList cols = splitLineAndTrim(line, ",");

            metadata.fileName = fileName;
            for (QString s : cols)
            {
                metadata.dataColumns.push_back(s);
            }
            break;
        }

        closeFile(file);
    }

    return metadata;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElementPropertyTableReader::readData(const ElementPropertyMetadata *metadata, ElementPropertyTable *table)
{
    CVF_ASSERT(metadata && table);

    QFile*  file = openFile(metadata->fileName);
    int     expectedColumnCount = (int)metadata->dataColumns.size() + 1;

    if (file && expectedColumnCount > 0)
    {
        QTextStream     stream(file);
        bool            dataBlockFound = false;
        int             lineNo = 0;

        // Init data vectors
        table->elementIds.clear();
        table->data = std::vector<std::vector<float>>(metadata->dataColumns.size());

        while (!stream.atEnd())
        {
            QString     line = stream.readLine();
            QStringList cols = splitLineAndTrim(line, ",");
            lineNo++;

            if (!dataBlockFound)
            {
                if (!line.startsWith("*") && cols.size() == expectedColumnCount) dataBlockFound = true;
                else continue;
            }

            if (cols.size() != expectedColumnCount)
            {
                throw FileParseException(QString("Number of columns mismatch at %1:%2").arg(metadata->fileName).arg(lineNo));
            }

            for (int c = 0; c < expectedColumnCount; c++)
            {
                bool parseOk;

                if (c == 0)
                {
                    // Remove elementId column prefix
                    QStringList parts = cols[0].split(".");

                    int elementId = parts.last().toInt(&parseOk);
                    if (!parseOk)
                    {
                        throw FileParseException(QString("Parse failed at %1:%2").arg(metadata->fileName).arg(lineNo));
                    }
                    table->elementIds.push_back(elementId);
                }
                else
                {
                    float value = cols[c].toFloat(&parseOk);
                    if (!parseOk)
                    {
                        throw FileParseException(QString("Parse failed at %1:%2").arg(metadata->fileName).arg(lineNo));
                    }
                    table->data[c - 1].push_back(value);
                }
            }
        }

        table->hasData = true;
    }

    closeFile(file);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QFile* openFile(const QString &fileName)
{
    QFile *file;
    file = new QFile(fileName);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        RiaLogging::error(QString("Failed to open %1").arg(fileName));

        delete file;
        return nullptr;
    }
    return file;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void closeFile(QFile *file)
{
    if (file)
    {
        file->close();
        delete file;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList splitLineAndTrim(const QString& line, const QString& separator)
{
    QStringList cols = line.split(separator);
    for (QString& col : cols)
    {
        col = col.trimmed();
    }
    return cols;
}
