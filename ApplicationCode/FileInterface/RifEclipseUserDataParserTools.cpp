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
    
    if (line[0] == '1')
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
void RifEclipseUserDataParserTools::splitLineToDoubles(const std::string& line, std::vector<double>& values)
{
    std::istringstream iss(line);
    values.clear();
    
    double d;
    while (iss >> d)
    {
        values.push_back(d);
    }
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
    char* end;

    bool onlyValidValues = true;

    for (const auto& word : words)
    {
        if (word.find_first_not_of("0123456789.eE-") != std::string::npos)
        {
            onlyValidValues = false;
        }
        else
        {
            double doubleVal = strtod(word.data(), &end);
            doubleValues->push_back(doubleVal);
        }
    }

    return onlyValidValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataParserTools::hasDateUnit(const std::string& word)
{
    if (word.find("DATE") != std::string::npos) return true;

    return false;
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

            ColumnInfo ci(adr, unit);
            if (quantity == "DATE")
            {
                ci.isStringData = true;
            }

            columnInfos.push_back(ci);
        }
    }

    return TableData(origin, dateFormat, startDate, columnInfos);
}
