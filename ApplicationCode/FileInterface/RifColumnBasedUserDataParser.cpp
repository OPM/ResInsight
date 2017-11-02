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

#include "RifColumnBasedUserDataParser.h"

#include "RifEclipseUserDataParserTools.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"

#include "cvfAssert.h"

#include <QString>
#include <QStringList>
#include <QTextStream>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifColumnBasedUserDataParser::RifColumnBasedUserDataParser(const QString& data)
{
    parseTableData(data);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<TableData>& RifColumnBasedUserDataParser::tableData() const
{
    return m_tableDatas;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ColumnInfo* RifColumnBasedUserDataParser::columnInfo(size_t tableIndex, size_t columnIndex) const
{
    if (tableIndex >= m_tableDatas.size()) return nullptr;

    if (columnIndex >= m_tableDatas[tableIndex].columnInfos().size()) return nullptr;

    return &(m_tableDatas[tableIndex].columnInfos()[columnIndex]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifColumnBasedUserDataParser::parseTableData(const QString& data)
{
    std::stringstream streamData;
    streamData.str(data.toStdString());

    do
    {
        auto table = RifEclipseUserDataParserTools::tableDataFromText(streamData);
        std::vector<ColumnInfo>& columnInfos = table.columnInfos();
        int columnCount = static_cast<int>(columnInfos.size());
        if (columnCount == 0) break;

        std::string line;
        std::getline(streamData, line);

        do
        {
            QString qLine = QString::fromStdString(line);
            QStringList entries = qLine.split(" ", QString::SkipEmptyParts);

            if (entries.size() < columnCount) break;

            for (int i = 0; i < columnCount; i++)
            {
                if (columnInfos[i].isStringData)
                {
                    columnInfos[i].stringValues.push_back(entries[i].toStdString());
                }
                else
                {
                    double entry = entries[i].toDouble();
                    columnInfos[i].values.push_back(entry);
                }
            }
        } while (std::getline(streamData, line));

        m_tableDatas.push_back(table);

    } while (streamData.good());
}
