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
#include "cvfAssert.h"

// todo: Make class member
std::tuple<int, int, int> ijkTupleFromString(const std::string &s)
{
    auto firstSep = s.find(',');
    auto lastSep = s.find(',', firstSep + 1);
    CVF_ASSERT(firstSep != std::string::npos && lastSep != std::string::npos);
    auto textI = s.substr(0, firstSep);
    auto textJ = s.substr(firstSep + 1, lastSep - firstSep - 1);
    auto textK = s.substr(lastSep + 1);
    return std::make_tuple(std::stoi(textI), std::stoi(textJ), std::stoi(textK));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress(SummaryVarCategory category,
                                                   std::map<SummaryIdentifierType, std::string>& identifiers) :
    m_variableCategory(category),
    m_regionNumber(-1),
    m_regionNumber2(-1),
    m_wellSegmentNumber(-1),
    m_cellI(-1),
    m_cellJ(-1),
    m_cellK(-1)
{
    std::tuple<int, int, int> ijkTuple;
    switch (category)
    {
    case SUMMARY_REGION:
        m_regionNumber = std::stoi(identifiers[INPUT_REGION_NUMBER]);
        break;
    case SUMMARY_REGION_2_REGION:
        m_regionNumber = std::stoi(identifiers[INPUT_REGION_NUMBER]);
        m_regionNumber2 = std::stoi(identifiers[INPUT_REGION2_NUMBER]);
        break;
    case SUMMARY_WELL_GROUP:
        m_wellGroupName = identifiers[INPUT_WELL_GROUP_NAME];
        break;
    case SUMMARY_WELL:
        m_wellName = identifiers[INPUT_WELL_NAME];
        break;
    case SUMMARY_WELL_COMPLETION:
        m_wellName = identifiers[INPUT_WELL_NAME];
        ijkTuple = ijkTupleFromString(identifiers[INPUT_CELL_IJK]);
        m_cellI = std::get<0>(ijkTuple);
        m_cellJ = std::get<1>(ijkTuple);
        m_cellK = std::get<2>(ijkTuple);
        break;
    case SUMMARY_WELL_LGR:
        m_lgrName = identifiers[INPUT_LGR_NAME];
        m_wellName = identifiers[INPUT_WELL_NAME];
        break;
    case SUMMARY_WELL_COMPLETION_LGR:
        m_lgrName = identifiers[INPUT_LGR_NAME];
        m_wellName = identifiers[INPUT_WELL_NAME];
        ijkTuple = ijkTupleFromString(identifiers[INPUT_CELL_IJK]);
        m_cellI = std::get<0>(ijkTuple);
        m_cellJ = std::get<1>(ijkTuple);
        m_cellK = std::get<2>(ijkTuple);
        break;
    case SUMMARY_WELL_SEGMENT:
        m_wellName = identifiers[INPUT_WELL_NAME];
        m_wellSegmentNumber = std::stoi(identifiers[INPUT_SEGMENT_NUMBER]);
    case SUMMARY_BLOCK:
        ijkTuple = ijkTupleFromString(identifiers[INPUT_CELL_IJK]);
        m_cellI = std::get<0>(ijkTuple);
        m_cellJ = std::get<1>(ijkTuple);
        m_cellK = std::get<2>(ijkTuple);
        break;
    case SUMMARY_BLOCK_LGR:
        m_lgrName = identifiers[INPUT_LGR_NAME];
        ijkTuple = ijkTupleFromString(identifiers[INPUT_CELL_IJK]);
        m_cellI = std::get<0>(ijkTuple);
        m_cellJ = std::get<1>(ijkTuple);
        m_cellK = std::get<2>(ijkTuple);
        break;
    }

    // Set quantity for all categories
    m_quantityName = identifiers[INPUT_VECTOR_NAME];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fieldVarAddress(const std::string& fieldVarName)
{
    RifEclipseSummaryAddress fieldAddr;
    fieldAddr.m_variableCategory = SUMMARY_FIELD;
    fieldAddr.m_quantityName = fieldVarName;

    return fieldAddr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::uiText() const
{
    std::string text;
    text += m_quantityName;
    switch(this->category())
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            text += ":" + std::to_string(this->regionNumber());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            text += ":" + std::to_string(this->regionNumber());
            text += "-" + std::to_string(this->regionNumber2());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        {
            text += ":" + this->wellGroupName();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            text += ":" + this->wellName();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            text += ":" + this->wellName();
            text += ":" + std::to_string(this->cellI()) + ", "
                        + std::to_string(this->cellJ()) + ", "
                        + std::to_string(this->cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            text += ":" + this->lgrName();
            text += ":" + this->wellName();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            text += ":" + this->lgrName();
            text += ":" + this->wellName();
            text += ":" + std::to_string(this->cellI()) + ", " 
                        + std::to_string(this->cellJ()) + ", " 
                        + std::to_string(this->cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            text += ":" + this->wellName();
            text += ":" + std::to_string(this->wellSegmentNumber());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            text += ":" + std::to_string(this->cellI()) + ", "
                        + std::to_string(this->cellJ()) + ", "
                        + std::to_string(this->cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            text += ":" + this->lgrName();
            text += ":" + std::to_string(this->cellI()) + ", "
                        + std::to_string(this->cellJ()) + ", "
                        + std::to_string(this->cellK());
        }
        break;
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator==(const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second)
{
    if(first.category() != second.category()) return false;
    if(first.quantityName() != second.quantityName()) return false;
    switch(first.category())
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            if(first.regionNumber() != second.regionNumber()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            if(first.regionNumber() != second.regionNumber()) return false;
            if(first.regionNumber2() != second.regionNumber2()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        {
            if(first.wellGroupName() != second.wellGroupName()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            if(first.wellName() != second.wellName()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            if(first.wellName() != second.wellName()) return false;
            if(first.cellI() != second.cellI()) return false;
            if(first.cellJ() != second.cellJ()) return false;
            if(first.cellK() != second.cellK()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            if(first.wellName() != second.wellName()) return false;
            if(first.lgrName() != second.lgrName()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            if(first.wellName() != second.wellName()) return false;
            if(first.lgrName() != second.lgrName()) return false;
            if(first.cellI() != second.cellI()) return false;
            if(first.cellJ() != second.cellJ()) return false;
            if(first.cellK() != second.cellK()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            if(first.wellName() != second.wellName()) return false;
            if(first.wellSegmentNumber() != second.wellSegmentNumber()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            if(first.cellI() != second.cellI()) return false;
            if(first.cellJ() != second.cellJ()) return false;
            if(first.cellK() != second.cellK()) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            if(first.lgrName() != second.lgrName()) return false;
            if(first.cellI() != second.cellI()) return false;
            if(first.cellJ() != second.cellJ()) return false;
            if(first.cellK() != second.cellK()) return false;
        }
        break;

    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator<(const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second)
{
    if(first.category() != second.category())         return first.category() < second.category();
    if(first.quantityName() != second.quantityName()) return first.quantityName() < second.quantityName();

    switch(first.category())
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            if(first.regionNumber() != second.regionNumber()) return first.regionNumber() < second.regionNumber();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            if(first.regionNumber() != second.regionNumber()) return first.regionNumber() < second.regionNumber();
            if(first.regionNumber2() != second.regionNumber2()) return first.regionNumber2() < second.regionNumber2();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        {
            if(first.wellGroupName() != second.wellGroupName()) return first.wellGroupName() < second.wellGroupName();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            if(first.wellName() != second.wellName()) return (first.wellName() < second.wellName());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            if(first.wellName() != second.wellName()) return (first.wellName() < second.wellName());
            if(first.cellI() != second.cellI()) return (first.cellI() < second.cellI());
            if(first.cellJ() != second.cellJ()) return (first.cellJ() < second.cellJ());
            if(first.cellK() != second.cellK()) return (first.cellK() < second.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            if(first.wellName() != second.wellName()) return (first.wellName() < second.wellName());
            if(first.lgrName() != second.lgrName()) return (first.lgrName() < second.lgrName());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            if(first.wellName() != second.wellName()) return (first.wellName() < second.wellName());
            if(first.lgrName() != second.lgrName()) return (first.lgrName() < second.lgrName());
            if(first.cellI() != second.cellI()) return (first.cellI() < second.cellI());
            if(first.cellJ() != second.cellJ()) return (first.cellJ() < second.cellJ());
            if(first.cellK() != second.cellK()) return (first.cellK() < second.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            if(first.wellName() != second.wellName()) return (first.wellName() < second.wellName());
            if(first.wellSegmentNumber() != second.wellSegmentNumber()) return (first.wellSegmentNumber() < second.wellSegmentNumber());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            if(first.cellI() != second.cellI()) return (first.cellI() < second.cellI());
            if(first.cellJ() != second.cellJ()) return (first.cellJ() < second.cellJ());
            if(first.cellK() != second.cellK()) return (first.cellK() < second.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            if(first.lgrName() != second.lgrName()) return (first.lgrName() < second.lgrName());
            if(first.cellI() != second.cellI()) return (first.cellI() < second.cellI());
            if(first.cellJ() != second.cellJ()) return (first.cellJ() < second.cellJ());
            if(first.cellK() != second.cellK()) return (first.cellK() < second.cellK());
        }
        break;

    }
    return false;
}
