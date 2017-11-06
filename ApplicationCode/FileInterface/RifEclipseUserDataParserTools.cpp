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

#include "RifEclipseUserDataParserTools.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "RifEclipseUserDataKeywordTools.h"

#include "cvfAssert.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <numeric>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::isLineSkippable(const std::string& line)
{
    std::size_t found = line.find_first_not_of(" ");
    if (found == std::string::npos)
    {
        // Line with only spaces

        return true;
    }

    if (line[found] == '-')
    {
        // Comments start with -

        return true;
    }

    if (line[found] == '1' &&
        found == 0 &&
        line.find_first_not_of("1 ", 1) == std::string::npos)
    {
        // Single 1 at start of file

        return true;
    }

    std::string str(line);

    if (str.find("SUMMARY") < str.size())
    {
        return true;
    }

    if (str.find("PAGE") < str.size())
    {
        return true;
    }

    if (str.find("NULL") < str.size())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::isAComment(const std::string& word)
{
    if (word.find("--") != std::string::npos)
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifEclipseUserDataParserTools::splitLineAndRemoveComments(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> words{ std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>{} };

    for(auto wordsIterator = words.begin(); wordsIterator != words.end(); ++wordsIterator)
    {
        if (isAComment(*wordsIterator))
        {
            words.erase(wordsIterator, words.end());
            break;
        }
    }

    return words;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory RifEclipseUserDataParserTools::identifyCategory(const std::string& word)
{
    if (word.size() == 0) return RifEclipseSummaryAddress::SUMMARY_INVALID;

    char firstLetter = word.at(0);

    if (firstLetter == 'A') return RifEclipseSummaryAddress::SUMMARY_AQUIFER;
    if (firstLetter == 'B') return RifEclipseSummaryAddress::SUMMARY_BLOCK;
    if (firstLetter == 'C') return RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
    if (firstLetter == 'F') return RifEclipseSummaryAddress::SUMMARY_FIELD;
    if (firstLetter == 'G') return RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
    if (firstLetter == 'N') return RifEclipseSummaryAddress::SUMMARY_NETWORK;
    if (firstLetter == 'R') return RifEclipseSummaryAddress::SUMMARY_REGION; //TODO: CAN BE REGION2REGION OR MISC!!
    if (firstLetter == 'S') return RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
    if (firstLetter == 'W') return RifEclipseSummaryAddress::SUMMARY_WELL;

    if (word.size() < 2) return RifEclipseSummaryAddress::SUMMARY_INVALID;

    std::string firstTwoLetters;
    firstTwoLetters.push_back(word.at(0));
    firstTwoLetters.push_back(word.at(1));

    if (firstTwoLetters == "LB") return RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR;
    if (firstTwoLetters == "LC") return RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR;
    if (firstTwoLetters == "LW") return RifEclipseSummaryAddress::SUMMARY_WELL_LGR;

    /*
    TODO
    return RifEclipseSummaryAddress::SUMMARY_MISC
    return RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION
    */

    return RifEclipseSummaryAddress::SUMMARY_INVALID;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifEclipseUserDataParserTools::findFirstNonEmptyEntryIndex(std::vector<std::string>& list)
{
    for (size_t i = 0; i < list.size(); i++)
    {
        if (!list[i].empty())
        {
            return i;
        }
    }
    return list.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::keywordParser(const std::string& line, std::string& origin, std::string& dateFormat, std::string& startDate)
{
    std::vector<std::string> words = splitLineAndRemoveComments(line);
    if (words.size() < 2) return false;

    if (words[0] == "ORIGIN")
    {
        origin = words[1];
        return true;
    }
    else if (words[0] == "STARTDATE")
    {
        words.erase(words.begin());

        for (size_t i = 0; i < words.size(); i++)
        {
            std::string s = words[i];

            startDate += s;

            if (i < words.size() - 1)
            {
                startDate += " ";
            }
        }

        return true;
    }
    else if (words[0] == "DATEFORMAT")
    {
        dateFormat = words[1];
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RifEclipseUserDataParserTools::splitLineToDoubles(const std::string& line)
{
    std::vector<double> values;

    QString s = QString::fromStdString(line);

    QStringList words = s.split(" ");

    bool ok = false;
    for (auto w : words)
    {
        double val = w.toDouble(&ok);
        if (ok)
        {
            values.push_back(val);
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::isANumber(const std::string& line)
{
    try
    {
        std::stod(line);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifEclipseUserDataParserTools::headerReader(std::stringstream& streamData, std::string& line)
{
    std::vector<std::string> header;

    while (!isANumber(line) && !streamData.eof())
    {
        header.push_back(line);
        std::getline(streamData, line);
    }
    return header;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::hasTimeUnit(const std::string& word)
{
    if (word == "DAYS" ||
        word == "DAY" ||
        word == "YEARS" ||
        word == "YEAR" ||
        word == "DATE" ||
        word == "DATES")
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::hasOnlyValidDoubleValues(const std::vector<std::string>& words, std::vector<double>* doubleValues)
{
    bool onlyValidValues = true;

    for (const auto& word : words)
    {
        if (word.find_first_not_of("0123456789.eE-") != std::string::npos)
        {
            onlyValidValues = false;
        }
        else
        {
            double doubleVal = RiaStdStringTools::toDouble(word);
            doubleValues->push_back(doubleVal);
        }
    }

    return onlyValidValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::isValidTableData(size_t columnCount, const std::string& line)
{
    std::vector<std::string> words = splitLineAndRemoveComments(line);

    if (words.size() != columnCount) return false;

    std::vector<double> doubleValues;
    RifEclipseUserDataParserTools::hasOnlyValidDoubleValues(words, &doubleValues);
    if (doubleValues.size() == columnCount) return true;

    size_t columnsWithDate = 0;
    for (auto w : words)
    {
        if (RiaDateStringParser::parseDateString(w).isValid())
        {
            columnsWithDate++;
        }
    }

    if (columnsWithDate == 1 && doubleValues.size() == columnCount - 1)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TableData RifEclipseUserDataParserTools::tableDataFromText(std::stringstream& streamData, std::vector<std::string>* errorText)
{
    TableData emptyTable;

    std::string origin = "";
    std::string dateFormat = "";
    std::string startDate = "";

    std::string firstLine;
    std::getline(streamData, firstLine);

    while (isLineSkippable(firstLine) || keywordParser(firstLine, origin, dateFormat, startDate))
    {
        if (!streamData.good())
        {
            if (errorText) errorText->push_back("Failed to detect start of table header");

            return emptyTable;
        }

        std::getline(streamData, firstLine);
    }

    std::vector<std::string> quantityNames = splitLineAndRemoveComments(firstLine);
    size_t columnCount = quantityNames.size();

    if (columnCount == 0)
    {
        if (errorText) errorText->push_back("No quantities detected in table");

        return emptyTable;
    }

    std::vector< std::vector< std::string > > allHeaderRows;

    {
        std::stringstream::pos_type posAtStartOfFirstLine = streamData.tellg();

        std::string secondLine;
        std::getline(streamData, firstLine);

        std::stringstream::pos_type posAtStartOfSecondLine = streamData.tellg();
        std::getline(streamData, secondLine);

        bool header = true;
        while (header)
        {
            if (isValidTableData(columnCount, firstLine) &&
                isValidTableData(columnCount, secondLine))
            {
                header = false;
                break;
            }
            else
            {
                std::vector<std::string> words = splitLineAndRemoveComments(firstLine);
                if (words.size() > 0)
                {
                    allHeaderRows.push_back(words);
                }
            }

            posAtStartOfFirstLine = posAtStartOfSecondLine;
            firstLine = secondLine;

            posAtStartOfSecondLine = streamData.tellg();
            std::getline(streamData, secondLine);

            if (!streamData.good())
            {
                header = false;
            }
        }

        streamData.seekg(posAtStartOfFirstLine);
    }

    std::vector<std::string> unitNames;
    std::vector<double> scaleFactors;
    std::vector< std::vector< std::string > > headerRows;

    for (const auto& rowWords : allHeaderRows)
    {
        bool excludeFromHeader = false;

        if (rowWords.size() == columnCount)
        {
            if (unitNames.size() == 0)
            {
                for (const std::string& word : rowWords)
                {
                    if (hasTimeUnit(word))
                    {
                        unitNames = rowWords;
                        excludeFromHeader = true;
                    }
                }
            }

            if (scaleFactors.size() == 0)
            {
                std::vector<double> values;

                if (hasOnlyValidDoubleValues(rowWords, &values))
                {
                    scaleFactors = values;
                    excludeFromHeader = true;
                }
            }
        }

        if (!excludeFromHeader)
        {
            headerRows.push_back(rowWords);
        }
    }

    if (columnCount != unitNames.size())
    {
        if (errorText) errorText->push_back("Number of quantities is different from number of units");

        return emptyTable;
    }


    std::vector<ColumnInfo> columnInfos;

    // Create string vectors for each column
    {
        std::vector<std::string> parserErrors;
        std::vector<std::vector<std::string>> tableHeaderText = RifEclipseUserDataKeywordTools::buildColumnHeaderText(quantityNames, headerRows, &parserErrors);
        if (parserErrors.size() > 0)
        {
            if (errorText) errorText->insert(errorText->end(), parserErrors.begin(), parserErrors.end());
            
            return emptyTable;
        }


        // For each column header, create rif adress and date time
        for (size_t i = 0; i < tableHeaderText.size(); i++)
        {
            auto columnText = tableHeaderText[i];
            if (columnText.size() == 0)
            {
                if (errorText) errorText->push_back("Detected column with no content");
                continue;
            }

            std::string quantity = columnText[0];
            std::string unit = unitNames[i];

            std::vector<std::string> columnHeader;

            if (columnText.size() > 1) columnHeader.insert(columnHeader.begin(), columnText.begin() + 1, columnText.end());

            RifEclipseSummaryAddress adr = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnHeader);

            ColumnInfo ci = ColumnInfo::createColumnInfo(quantity, unit, adr);

            columnInfos.push_back(ci);
        }
    }

    return TableData(origin, dateFormat, startDate, columnInfos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::isFixedWidthHeader(const std::string& lines)
{
    std::stringstream streamData(lines);

    std::vector<std::string> headerLines = RifEclipseUserDataParserTools::findValidHeaderLines(streamData);
    if (headerLines.size() > 1)
    {
        std::vector<size_t> firstLine = RifEclipseUserDataParserTools::columnIndexForWords(headerLines[0]);

        for (auto line : headerLines)
        {
            std::vector<size_t> columnIndicesForLine = RifEclipseUserDataParserTools::columnIndexForWords(line);
            for (auto index : columnIndicesForLine)
            {
                if (std::find(firstLine.begin(), firstLine.end(), index) == firstLine.end())
                {
                    return false;
                }
            }
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::hasCompleteDataForAllHeaderColumns(const std::string& lines)
{
    std::stringstream streamData(lines);

    bool headerDataComplete = true;
    {
        auto lines = RifEclipseUserDataParserTools::findValidHeaderLines(streamData);
        if (lines.size() > 0)
        {
            size_t wordsFirstLine = lines[0].size();

            for (auto line : lines)
            {
                if (wordsFirstLine != line.size())
                {
                    headerDataComplete = false;
                }
            }
        }
    }

    return headerDataComplete;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<ColumnInfo> RifEclipseUserDataParserTools::columnInfoForFixedColumnWidth(std::stringstream& streamData)
{
    auto headerLines = RifEclipseUserDataParserTools::findValidHeaderLines(streamData);

    auto columnHeaders = RifEclipseUserDataParserTools::splitIntoColumnHeaders(headerLines);

    return RifEclipseUserDataParserTools::columnInfoFromColumnHeaders(columnHeaders);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifEclipseUserDataParserTools::findValidHeaderLines(std::stringstream& streamData)
{
    std::vector<std::string> headerLines;

    std::stringstream::pos_type posAtTableDataStart = streamData.tellg();

    size_t columnCount = 0;
    std::string line;
    bool continueParsing = true;
    bool hasStepType = false;
    size_t minimunRequiredExtraHeaderLines = 0;

    while (continueParsing)
    {
        posAtTableDataStart = streamData.tellg();

        if (!std::getline(streamData, line))
        {
            continueParsing = false;
        }
        else
        {
            if (!RifEclipseUserDataParserTools::isLineSkippable(line))
            {
                auto words = RifEclipseUserDataParserTools::splitLineAndRemoveComments(line);

                if (!hasStepType)
                {
                    for (size_t i = 0; i < words.size(); i++)
                    {
                        if (RifEclipseUserDataKeywordTools::isStepType(words[i]))
                        {
                            hasStepType = true;
                        }
                    }
                }

                if (columnCount == 0)
                {
                    // Fist line with valid header data defines the number of columns

                    columnCount = words.size();

                    minimunRequiredExtraHeaderLines = RifEclipseUserDataKeywordTools::computeRequiredHeaderLineCount(words);

                    headerLines.push_back(line);
                }
                else if (headerLines.size() < minimunRequiredExtraHeaderLines)
                {
                    headerLines.push_back(line);
                }
                else
                {
                    std::vector<double> doubleValues = RifEclipseUserDataParserTools::splitLineToDoubles(line);

                    if (doubleValues.size() < columnCount && words.size() < columnCount)
                    {
                        if (hasStepType && (words.size() + 1 == columnCount))
                        {
                            continueParsing = false;
                        }
                        else
                        {
                            // Consider a line with double values less than column count as a table header
                            headerLines.push_back(line);
                        }
                    }
                    else
                    {
                        continueParsing = false;
                    }
                }
            }
        }
    }

    streamData.seekg(posAtTableDataStart);

    return headerLines;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<std::string>> RifEclipseUserDataParserTools::splitIntoColumnHeaders(const std::vector<std::string>& headerLines)
{
    std::vector<std::vector<std::string>> headerLinesPerColumn;

    if (headerLines.size() > 0)
    {
        std::vector<size_t> columnOffsets = RifEclipseUserDataParserTools::columnIndexForWords(headerLines[0]);

        if (columnOffsets.size() > 0)
        {
            headerLinesPerColumn.resize(columnOffsets.size());

            for (auto headerLine : headerLines)
            {
                for (size_t i = 0; i < columnOffsets.size(); i++)
                {
                    size_t colStart = columnOffsets[i];

                    size_t columnWidth = std::string::npos;
                    if (i < columnOffsets.size() - 1)
                    {
                        columnWidth = columnOffsets[i + 1] - colStart;
                    }
                    else
                    {
                        if (headerLine.size() > colStart)
                        {
                            columnWidth = headerLine.size() - colStart;
                        }
                    }

                    std::string subString;
                    if (columnWidth != std::string::npos &&
                        colStart < headerLine.size() &&
                        colStart + columnWidth <= headerLine.size())
                    {
                        subString = headerLine.substr(colStart, columnWidth);
                    }

                    subString = trimString(subString);

                    headerLinesPerColumn[i].push_back(subString);
                }
            }
        }
    }

    return headerLinesPerColumn;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<ColumnInfo> RifEclipseUserDataParserTools::columnInfoFromColumnHeaders(const std::vector<std::vector<std::string>>& columnData)
{
    std::vector<ColumnInfo> table;

    bool isUnitsDetected = false;
    bool isScalingDetected = false;

    for (auto columnLines : columnData)
    {
        if (columnLines.size() > 1 && isUnitText(columnLines[1]))
        {
            isUnitsDetected = true;
        }

        if (columnLines.size() > 2 && isScalingText(columnLines[2]))
        {
            isScalingDetected = true;
        }
    }

    for (auto columnLines : columnData)
    {
        if (columnLines.size() == 0) continue;

        std::string quantity = columnLines[0];
        std::string unit;
        std::string scaling;

        size_t startIndex = 1;

        if (isUnitsDetected)
        {
            unit = columnLines[1];

            startIndex = 2;
        }

        if (isScalingDetected)
        {
            scaling = columnLines[2];

            startIndex = 3;
        }

        std::vector<std::string> restOfHeader;
        for (size_t i = startIndex; i < columnLines.size(); i++)
        {
            restOfHeader.push_back(columnLines[i]);
        }

        RifEclipseSummaryAddress adr = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, restOfHeader);

        ColumnInfo ci = ColumnInfo::createColumnInfo(quantity, unit, adr);

        table.push_back(ci);
    }

    return table;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifEclipseUserDataParserTools::columnIndexForWords(const std::string& line)
{
    std::vector<size_t> columnOffsets;

    std::size_t offset = line.find_first_not_of(" ");
    while (offset != std::string::npos)
    {
        columnOffsets.push_back(offset);

        offset = line.find_first_of(" ", offset);
        offset = line.find_first_not_of(" ", offset);
    }

    return columnOffsets;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<TableData> RifEclipseUserDataParserTools::mergeEqualTimeSteps(const std::vector<TableData>& tables)
{
    if (tables.size() < 2)
    {
        return tables;
    }

    if (tables[0].columnInfos().size() == 0) return tables;

    ColumnInfo* dateColumn = nullptr;
    for (auto c : tables[0].columnInfos())
    {
        if (c.summaryAddress.quantityName() == "DATE")
        {
            dateColumn = &c;
        }
    }

    // Support only merge of tables with the DATE column present
    if (!dateColumn) return tables;

    std::vector<TableData> largeTables;

    largeTables.push_back(tables[0]);

    TableData& firstTable = largeTables[0];
    size_t itemsInFirstTable = tables[0].columnInfos()[0].itemCount();

    for (size_t i = 1; i < tables.size(); i++)
    {
        if (tables[i].columnInfos().size() > 0 &&
            tables[i].columnInfos()[0].itemCount() == itemsInFirstTable)
        {
            for (auto& c : tables[i].columnInfos())
            {
                if (c.summaryAddress.quantityName() != "DATE")
                {
                    firstTable.columnInfos().push_back(c);
                }
            }
        }
        else
        {
            largeTables.push_back(tables[i]);
        }
    }

    return largeTables;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseUserDataParserTools::trimString(const std::string& s)
{
    auto sCopy = s.substr(0, s.find_last_not_of(' ') + 1);
    if (sCopy.size() > 0)
    {
        sCopy = sCopy.substr(sCopy.find_first_not_of(' '));
    }

    return sCopy;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::isUnitText(const std::string& word)
{
    if (hasTimeUnit(word)) return true;

    if (word.find("BARSA") != std::string::npos) return true;
    if (word.find("SM3") != std::string::npos) return true;
    if (word.find("RM3") != std::string::npos) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::isScalingText(const std::string& word)
{
    return word.find_first_of('*') != std::string::npos;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t ColumnInfo::itemCount() const
{
    if (isStringData)
    {
        return stringValues.size();
    }
    else
        return values.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ColumnInfo ColumnInfo::createColumnInfo(const std::string& quantity, const std::string& unit, const RifEclipseSummaryAddress& adr)
{
    ColumnInfo ci(adr, unit);

    if (RifEclipseUserDataKeywordTools::isDate(quantity))
    {
        ci.isStringData = true;
    }
    else if (RifEclipseUserDataKeywordTools::isStepType(quantity))
    {
        ci.isStringData = true;
    }

    return ci;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime TableData::findFirstDate() const
{
    QDateTime dt;

    for (auto ci : m_columnInfos)
    {
        if (RifEclipseUserDataKeywordTools::isDate(ci.summaryAddress.quantityName()))
        {
            if (ci.stringValues.size() > 0)
            {
                std::string firstDateString = ci.stringValues[0];

                dt = RiaDateStringParser::parseDateString(firstDateString);
            }
        }
    }

    return dt;
}
