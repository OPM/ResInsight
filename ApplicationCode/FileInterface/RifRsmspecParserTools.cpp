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
bool RifRsmspecParserTools::isAMnemonic(const std::string& word)
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
std::vector<ColumnInfo> RifRsmspecParserTools::columnInfoForTable(std::stringstream& streamData, std::string& line)
{
    size_t columnCount = 0;
    size_t vectorCount = 0;

    std::vector<ColumnInfo> table;
    bool header = true;
    while (header)
    {
        while (isLineSkippable(line))
        {
            if (!streamData.good()) return table;
            std::getline(streamData, line);
        }

        std::vector<std::string> words = splitLine(line);
        if (!words.empty())
        {
            if (words[0] == "TIME")
            {
                for (std::string word : words)
                {
                    ColumnInfo columnInfo;
                    if (isAMnemonic(word))
                    {
                        columnInfo.isAVector = true;
                        columnInfo.category = identifyCategory(word);
                        ++vectorCount;
                    }
                    columnInfo.quantityName = word;
                    table.push_back(columnInfo);
                }
                columnCount = table.size();
            }
            else if (words[0] == "DAYS")
            {
                if (words.size() == columnCount)
                {
                    for (int i = 0; i < words.size(); i++)
                    {
                        table[i].unitName = words[i];
                    }
                }
            }
            else if (words.size() == vectorCount)
            {
                for (int i = 0; i < words.size(); i++)
                {
                    switch (table[i].category) //TODO: More categories
                    {
                    case (RifEclipseSummaryAddress::SUMMARY_INVALID):
                        break;
                    case (RifEclipseSummaryAddress::SUMMARY_WELL):
                        table[i].wellName = words[i];
                        break;
                    case (RifEclipseSummaryAddress::SUMMARY_WELL_GROUP):
                        table[i].wellGroupName = words[i];
                        break;
                    case (RifEclipseSummaryAddress::SUMMARY_REGION):
                        table[i].regionNumber = std::stoi(words[i]);
                        break;
                    default:
                        break;
                    }
                }
            }
            else if (words.size() == columnCount)
            {
                /* TODO: Scale factor
                for (int i = 0; i < words.size(); i++)
                {
                table[i].scaleFactor = words[i];
                }*/

                header = false;
                break;
            }
        }

        std::getline(streamData, line);
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