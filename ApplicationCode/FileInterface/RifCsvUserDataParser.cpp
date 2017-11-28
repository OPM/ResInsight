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

#include "../Commands/SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "cvfAssert.h"

#include <QString>
#include <QTextStream>
#include <QFile>

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserDataParser::RifCsvUserDataParser(QString* errorText) :
    m_errorText(errorText)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserDataParser::~RifCsvUserDataParser()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parse(const AsciiDataParseOptions& parseOptions)
{
    return parseData(parseOptions);
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
const ColumnInfo* RifCsvUserDataParser::dateTimeColumn() const
{
    for (const ColumnInfo& col : m_tableData.columnInfos())
    {
        if (col.dataType == ColumnInfo::DATETIME)
        {
            return &col;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parseColumnInfo(const QString& cellSeparator)
{
    QTextStream* dataStream = openDataStream();
    std::vector<ColumnInfo> columnInfoList;
    bool result = parseColumnInfo(dataStream, cellSeparator, &columnInfoList);

    if (result)
    {
        m_tableData = TableData("", "", columnInfoList);
    }
    closeDataStream();
    return result;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifCsvUserDataParser::previewText(int lineCount, const QString& cellSeparator)
{
    QTextStream *stream = openDataStream();

    if (!stream) return "";

    QString preview;
    QTextStream outStream(&preview);
    int iLine = 0;

    while (iLine < lineCount && !stream->atEnd())
    {
        QString line = stream->readLine();

        if (line.isEmpty()) continue;

        outStream << line;
        outStream << "\n";
        iLine++;
    }
    closeDataStream();
    return columnifyText(preview, cellSeparator);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parseColumnInfo(QTextStream* dataStream, const QString& cellSeparator, std::vector<ColumnInfo>* columnInfoList)
{
    bool headerFound = false;

    if (!columnInfoList) return false;

    columnInfoList->clear();
    while (!headerFound)
    {
        QString line = dataStream->readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList lineColumns = splitLineAndTrim(line, cellSeparator);

        int colCount = lineColumns.size();

        for (int iCol = 0; iCol < colCount; iCol++)
        {
            QString colName = lineColumns[iCol];
            RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::importedAddress(colName.toStdString());
            ColumnInfo col = ColumnInfo::createColumnInfoFromCsvData(addr, "");

            columnInfoList->push_back(col);
        }
        headerFound = true;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parseData(const AsciiDataParseOptions& parseOptions)
{
    bool errors = false;
    enum { FIRST_DATA_ROW, DATA_ROW } parseState = FIRST_DATA_ROW;
    int colCount;
    std::vector<ColumnInfo> columnInfoList;

    QTextStream* dataStream = openDataStream();

    // Parse header
    if (!parseColumnInfo(dataStream, parseOptions.cellSeparator, &columnInfoList))
    {
        m_errorText->append("CSV import: Failed to parse header columns");
        return false;
    }

    colCount = (int)columnInfoList.size();
    while (!dataStream->atEnd() && !errors)
    {
        QString line = dataStream->readLine();
        if(line.trimmed().isEmpty()) continue;

        QStringList lineColumns = splitLineAndTrim(line, parseOptions.cellSeparator);

        if(lineColumns.size() != colCount)
        {
            m_errorText->append("CSV import: Varying number of columns");
            errors = true;
            break;
        }
        else if(parseState == FIRST_DATA_ROW)
        {
            for (int iCol = 0; iCol < colCount; iCol++)
            {
                std::string colData = lineColumns[iCol].toStdString();
                ColumnInfo& col = columnInfoList[iCol];

                // Determine column data type
                if (col.dataType == ColumnInfo::NONE)
                {
                    if (QString::fromStdString(col.summaryAddress.quantityName()) == parseOptions.timeSeriesColumnName)
                    {
                        col.dataType = ColumnInfo::DATETIME;
                    }
                    else
                    {
                        if (RiaStdStringTools::isNumber(colData, parseOptions.locale.decimalPoint().toAscii()))
                        {
                            col.dataType = ColumnInfo::NUMERIC;
                        }
                        else
                        {
                            col.dataType = ColumnInfo::TEXT;
                        }
                    }
                }
            }
            
            parseState = DATA_ROW;
        }
                
        if (parseState == DATA_ROW)
        {
            for (int iCol = 0; iCol < colCount; iCol++)
            {
                QString& colData = lineColumns[iCol];
                ColumnInfo& col = columnInfoList[iCol];

                try
                {
                    if (col.dataType == ColumnInfo::NUMERIC)
                    {
                        bool parseOk = true;
                        double value = parseOptions.locale.toDouble(colData, &parseOk);

                        if (!parseOk)
                        {
                            // Find the error reason, wrong decimal sign or something else
                            if (RiaStdStringTools::isNumber(colData.toStdString(), '.') || RiaStdStringTools::isNumber(colData.toStdString(), ','))
                            {
                                m_errorText->append(QString("CSV import: Failed to parse numeric value in column %1\n").arg(QString::number(iCol + 1)));
                                throw 0;
                            }

                            // Add NULL value
                            value = HUGE_VAL;
                        }
                        col.values.push_back(value);
                    }
                    else if (col.dataType == ColumnInfo::TEXT)
                    {
                        col.textValues.push_back(colData.toStdString());
                    }
                    else if (col.dataType == ColumnInfo::DATETIME)
                    {
                        QDateTime dt;
                        dt = tryParseDateTime(colData.toStdString(), parseOptions.dateTimeFormat);

                        if (!dt.isValid() && !parseOptions.useCustomDateTimeFormat)
                        {
                            // Try to match date format only
                            dt = tryParseDateTime(colData.toStdString(), parseOptions.dateFormat);
                        }

                        if (!dt.isValid())
                        {
                            m_errorText->append("CSV import: Failed to parse date time value");
                            throw 0;
                        }
                        col.dateTimeValues.push_back(dt);
                    }
                }
                catch (...)
                {
                    errors = true;
                    break;
                }
            }
        }
    }

    closeDataStream();

    if (!errors)
    {
        TableData td("", "", columnInfoList);
        m_tableData = td;
    }
    return !errors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifCsvUserDataParser::columnifyText(const QString& text, const QString& cellSeparator)
{
    QString pretty = text;

    if (!cellSeparator.isEmpty())
    {
        if (cellSeparator == ";" || cellSeparator == ",")
        {
            pretty = pretty.replace(cellSeparator, QString("\t") + cellSeparator);
        }
    }

    return pretty;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifCsvUserDataParser::tryDetermineCellSeparator()
{
    QTextStream* dataStream = openDataStream();
    std::vector<QString> lines;
    int iLine = 0;

    while(iLine < 10 && !dataStream->atEnd())
    {
        QString line = dataStream->readLine();
        if(line.isEmpty()) continue;

        lines.push_back(line);
        iLine++;
    }
    closeDataStream();

    // Try different cell separators
    int totColumnCountTab = 0;
    int totColumnCountSemicolon = 0;
    int totColumnCountComma = 0;

    for (const QString& line : lines)
    {
        totColumnCountTab       += splitLineAndTrim(line, "\t").size();
        totColumnCountSemicolon += splitLineAndTrim(line, ";").size();
        totColumnCountComma     += splitLineAndTrim(line, ",").size();
    }

    double avgColumnCountTab        = (double)totColumnCountTab / lines.size();
    double avgColumnCountSemicolon  = (double)totColumnCountSemicolon / lines.size();
    double avgColumnCountComma      = (double)totColumnCountComma / lines.size();

    // Select the one having highest average
    double maxAvg = std::max(std::max(avgColumnCountTab, avgColumnCountSemicolon), avgColumnCountComma);

    if (maxAvg == avgColumnCountTab)         return "\t";
    if (maxAvg == avgColumnCountSemicolon)   return ";";
    if (maxAvg == avgColumnCountComma)       return ",";
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifCsvUserDataParser::tryDetermineDecimalSeparator(const QString& cellSeparator)
{
    QTextStream* dataStream = openDataStream();
    std::vector<QString> lines;
    int iLine = 0;

    int successfulParsesDot = 0;
    int successfulParsesComma = 0;

    while (iLine < 10 && !dataStream->atEnd())
    {
        QString line = dataStream->readLine();
        if (line.isEmpty()) continue;

        for (QString cellData : splitLineAndTrim(line, cellSeparator))
        {
            bool parseOk;
            QLocale locale;

            locale = localeFromDecimalSeparator(".");
            locale.toDouble(cellData, &parseOk);
            if (parseOk) successfulParsesDot++;

            locale = localeFromDecimalSeparator(",");
            locale.toDouble(cellData, &parseOk);
            if (parseOk) successfulParsesComma++;
        }
    }
    closeDataStream();

    if (successfulParsesComma > successfulParsesDot)    return ",";
    else                                                return ".";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QLocale RifCsvUserDataParser::localeFromDecimalSeparator(const QString& decimalSeparator)
{
    if (decimalSeparator == ",")
    {
        return QLocale::Norwegian;
    }
    return QLocale::c();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserDataFileParser::RifCsvUserDataFileParser(const QString& fileName, QString* errorText) :
    RifCsvUserDataParser(errorText)
{
    m_fileName = fileName;
    m_file = nullptr;
    m_textStream = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserDataFileParser::~RifCsvUserDataFileParser()
{
    if (m_textStream)
    {
        delete m_textStream;
    }
    closeFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream* RifCsvUserDataFileParser::openDataStream()
{
    if (!openFile()) return nullptr;

    m_textStream = new QTextStream(m_file);
    return m_textStream;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifCsvUserDataFileParser::closeDataStream()
{
    if (m_textStream)
    {
        delete m_textStream;
        m_textStream = nullptr;
    }
    closeFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataFileParser::openFile()
{
    if (!m_file)
    {
        m_file = new QFile(m_fileName);
        if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            RiaLogging::error(QString("Failed to open %1").arg(m_fileName));

            delete m_file;
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifCsvUserDataFileParser::closeFile()
{
    if (m_file)
    {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserDataPastedTextParser::RifCsvUserDataPastedTextParser(const QString& text, QString* errorText):
    RifCsvUserDataParser(errorText)
{
    m_text = text;
    m_textStream = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserDataPastedTextParser::~RifCsvUserDataPastedTextParser()
{
    if (m_textStream)
    {
        delete m_textStream;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream* RifCsvUserDataPastedTextParser::openDataStream()
{
    if (m_textStream)
    {
        delete m_textStream;
    }
    m_textStream = new QTextStream(&m_text);
    return m_textStream;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifCsvUserDataPastedTextParser::closeDataStream()
{
    if (m_textStream)
    {
        delete m_textStream;
        m_textStream = nullptr;
    }
}