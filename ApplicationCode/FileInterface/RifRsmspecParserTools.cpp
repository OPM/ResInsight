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

#include "RifRsmspecParserTools.h"

#include "RiaLogging.h"

#include "cvfAssert.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifRsmspecParserTools::isLineSkippable(const std::string& line)
{
    if (line.size() == 0)
    {
        return true;
    }
    else if (line.size() > 1 && line[0] == '-' && line[1] == '-')
    {
        return true;
    }
    else if (line.size() == 1 && line[0] == '1')
    {
        return true;
    }

    std::string str(line);
    
    if (str.find("SUMMARY") < str.size())
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifRsmspecParserTools::splitLine(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> words{ std::istream_iterator<std::string>{iss},
                               std::istream_iterator<std::string>{} };
    return words;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifRsmspecParserTools::isAComment(const std::string& word)
{
    if (word.size() > 1 && word[0] == '-' && word[1] == '-')
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifRsmspecParserTools::splitLineAndRemoveComments(const std::string& line)
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
bool RifRsmspecParserTools::canBeAMnemonic(const std::string& word)
{
    if (word.size() < 1) return false;

    char firstLetter = word.at(0);

    if (firstLetter == 'A' ||
        firstLetter == 'B' ||
        firstLetter == 'C' ||
        firstLetter == 'F' ||
        firstLetter == 'G' ||
        firstLetter == 'N' ||
        firstLetter == 'R' ||
        firstLetter == 'S' ||
        firstLetter == 'W' )
    {
        return true;
    }

    if (word.size() < 2) return false;
    
    std::string firstTwoLetters;
    firstTwoLetters.push_back(word.at(0));
    firstTwoLetters.push_back(word.at(1));

    if (firstTwoLetters == "LB" ||
        firstTwoLetters == "LC" ||
        firstTwoLetters == "LW" )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory RifRsmspecParserTools::identifyCategory(const std::string& word)
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
size_t RifRsmspecParserTools::findFirstNonEmptyEntryIndex(std::vector<std::string>& list)
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
RifEclipseSummaryAddress RifRsmspecParserTools::makeAndFillAddress(std::string scaleFactor, std::string quantityName, std::vector< std::string > headerColumn)
{
    int                                             regionNumber = -1;
    int                                             regionNumber2 = -1;
    std::string                                     wellGroupName = "";
    std::string                                     wellName = "";
    int                                             wellSegmentNumber = -1;
    std::string                                     lgrName = "";
    int                                             cellI = -1;
    int                                             cellJ = -1;
    int                                             cellK = -1;

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
std::vector<ColumnInfo> RifRsmspecParserTools::columnInfoForTable(std::stringstream& streamData, std::string& line)
{
    size_t columnCount = 0;

    std::vector<ColumnInfo> table;

    while (isLineSkippable(line))
    {
        if (!streamData.good()) return table;
        std::getline(streamData, line);
    }

    std::vector<std::string> quantityNames = splitLineAndRemoveComments(line);
    std::getline(streamData, line);
    std::vector<std::string> unitNames = splitLineAndRemoveComments(line);
    std::getline(streamData, line);
    std::vector<std::string> scaleFactors = splitLineAndRemoveComments(line);
    
    std::vector<RifEclipseSummaryAddress::SummaryVarCategory> categories;
    columnCount = quantityNames.size();

    for (std::string unit : unitNames)
    {
        ColumnInfo columnInfo;
        columnInfo.unitName = unit;
        table.push_back(columnInfo);
    }

    if (scaleFactors.size() < columnCount)
    {
        int diff = columnCount - scaleFactors.size();
        for (int i = 0; i < diff; i++)
        {
            scaleFactors.push_back("1.0");
        }
    }

    std::vector< std::vector< std::string > > restOfHeader;

    bool header = true;
    while (header)
    {
        std::getline(streamData, line);

        std::vector<std::string> words = splitLineAndRemoveComments(line);

        if (words.size() == columnCount)
        {
            header = false;
            break;
        }
        else
        {
            int diff = columnCount - words.size();
            
            if (diff == columnCount)
            {
                std::vector< std::string > vectorOfEmptyStrings(columnCount, "");
                restOfHeader.push_back(vectorOfEmptyStrings);
            }
            else
            {
                std::vector< std::string > wordsWithEmptyStrings(diff, "");
                wordsWithEmptyStrings.insert(wordsWithEmptyStrings.begin(), words.begin(), words.end());
                restOfHeader.push_back(wordsWithEmptyStrings);
            }
        }
    }
    
    for (size_t i = 0; i < columnCount; i++)
    {
        std::vector< std::string > restOfHeaderColumn;
        for (std::vector< std::string > restOfHeaderRow : restOfHeader)
        {
            restOfHeaderColumn.push_back(restOfHeaderRow.at(i));
        }
        table[i].summaryAddress = makeAndFillAddress(scaleFactors.at(i), quantityNames.at(i), restOfHeaderColumn);
    }

    return table;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifRsmspecParserTools::splitLineToDoubles(const std::string& line, std::vector<double>& values)
{
    std::istringstream iss(line);
    values.clear();
    
    while (iss.good())
    {
        double d;
        iss >> d;
        values.push_back(d);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifRsmspecParserTools::isANumber(const std::string& line)
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
std::vector<std::string> RifRsmspecParserTools::headerReader(std::stringstream& streamData, std::string& line)
{
    std::vector<std::string> header;

    while (!isANumber(line) && !streamData.eof())
    {
        header.push_back(line);
        std::getline(streamData, line);
    }
    return header;
}