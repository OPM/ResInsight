/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-  Statoil ASA
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

#include "RifColumnBasedAsciiParser.h"

#include "RiaLogging.h"

#include "cvfAssert.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifColumnBasedAsciiParser::RifColumnBasedAsciiParser(QString& data, const QString dateFormat, QLocale decimalLocale, QString cellSeparator)
{
    parseData(data, dateFormat, decimalLocale, cellSeparator);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<QString>& RifColumnBasedAsciiParser::headers() const
{
    return m_data.m_headers;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<QDateTime>& RifColumnBasedAsciiParser::timeSteps() const
{
    return m_data.m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifColumnBasedAsciiParser::columnValues(size_t columnIndex) const
{
    CVF_TIGHT_ASSERT(columnIndex < m_data.m_values.size());

    return m_data.m_values[columnIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifColumnBasedAsciiParser::parseData(QString& data, QString dateFormat, QLocale decimalLocale, QString cellSeparator)
{
    QTextStream tableData(&data);

    QString header;

    do {
        header = tableData.readLine();
    } while (header.isEmpty() && !tableData.atEnd());

    // No header row found
    if (header.isEmpty()) return;

    QStringList columnHeaders = header.split(cellSeparator);

    for (int i = 1; i < columnHeaders.size(); ++i)
    {
        m_data.m_headers.push_back(columnHeaders[i]);
    }

    // No columns found
    if (m_data.m_headers.empty()) return;


    int numColumns = static_cast<int>(m_data.m_headers.size());

    m_data.m_values.resize(numColumns);

    size_t row = 0;
    while (!tableData.atEnd())
    {
        ++row;
        QString line = tableData.readLine();

        // Skip empty lines
        if (line.isEmpty()) continue;

        QStringList columns = line.split(cellSeparator);

        if (columns.size() != numColumns + 1)
        {
            RiaLogging::warning(QString("Invalid number of columns in row %1").arg(row));
            continue;
        }

        QDateTime date = QDateTime::fromString(columns[0], dateFormat);
        if (!date.isValid())
        {
            RiaLogging::warning(QString("First column of row %1 could not be parsed as a date: %2").arg(row).arg(columns[0]));
            continue;
        }
        m_data.m_timeSteps.push_back(date);

        for (int col = 1; col < columns.size(); ++col)
        {
            bool ok;
            m_data.m_values[col - 1].push_back(decimalLocale.toDouble(columns[col], &ok));
            if (!ok)
            {
                RiaLogging::warning(QString("Could not parse value at row %1 column %2 as double: %3. Defaulting to 0.0").arg(row).arg(col).arg(columns[col]));
            }
        }
    }
}
