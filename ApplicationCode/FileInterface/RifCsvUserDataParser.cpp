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
#include "RifFileParseTools.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RiaTextStringTools.h"
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
const Column* RifCsvUserDataParser::columnInfo(size_t columnIndex) const
{
    if (columnIndex >= m_tableData.columnInfos().size()) return nullptr;

    return &(m_tableData.columnInfos()[columnIndex]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Column* RifCsvUserDataParser::dateTimeColumn() const
{
    for (const Column& col : m_tableData.columnInfos())
    {
        if (col.dataType == Column::DATETIME)
        {
            return &col;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parseColumnInfo(const AsciiDataParseOptions& parseOptions)
{
    QTextStream* dataStream = openDataStream();
    std::vector<Column> columnInfoList;
    bool result = parseColumnInfo(dataStream, parseOptions, &columnInfoList);

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
QString RifCsvUserDataParser::previewText(int lineCount, const AsciiDataParseOptions& parseOptions)
{
    QTextStream *stream = openDataStream();

    if (!stream) return "";

    QString preview;
    QTextStream outStream(&preview);
    int iLine = 0;
    bool header = true;
    int timeColumnIndex = -1;

    outStream << "<Table>";
    outStream << "<Style> th, td {padding-right: 15px;} </Style>";
    while (iLine < lineCount && !stream->atEnd())
    {
        QString line = stream->readLine();

        if (line.isEmpty()) continue;

        outStream << "<tr>";
        int iCol = 0;
        QStringList cols = RifFileParseTools::splitLineAndTrim(line, parseOptions.cellSeparator);
        for (const QString& cellData : cols)
        {
            if (cellData == parseOptions.timeSeriesColumnName && header)
            {
                timeColumnIndex = iCol;
            }
            
            outStream << (header ? "<th" : "<td");

            if (iCol == timeColumnIndex)
            {
                outStream << " style=\"background-color: #FFFFD0;\"";
            }
            outStream << ">";
            outStream << cellData;
            if (iCol < cols.size() - 1 && (parseOptions.cellSeparator == ";" || parseOptions.cellSeparator == ","))
            {
                outStream << parseOptions.cellSeparator;
            }
            outStream << (header ? "</th>" : "</td>");

            iCol++;
        }
        outStream << "</tr>";

        header = false;
        iLine++;
    }

    outStream << "</Table>";

    closeDataStream();
    return preview;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parseColumnInfo(QTextStream* dataStream, const AsciiDataParseOptions& parseOptions, std::vector<Column>* columnInfoList)
{
    bool headerFound = false;

    if (!columnInfoList) return false;

    columnInfoList->clear();
    while (!headerFound)
    {
        QString line = dataStream->readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList lineColumns = RifFileParseTools::splitLineAndTrim(line, parseOptions.cellSeparator);

        int colCount = lineColumns.size();

        for (int iCol = 0; iCol < colCount; iCol++)
        {
            QString colName = RiaTextStringTools::trimAndRemoveDoubleSpaces(lineColumns[iCol]);
            RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(colName.toStdString());
            Column col = Column::createColumnInfoFromCsvData(addr, "");

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
    std::vector<Column> columnInfoList;

    QTextStream* dataStream = openDataStream();

    // Parse header
    if (!parseColumnInfo(dataStream, parseOptions, &columnInfoList))
    {
        if (m_errorText) m_errorText->append("CSV import: Failed to parse header columns");
        return false;
    }

    colCount = (int)columnInfoList.size();
    while (!dataStream->atEnd() && !errors)
    {
        QString line = dataStream->readLine();
        if(line.trimmed().isEmpty()) continue;

        QStringList lineColumns = RifFileParseTools::splitLineAndTrim(line, parseOptions.cellSeparator);

        if(lineColumns.size() != colCount)
        {
            if (m_errorText) m_errorText->append("CSV import: Varying number of columns");
            errors = true;
            break;
        }
        else if(parseState == FIRST_DATA_ROW)
        {
            for (int iCol = 0; iCol < colCount; iCol++)
            {
                std::string colData = lineColumns[iCol].toStdString();
                Column& col = columnInfoList[iCol];

                // Determine column data type
                if (col.dataType == Column::NONE)
                {
                    if (QString::fromStdString(col.summaryAddress.quantityName()) == parseOptions.timeSeriesColumnName)
                    {
                        col.dataType = Column::DATETIME;
                    }
                    else
                    {
                        if (parseOptions.assumeNumericDataColumns || RiaStdStringTools::isNumber(colData, parseOptions.locale.decimalPoint().toAscii()))
                        {
                            col.dataType = Column::NUMERIC;
                        }
                        else
                        {
                            col.dataType = Column::TEXT;
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
                Column& col = columnInfoList[iCol];

                try
                {
                    if (col.dataType == Column::NUMERIC)
                    {
                        bool parseOk = true;
                        double value = parseOptions.locale.toDouble(colData, &parseOk);

                        if (!parseOk)
                        {
                            // Find the error reason, wrong decimal sign or something else
                            if (RiaStdStringTools::isNumber(colData.toStdString(), '.') || RiaStdStringTools::isNumber(colData.toStdString(), ','))
                            {
                                if (m_errorText) m_errorText->append(QString("CSV import: Failed to parse numeric value in column %1\n").arg(QString::number(iCol + 1)));
                                throw 0;
                            }

                            // Add NULL value
                            value = HUGE_VAL;
                        }
                        col.values.push_back(value);
                    }
                    else if (col.dataType == Column::TEXT)
                    {
                        col.textValues.push_back(colData.toStdString());
                    }
                    else if (col.dataType == Column::DATETIME)
                    {
                        QDateTime dt;
                        dt = tryParseDateTime(colData.toStdString(), parseOptions.dateTimeFormat);

                        if (!dt.isValid() && !parseOptions.useCustomDateTimeFormat)
                        {
                            // Try to match date format only
                            if (parseOptions.dateFormat != parseOptions.dateTimeFormat)
                            {
                                dt = tryParseDateTime(colData.toStdString(), parseOptions.dateFormat);
                            }
                            if (!dt.isValid() && !parseOptions.fallbackDateTimeFormat.isEmpty())
                            {
                                dt = tryParseDateTime(colData.toStdString(), parseOptions.fallbackDateTimeFormat);
                            }
                        }

                        if (!dt.isValid())
                        {
                            if (m_errorText) m_errorText->append("CSV import: Failed to parse date time value");
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
        totColumnCountTab       += RifFileParseTools::splitLineAndTrim(line, "\t").size();
        totColumnCountSemicolon += RifFileParseTools::splitLineAndTrim(line, ";").size();
        totColumnCountComma     += RifFileParseTools::splitLineAndTrim(line, ",").size();
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

        for (const QString& cellData : RifFileParseTools::splitLineAndTrim(line, cellSeparator))
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