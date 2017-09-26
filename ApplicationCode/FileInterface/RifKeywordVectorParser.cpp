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

#include "RifKeywordVectorParser.h"

#include "RifRsmspecParserTools.h"

#include "cvfAssert.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifKeywordVectorParser::RifKeywordVectorParser(const QString& data)
{
    parseData(data);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<KeywordBasedVector>& RifKeywordVectorParser::keywordBasedVectors() const
{
    return m_keywordBasedVectors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifKeywordVectorParser::canBeParsed(const QString& data)
{
    std::stringstream streamData;
    streamData.str(data.toStdString());
    std::string line;
    std::getline(streamData, line);

    while (streamData.good())
    {
        if (line.size() > 1 && line[0] == '-' && line[1] == '-')
        {
            std::getline(streamData, line);
        }
        else if (line.find("VECTOR") == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifKeywordVectorParser::parseData(const QString& data)
{
    std::stringstream streamData;
    streamData.str(data.toStdString());
    std::string line;
    std::getline(streamData, line);

    do 
    {
        while (RifRsmspecParserTools::isLineSkippable(line) && !streamData.eof())
        {
            std::getline(streamData, line);
        }

        KeywordBasedVector keywordBasedVector;
        keywordBasedVector.header = RifRsmspecParserTools::headerReader(streamData, line);
        if (keywordBasedVector.header.empty()) break;

        while (RifRsmspecParserTools::isANumber(line))
        {
            keywordBasedVector.values.push_back(std::stod(line));
            std::getline(streamData, line);
        }

        m_keywordBasedVectors.push_back(keywordBasedVector);

    } while (!streamData.eof());
}
