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

#include "RiaLogging.h"

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
    if (word.size() > 1 && word.substr(0, 2) == "--")
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
RifEclipseSummaryAddress RifEclipseUserDataParserTools::makeAndFillAddress(std::string quantityName, std::vector< std::string > headerColumn)
{
    int         regionNumber = -1;
    int         regionNumber2 = -1;
    std::string wellGroupName = "";
    std::string wellName = "";
    int         wellSegmentNumber = -1;
    std::string lgrName = "";
    int         cellI = -1;
    int         cellJ = -1;
    int         cellK = -1;

    RifEclipseSummaryAddress::SummaryVarCategory category = identifyCategory(quantityName);

    switch (category) //TODO: More categories
    {
    case (RifEclipseSummaryAddress::SUMMARY_INVALID):
    {
        break;
    }
    case (RifEclipseSummaryAddress::SUMMARY_WELL):
    {
        size_t index = findFirstNonEmptyEntryIndex(headerColumn);
        if (index < headerColumn.size())
        {
            wellName = headerColumn[index];
        }
	    break;
    }
    case (RifEclipseSummaryAddress::SUMMARY_WELL_GROUP):
    {
        size_t index = findFirstNonEmptyEntryIndex(headerColumn);
        if (index < headerColumn.size())
        {
            wellGroupName = headerColumn[index];
        }
        break;
    }
    case (RifEclipseSummaryAddress::SUMMARY_REGION):
    {
        size_t index = findFirstNonEmptyEntryIndex(headerColumn);
        if (index < headerColumn.size())
        {
            try
            {
                regionNumber = std::stoi(headerColumn[index]);
            }
            catch (...){}
        }
        break;
    }
    default:
        break;
    }

    return RifEclipseSummaryAddress(category,
        quantityName,
        regionNumber,
        regionNumber2,
        wellGroupName,
        wellName,
        wellSegmentNumber,
        lgrName,
        cellI,
        cellJ,
        cellK);
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
        startDate = std::accumulate(words.begin(), words.end(), std::string(""));
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
std::vector<ColumnInfo> RifEclipseUserDataParserTools::columnInfoForTable(std::stringstream& streamData)
{
    std::vector<ColumnInfo> table;

    std::string origin = "";
    std::string dateFormat = "";
    std::string startDate  = "";

    std::string line;
    std::getline(streamData, line);

    while (isLineSkippable(line) || keywordParser(line, origin, dateFormat, startDate))
    {
        if (!streamData.good()) return table;
        
        std::getline(streamData, line);
    }

    std::vector<std::string> quantityNames = splitLineAndRemoveComments(line);
    size_t columnCount = quantityNames.size();

    std::vector< std::vector< std::string > > allHeaderRows;

    {
        std::stringstream::pos_type posAtStartOfLine = streamData.tellg();

        std::string secondLine;
        std::getline(streamData, line);
    
        std::stringstream::pos_type posAtStartOfSecondLine = streamData.tellg();
        std::getline(streamData, secondLine);

        bool header = true;
        while (header)
        {
            std::vector<std::string> words = splitLineAndRemoveComments(line);
            std::vector<std::string> wordsSecondLine = splitLineAndRemoveComments(secondLine);

            if (words.size() == columnCount &&
                wordsSecondLine.size() == columnCount &&
                hasOnlyValidDoubleValues(words) &&
                hasOnlyValidDoubleValues(wordsSecondLine))
            {
                header = false;
                break;
            }
            else if (words.size() > columnCount)
            {
                continue;
            }
            else
            {
                size_t diff = columnCount - words.size();

                if (diff == columnCount)
                {
                    std::vector< std::string > vectorOfEmptyStrings(columnCount, "");
                    allHeaderRows.push_back(vectorOfEmptyStrings);
                }
                else
                {
                    words.insert(words.begin(), diff, "");
                    allHeaderRows.push_back(words);
                }
            }

            posAtStartOfLine = posAtStartOfSecondLine;
            line = secondLine;

            posAtStartOfSecondLine = streamData.tellg();
            std::getline(streamData, secondLine);
        }

        streamData.seekg(posAtStartOfLine);
    }

    std::vector<std::string> unitNames;
    std::vector<double> scaleFactors;
    std::vector< std::vector< std::string > > restOfHeaderRows;

    for (const auto& wordsForRow : allHeaderRows)
    {
        bool excludeFromHeader = false;
        if (unitNames.size() == 0)
        {
            for (const std::string& word : wordsForRow)
            {
                if (hasTimeUnit(word))
                {
                    unitNames = wordsForRow;
                    excludeFromHeader = true;
                }
            }
        }

        if (scaleFactors.size() == 0)
        {
            std::vector<double> values;

            if (hasOnlyValidDoubleValues(wordsForRow, &values))
            {
                scaleFactors = values;
                excludeFromHeader = true;
            }
        }

        if (!excludeFromHeader)
        {
            restOfHeaderRows.push_back(wordsForRow);
        }
    }

    for (const std::string& unit : unitNames)
    {
        ColumnInfo columnInfo;
        columnInfo.unitName = unit;
        columnInfo.origin = origin;
        columnInfo.dateFormatString = dateFormat;
        columnInfo.startDateString = startDate;
        table.push_back(columnInfo);
    }

    for (size_t i = 0; i < table.size(); i++)
    {
        if (scaleFactors.size() == table.size())
        {
            table[i].scaleFactor = scaleFactors[i];
        }
        else
        {
            table[i].scaleFactor = 1.0;
        }
    }
    
    for (size_t i = 0; i < table.size(); i++)
    {
        std::vector< std::string > restOfHeaderColumn;
        for (std::vector< std::string > restOfHeaderRow : restOfHeaderRows)
        {
            restOfHeaderColumn.push_back(restOfHeaderRow.at(i));
        }
        table[i].summaryAddress = makeAndFillAddress(quantityNames.at(i), restOfHeaderColumn);
    }

    for (ColumnInfo& column : table)
    {
        if (column.summaryAddress.category() != RifEclipseSummaryAddress::SUMMARY_INVALID)
        {
            column.isAVector = true;
        }
    }

    return table;
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

    for (const auto& word : words)
    {
        double doubleVal = strtod(word.data(), &end);
        if (end == word.data())
        {
            return false;
        }

        if (doubleValues)
        {
            doubleValues->push_back(doubleVal);
        }
    }

    return true;
}
