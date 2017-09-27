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

#include "RifColumnBasedRsmspecParser.h"

#include "RifRsmspecParserTools.h"

#include "RiaLogging.h"

#include "cvfAssert.h"

#include <QString>
#include <QStringList>
#include <QTextStream>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifColumnBasedRsmspecParser::RifColumnBasedRsmspecParser(const QString& data)
{
    parseData(data);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector< std::vector<ColumnInfo> >& RifColumnBasedRsmspecParser::tables() const
{
    return m_tables;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifColumnBasedRsmspecParser::parseData(const QString& data)
{
    std::stringstream streamData;
    streamData.str(data.toStdString());
    std::string line;
    std::getline(streamData, line);

    do 
    {
        std::vector<ColumnInfo> table = RifRsmspecParserTools::columnInfoForTable(streamData, line);
        size_t columnCount = table.size();
        if (columnCount == 0) break;

        std::vector<double> values;

        QString qLine;
        do
        {
            qLine = QString::fromStdString(line);
            QStringList entries = qLine.split(" ", QString::SkipEmptyParts);

            if (entries.size() < columnCount) break;

            for (size_t i = 0; i < columnCount; i++)
            {
                double entry = entries.at(static_cast<int>(i)).toDouble();
                table[i].values.push_back(entry);
            }
        } while (std::getline(streamData, line));

        m_tables.push_back(table);

    } while (streamData.good());
}
