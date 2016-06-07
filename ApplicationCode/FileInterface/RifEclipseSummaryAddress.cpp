/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifEclipseSummaryAddress.h"

#include <vector>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress(const std::string& ertSummaryVarId)
    : m_ertSummaryVarId(ertSummaryVarId)
{
    fromErtSummaryVarId(m_ertSummaryVarId, &m_variableCategory, &m_simulationItemName, &m_quantityName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress(SummaryVarCategory category, const std::string& simulationItemName, const std::string& quantityName)
    : m_variableCategory(category),
    m_simulationItemName(simulationItemName),
    m_quantityName(quantityName)
{
    m_ertSummaryVarId = toErtSummaryVarId(m_variableCategory, m_simulationItemName, m_quantityName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory RifEclipseSummaryAddress::category() const
{
    return m_variableCategory;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::simulationItemName() const
{
    return m_simulationItemName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::quantityName() const
{
    return m_quantityName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::ertSummaryVarId() const
{
    return m_ertSummaryVarId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::categoryName(SummaryVarCategory category)
{
    std::string name;

    switch (category)
    {
    case RifEclipseSummaryAddress::SUMMARY_WELL:
        name = "Well";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        name = "Completion";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        name = "Group";
        break;
    case RifEclipseSummaryAddress::SUMMARY_FIELD:
        name = "Field";
        break;
    case RifEclipseSummaryAddress::SUMMARY_REGION:
        name = "Region";
        break;
    case RifEclipseSummaryAddress::SUMMARY_MISC:
        name = "Misc";
        break;
    case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        name = "Block";
        break;
    case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        name = "LGR Block";
        break;
    case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        name = "Aquifier";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        name = "Segment";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT_RIVER:
        name = "Segment River";
        break;
    default:
        break;
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::toErtSummaryVarId(SummaryVarCategory category, const std::string& simulationItemName, const std::string& quantityName)
{
    std::string ertSummaryVarId;

    ertSummaryVarId += prefixForCategory(category);
    ertSummaryVarId += quantityName;

    if (simulationItemName.size() != 0)
    {
        ertSummaryVarId += ":";
        ertSummaryVarId += simulationItemName;
    }

    return ertSummaryVarId;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> split(const std::string &text, char sep) {
    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
    return tokens;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::fromErtSummaryVarId(const std::string& ertSummaryVarId, SummaryVarCategory* type, std::string* simulationItemName, std::string* quantityName)
{
    *type = categoryFromErtSummaryVarId(ertSummaryVarId);

    std::string prefix = prefixForCategory(*type);

    std::string addressNoPrefix = ertSummaryVarId.substr(prefix.size());
    
    std::vector<std::string> tokens = split(addressNoPrefix, ':');
    if (tokens.size() > 0)
    {
        *quantityName = tokens[0];
    }

    if (tokens.size() > 1)
    {
        *simulationItemName = tokens[1];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::prefixForCategory(SummaryVarCategory category)
{
    std::string prefix;

    switch (category)
    {
    case RifEclipseSummaryAddress::SUMMARY_WELL:
        prefix = "W";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        prefix = "C";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        prefix = "G";
        break;
    case RifEclipseSummaryAddress::SUMMARY_FIELD:
        prefix = "F";
        break;
    case RifEclipseSummaryAddress::SUMMARY_REGION:
        prefix = "R";
        break;
    case RifEclipseSummaryAddress::SUMMARY_MISC:
        break;
    case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        prefix = "B";
        break;
    case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        prefix = "LB";
        break;
    case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        prefix = "A";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        prefix = "S";
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT_RIVER:
        prefix = "SR";
        break;
    default:
        break;
    }

    return prefix;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory RifEclipseSummaryAddress::categoryFromErtSummaryVarId(const std::string& resultAddress)
{
    SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_MISC;

    if (resultAddress.size() > 1)
    {
        std::string twoFirstChars = resultAddress.substr(0, 2);

        if (twoFirstChars == "SR")
        {
            category = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT_RIVER;
        }
        else if (twoFirstChars == "LB")
        {
            category = RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR;
        }
        else
        {
            char firstChar = resultAddress[0];
            switch (firstChar)
            {
            case 'W':
                category = RifEclipseSummaryAddress::SUMMARY_WELL;
                break;
            case 'C':
                category = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
                break;
            case 'G':
                category = RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
                break;
            case 'F':
                category = RifEclipseSummaryAddress::SUMMARY_FIELD;
                break;
            case 'R':
                category = RifEclipseSummaryAddress::SUMMARY_REGION;
                break;
            case 'B':
                category = RifEclipseSummaryAddress::SUMMARY_BLOCK;
                break;
            case 'A':
                category = RifEclipseSummaryAddress::SUMMARY_AQUIFER;
                break;
            case 'S':
                category = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
                break;
            default:
                break;
            }
        }
    }

    return category;
}

