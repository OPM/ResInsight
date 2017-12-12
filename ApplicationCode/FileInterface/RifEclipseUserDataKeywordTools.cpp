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

#include "RifEclipseUserDataKeywordTools.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "RifEclipseUserDataParserTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(const std::string& identifier)
{
    if (identifier.size() < 2) return {};

    char firstLetter = identifier[0];
    switch (firstLetter)
    {
        case 'B': return { 3 };     // Block triplet
        case 'C': return { 1, 3 };  // Well Name and completion triplet
        case 'G': return { 1 };     // Group
        case 'R': return { 1 };     // Region number
        case 'S': return { 1, 1 };  // Well name and segment number
        case 'W': return { 1 };     // Well Name
    }

    std::string firstTwoLetters = identifier.substr(0, 2);

    if      (firstTwoLetters == "LB") return { 1, 3 };      // LGR name and block triplet
    else if (firstTwoLetters == "LC") return { 1, 1, 3 };   // LGR name, well name and block triplet
    else if (firstTwoLetters == "LW") return { 1, 1 };      // LGR name and well name

    return { };
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<std::string>> RifEclipseUserDataKeywordTools::buildColumnHeaderText(const std::vector<std::string>& quantityNames,
                                                                                            const std::vector<std::vector<std::string>>& restOfHeaderRows,
                                                                                            std::vector<std::string>* errorText)
{
    std::vector<std::vector<std::string>> tableHeaderText;

    std::vector<size_t> headerLineWordIndices(restOfHeaderRows.size(), 0);

    for (size_t i = 0; i < quantityNames.size(); i++)
    {
        std::vector<std::string> columnHeaderText;

        auto quantityName = quantityNames[i];
        columnHeaderText.push_back(quantityName);

        auto itemCountPerLine = RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(quantityName);
        if (itemCountPerLine.size() > restOfHeaderRows.size())
        {
            std::string text = "Detected too few header lines";
            if (errorText) errorText->push_back(text);
            
            return std::vector<std::vector<std::string>>();
        }

        for (size_t lineIdx = 0; lineIdx < itemCountPerLine.size(); lineIdx++)
        {
            auto line = restOfHeaderRows[lineIdx];

            for (size_t itemIndex = 0; itemIndex < itemCountPerLine[lineIdx]; itemIndex++)
            {
                size_t wordIndex = headerLineWordIndices[lineIdx];
                if (wordIndex >= line.size())
                {
                    std::string text = "Detected too few items for header line";
                    if (errorText) errorText->push_back(text);

                    return std::vector<std::vector<std::string>>();
                }

                auto word = line[wordIndex];

                columnHeaderText.push_back(word);

                headerLineWordIndices[lineIdx]++;
            }
        }

        tableHeaderText.push_back(columnHeaderText);
    }

    return tableHeaderText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataKeywordTools::isTime(const std::string& identifier)
{
    return (identifier == "TIME");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataKeywordTools::isDate(const std::string& identifier)
{
    return (identifier == "DATE");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataKeywordTools::isDays(const std::string& identifier)
{
    return (identifier.find("DAYS") != std::string::npos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataKeywordTools::isYears(const std::string& identifier)
{
    return (identifier.find("YEARS") != std::string::npos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataKeywordTools::isYearX(const std::string& identifier)
{
    return (identifier.find("YEARX") != std::string::npos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseUserDataKeywordTools::makeAndFillAddress(const std::string quantityName, const std::vector<std::string>& columnHeaderText)
{
    RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseUserDataParserTools::identifyCategory(quantityName);
    
    if (category == RifEclipseSummaryAddress::SUMMARY_INVALID)
    {
        return RifEclipseSummaryAddress::importedAddress(quantityName);
    }

    int         regionNumber = -1;
    int         regionNumber2 = -1;
    std::string wellGroupName = "";
    std::string wellName = "";
    int         wellSegmentNumber = -1;
    std::string lgrName = "";
    int         cellI = -1;
    int         cellJ = -1;
    int         cellK = -1;
    int         aquiferNumber = -1;

    switch (category)
    {
    case RifEclipseSummaryAddress::SUMMARY_FIELD:
        break;
    case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
    {
        if (columnHeaderText.size() > 0)
        {
            aquiferNumber = RiaStdStringTools::toInt(columnHeaderText[0]);
        }
        break;
    }
    case RifEclipseSummaryAddress::SUMMARY_NETWORK:
        break;
    case RifEclipseSummaryAddress::SUMMARY_MISC:
        break;
    case RifEclipseSummaryAddress::SUMMARY_REGION:
    {
        if (columnHeaderText.size() > 0)
        {
            regionNumber = RiaStdStringTools::toInt(columnHeaderText[0]);
        }
        break;
    }
    case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
    {
        if (columnHeaderText.size() > 0)
        {
            wellGroupName = columnHeaderText[0];
        }
        break;
    }
    case RifEclipseSummaryAddress::SUMMARY_WELL:
    {
        if (columnHeaderText.size() > 0)
        {
            wellName = columnHeaderText[0];
        }
        break;
    }
    case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
    {
        if (columnHeaderText.size() > 1)
        {
            wellName = columnHeaderText[0];

            RifEclipseUserDataKeywordTools::extractThreeInts(&cellI, &cellJ, &cellK, columnHeaderText[1]);
        }
        break;
    }
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        if (columnHeaderText.size() > 1)
        {
            wellName = columnHeaderText[0];
            lgrName  = columnHeaderText[1];
        }
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        if (columnHeaderText.size() > 2)
        {
            wellName = columnHeaderText[0];
            lgrName  = columnHeaderText[1];

            RifEclipseUserDataKeywordTools::extractThreeInts(&cellI, &cellJ, &cellK, columnHeaderText[2]);
        }
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        if (columnHeaderText.size() > 1)
        {
            wellName                                   = columnHeaderText[0];
            wellSegmentNumber = RiaStdStringTools::toInt(columnHeaderText[1]);
        }
        break;
    case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        if (columnHeaderText.size() > 0)
        {
            RifEclipseUserDataKeywordTools::extractThreeInts(&cellI, &cellJ, &cellK, columnHeaderText[0]);
        }
        break;
    case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        if (columnHeaderText.size() > 1)
        {
            lgrName = columnHeaderText[0];

            RifEclipseUserDataKeywordTools::extractThreeInts(&cellI, &cellJ, &cellK, columnHeaderText[1]);
        }
        break;
    case RifEclipseSummaryAddress::SUMMARY_CALCULATED:
        break;
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
                                    cellK,
                                    aquiferNumber);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataKeywordTools::isStepType(const std::string& identifier)
{
    return (identifier.find("STEPTYPE") != std::string::npos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifEclipseUserDataKeywordTools::computeRequiredHeaderLineCount(const std::vector<std::string>& words)
{
    size_t maxHeaderLinesFromKeywords = 0;

    for (auto w : words)
    {
        if (knownKeywordsWithZeroRequiredHeaderLines(w)) continue;

        auto linesForKeyword = RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(w).size();
        if (linesForKeyword > maxHeaderLinesFromKeywords)
        {
            maxHeaderLinesFromKeywords = linesForKeyword;
        }
    }

    // Quantity and unit, scaling is optional
    return 1 + maxHeaderLinesFromKeywords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUserDataKeywordTools::knownKeywordsWithZeroRequiredHeaderLines(const std::string& identifier)
{
    if (identifier.find("DAY") != std::string::npos) return true;
    if (identifier.find("MONTH") != std::string::npos) return true;
    if (identifier.find("YEAR") != std::string::npos) return true;
    
    if (identifier.find("DATE") != std::string::npos) return true;
    if (identifier.find("TIME") != std::string::npos) return true;
    
    if (identifier.find("ELAPSED") != std::string::npos) return true;

    if (identifier.find("NEWTON") != std::string::npos) return true;
    
    if (identifier.find("NLINSMIN") != std::string::npos) return true;
    if (identifier.find("NLINSMAX") != std::string::npos) return true;
    
    if (identifier.find("MLINEARS") != std::string::npos) return true;
    
    if (identifier.find("MSUMLINS") != std::string::npos) return true;
    if (identifier.find("MSUMNEWT") != std::string::npos) return true;
   
    if (identifier.find("TCPU") != std::string::npos) return true;
    if (identifier.find("TCPUTS") != std::string::npos) return true;
    if (identifier.find("TCPUDAY") != std::string::npos) return true;

    if (identifier.find("TELAPLIN") != std::string::npos) return true;
    if (identifier.find("STEPTYPE") != std::string::npos) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseUserDataKeywordTools::extractThreeInts(int* cellI, int* cellJ, int* cellK, const std::string& line)
{
    std::vector<std::string> words = RiaStdStringTools::splitStringBySpace(line);
    if (words.size() > 2)
    {
        *cellI = RiaStdStringTools::toInt(words[0]);
        *cellJ = RiaStdStringTools::toInt(words[1]);
        *cellK = RiaStdStringTools::toInt(words[2]);
    }
}

