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

#include "RifCsvUserDataParser.h"

#include "RifEclipseUserDataKeywordTools.h"
#include "RifEclipseUserDataParserTools.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RiaQDateTimeTools.h"

#include "cvfAssert.h"

#include <QString>
#include <QTextStream>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserDataParser::RifCsvUserDataParser(QString* errorText)
    : m_errorText(errorText)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parse(const QString& data, const AsciiDataParseOptions& parseOptions)
{
    return parseData(data, parseOptions);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const TableData& RifCsvUserDataParser::tableData() const
{
    return m_tableData;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ColumnInfo* RifCsvUserDataParser::columnInfo(size_t columnIndex) const
{
    if (columnIndex >= m_tableData.columnInfos().size()) return nullptr;

    return &(m_tableData.columnInfos()[columnIndex]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parseData(const QString& data, const AsciiDataParseOptions& parseOptions)
{
    enum { HEADER_ROW, FIRST_DATA_ROW, DATA_ROW } parseState = HEADER_ROW;
    int colCount = -1;
    std::vector<ColumnInfo> cols;

    QTextStream dataStream(const_cast<QString*>(&data));
    while (!dataStream.atEnd())
    {
        QString line = dataStream.readLine();
        if(line.trimmed().isEmpty()) continue;

        QStringList lineColumns = splitLineAndTrim(line, parseOptions.cellSeparator);

        if (parseState == HEADER_ROW)
        {
            colCount = lineColumns.size();

            for (int iCol = 0; iCol < colCount; iCol++)
            {
                std::string colName = lineColumns[iCol].toStdString();
                RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::importedAddress(colName);
                ColumnInfo col = ColumnInfo::createColumnInfoFromCsvData(addr, "");
                cols.push_back(col);
            }

            parseState = FIRST_DATA_ROW;
        }
        else if(lineColumns.size() != colCount)
        {
            m_errorText->append("CSV file has invalid content (Column count mismatch)");
            return false;
        }
        else if(parseState == FIRST_DATA_ROW)
        {
            for (int iCol = 0; iCol < colCount; iCol++)
            {
                std::string colData = lineColumns[iCol].toStdString();
                ColumnInfo& col = cols[iCol];

                // Check if text column
                if (RiaStdStringTools::isNumber(colData))
                {
                    col.dataType = ColumnInfo::NUMERIC;
                }
                else if (tryParseDateTime(colData, parseOptions.dateTimeFormat()).isValid() ||
                         tryParseDateTime(colData, parseOptions.dateFormat_).isValid())
                {
                    col.dataType = ColumnInfo::DATETIME;
                }
                else
                {
                    col.dataType = ColumnInfo::TEXT;
                }
            }
            
            parseState = DATA_ROW;
        }
                
        if (parseState == DATA_ROW)
        {
            for (int iCol = 0; iCol < colCount; iCol++)
            {
                std::string colData = lineColumns[iCol].toStdString();
                ColumnInfo& col = cols[iCol];

                try
                {
                    if (col.dataType == ColumnInfo::NUMERIC)
                    {
                        col.values.push_back(RiaStdStringTools::toDouble(colData));
                    }
                    else if (col.dataType == ColumnInfo::TEXT)
                    {
                        col.textValues.push_back(colData);
                    }
                    else if (col.dataType == ColumnInfo::DATETIME)
                    {
                        QDateTime dt = tryParseDateTime(colData, parseOptions.dateTimeFormat());
                        if (!dt.isValid())
                        {
                            dt = tryParseDateTime(colData, parseOptions.dateFormat_);
                        }
                        if (!dt.isValid()) throw 0;
                        col.dateTimeValues.push_back(dt);
                    }
                }
                catch (...)
                {
                    m_errorText->append("CSV file has invalid content (Column type mismatch)");
                    return false;
                }
            }
        }
    }

    TableData td("", "", cols);
    m_tableData = td;
    return true;

    //std::string stdData = data.toStdString();
    //bool isFixedWidth = RifEclipseUserDataParserTools::isFixedWidthHeader(stdData);

    //std::stringstream streamData;
    //streamData.str(stdData);

    //std::vector<TableData> rawTables;

    //do
    //{
    //    std::vector<std::string> errorStrings;

    //    TableData table;

    //    if (isFixedWidth)
    //    {
    //        auto columnInfos = RifEclipseUserDataParserTools::columnInfoForFixedColumnWidth(streamData);
    //        table = TableData("", "", "", columnInfos);
    //    }
    //    else
    //    {
    //        table = RifEclipseUserDataParserTools::tableDataFromText(streamData, &errorStrings);
    //    }

    //    if (m_errorText)
    //    {
    //        for (auto s : errorStrings)
    //        {
    //            QString errorText = QString("\n%1").arg(QString::fromStdString(s));
    //            m_errorText->append(errorText);
    //        }
    //    }

    //    std::vector<ColumnInfo>& columnInfos = table.columnInfos();
    //    int columnCount = static_cast<int>(columnInfos.size());
    //    if (columnCount == 0) break;

    //    int stepTypeIndex = -1;
    //    for (size_t i = 0; i < columnInfos.size(); i++)
    //    {
    //        if (RifEclipseUserDataKeywordTools::isStepType(columnInfos[i].summaryAddress.quantityName()))
    //        {
    //            stepTypeIndex = static_cast<int>(i);
    //        }
    //    }

    //    std::string line;
    //    std::getline(streamData, line);

    //    do
    //    {
    //        QString qLine = QString::fromStdString(line);
    //        QStringList entries = qLine.split(" ", QString::SkipEmptyParts);

    //        if (stepTypeIndex > -1 &&
    //            (unsigned int)entries.size() < columnInfos.size())
    //        {
    //            entries.insert(stepTypeIndex, " ");
    //        }

    //        if (entries.size() < columnCount) break;

    //        for (int i = 0; i < columnCount; i++)
    //        {
    //            if (columnInfos[i].dataType == ColumnInfo::NUMERIC)
    //            {
    //                double entry = entries[i].toDouble();
    //                columnInfos[i].values.push_back(entry);
    //            }
    //            else
    //            {
    //                columnInfos[i].textValues.push_back(entries[i].toStdString());
    //            }
    //        }
    //    } while (std::getline(streamData, line));

    //    rawTables.push_back(table);

    //} while (streamData.good());

    //m_tableDatas = RifEclipseUserDataParserTools::mergeEqualTimeSteps(rawTables);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RifCsvUserDataParser::splitLineAndTrim(const QString& line, const QString& separator)
{
    QStringList cols = line.split(separator);
    for (QString& col : cols)
    {
        col = col.trimmed();
    }
    return cols;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RifCsvUserDataParser::tryParseDateTime(const std::string& colData, const QString& format)
{
    return RiaQDateTimeTools::fromString(QString::fromStdString(colData), format);
}
