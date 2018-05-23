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

#include "RiaStdStringTools.h"

#include <QTextStream>

#include "cvfAssert.h"

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
    m_cellK(-1),
    m_aquiferNumber(-1),
    m_isErrorResult(false)
{
    std::tuple<int, int, int> ijkTuple;
    std::pair<int, int> reg2regPair;
    switch (category)
    {
    case SUMMARY_REGION:
        m_regionNumber = RiaStdStringTools::toInt(identifiers[INPUT_REGION_NUMBER]);
        break;
    case SUMMARY_REGION_2_REGION:
        reg2regPair = regionToRegionPairFromUiText(identifiers[INPUT_REGION_2_REGION]);
        m_regionNumber = reg2regPair.first;
        m_regionNumber2 = reg2regPair.second;
        break;
    case SUMMARY_WELL_GROUP:
        m_wellGroupName = identifiers[INPUT_WELL_GROUP_NAME];
        break;
    case SUMMARY_WELL:
        m_wellName = identifiers[INPUT_WELL_NAME];
        break;
    case SUMMARY_WELL_COMPLETION:
        m_wellName = identifiers[INPUT_WELL_NAME];
        ijkTuple = ijkTupleFromUiText(identifiers[INPUT_CELL_IJK]);
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
        ijkTuple = ijkTupleFromUiText(identifiers[INPUT_CELL_IJK]);
        m_cellI = std::get<0>(ijkTuple);
        m_cellJ = std::get<1>(ijkTuple);
        m_cellK = std::get<2>(ijkTuple);
        break;
    case SUMMARY_WELL_SEGMENT:
        m_wellName = identifiers[INPUT_WELL_NAME];
        m_wellSegmentNumber = RiaStdStringTools::toInt(identifiers[INPUT_SEGMENT_NUMBER]);
    case SUMMARY_BLOCK:
        ijkTuple = ijkTupleFromUiText(identifiers[INPUT_CELL_IJK]);
        m_cellI = std::get<0>(ijkTuple);
        m_cellJ = std::get<1>(ijkTuple);
        m_cellK = std::get<2>(ijkTuple);
        break;
    case SUMMARY_BLOCK_LGR:
        m_lgrName = identifiers[INPUT_LGR_NAME];
        ijkTuple = ijkTupleFromUiText(identifiers[INPUT_CELL_IJK]);
        m_cellI = std::get<0>(ijkTuple);
        m_cellJ = std::get<1>(ijkTuple);
        m_cellK = std::get<2>(ijkTuple);
        break;
    case SUMMARY_AQUIFER:
        m_aquiferNumber = RiaStdStringTools::toInt(identifiers[INPUT_AQUIFER_NUMBER]);
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
RifEclipseSummaryAddress RifEclipseSummaryAddress::calculatedCurveAddress(const std::string& curveName)
{
    RifEclipseSummaryAddress fieldAddr;
    fieldAddr.m_variableCategory = SUMMARY_CALCULATED;
    fieldAddr.m_quantityName = curveName;

    return fieldAddr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::importedAddress(const std::string& quantityName)
{
    RifEclipseSummaryAddress fieldAddr;
    fieldAddr.m_variableCategory = SUMMARY_IMPORTED;
    fieldAddr.m_quantityName = quantityName;

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
            text += ":" + formatUiTextRegionToRegion();
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
            text += ":" + formatUiTextIJK();
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
            text += ":" + formatUiTextIJK();
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
            text += ":" + formatUiTextIJK();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            text += ":" + this->lgrName();
            text += ":" + formatUiTextIJK();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        {
            text += ":" + std::to_string(this->aquiferNumber());
        }
        break;
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// Returns the stringified value for the specified identifier type
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::uiText(RifEclipseSummaryAddress::SummaryIdentifierType identifierType) const
{
    switch (identifierType)
    {
    case RifEclipseSummaryAddress::INPUT_REGION_NUMBER: return std::to_string(regionNumber());
    case RifEclipseSummaryAddress::INPUT_REGION_2_REGION: return formatUiTextRegionToRegion();
    case RifEclipseSummaryAddress::INPUT_WELL_NAME: return wellName();
    case RifEclipseSummaryAddress::INPUT_WELL_GROUP_NAME: return wellGroupName();
    case RifEclipseSummaryAddress::INPUT_CELL_IJK: return formatUiTextIJK();
    case RifEclipseSummaryAddress::INPUT_LGR_NAME: return lgrName();
    case RifEclipseSummaryAddress::INPUT_SEGMENT_NUMBER: return std::to_string(wellSegmentNumber());
    case RifEclipseSummaryAddress::INPUT_AQUIFER_NUMBER: return std::to_string(aquiferNumber());
    case RifEclipseSummaryAddress::INPUT_VECTOR_NAME: return quantityName() + (m_isErrorResult ? " (err)" : "");
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isValid() const
{
    switch (category())
    {
    case SUMMARY_INVALID:
        return false;

    case SUMMARY_REGION:
        if (m_regionNumber == -1) return false;
        return true;

    case SUMMARY_REGION_2_REGION:
        if (m_regionNumber == -1) return false;
        if (m_regionNumber2 == -1) return false;
        return true;

    case SUMMARY_WELL_GROUP:
        if (m_wellGroupName.size() == 0) return false;
        return true;

    case SUMMARY_WELL:
        if (m_wellName.size() == 0) return false;
        return true;

    case SUMMARY_WELL_COMPLETION:
        if (m_wellName.size() == 0) return false;
        if (m_cellI == -1) return false;
        if (m_cellJ == -1) return false;
        if (m_cellK == -1) return false;
        return true;

    case SUMMARY_WELL_LGR:
        if (m_lgrName.size() == 0) return false;
        if (m_wellName.size() == 0) return false;
        return true;

    case SUMMARY_WELL_COMPLETION_LGR:
        if (m_lgrName.size() == 0) return false;
        if (m_wellName.size() == 0) return false;
        if (m_cellI == -1) return false;
        if (m_cellJ == -1) return false;
        if (m_cellK == -1) return false;
        return true;

    case SUMMARY_WELL_SEGMENT:
        if (m_wellName.size() == 0) return false;
        if (m_wellSegmentNumber == -1) return false;
        return true;

    case SUMMARY_BLOCK:
        if (m_cellI == -1) return false;
        if (m_cellJ == -1) return false;
        if (m_cellK == -1) return false;
        return true;

    case SUMMARY_BLOCK_LGR:
        if (m_lgrName.size() == 0) return false;
        if (m_cellI == -1) return false;
        if (m_cellJ == -1) return false;
        if (m_cellK == -1) return false;
        return true;

    case SUMMARY_AQUIFER:
        if (m_aquiferNumber == -1) return false;
        return true;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::formatUiTextIJK() const
{
    return std::to_string(this->cellI()) + ", "
        + std::to_string(this->cellJ()) + ", "
        + std::to_string(this->cellK());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::tuple<int, int, int> RifEclipseSummaryAddress::ijkTupleFromUiText(const std::string &s)
{
    auto firstSep = s.find(',');
    auto lastSep = s.find(',', firstSep + 1);
    CVF_ASSERT(firstSep != std::string::npos && lastSep != std::string::npos);
    auto textI = s.substr(0, firstSep);
    auto textJ = s.substr(firstSep + 1, lastSep - firstSep - 1);
    auto textK = s.substr(lastSep + 1);

    return std::make_tuple(RiaStdStringTools::toInt(textI), RiaStdStringTools::toInt(textJ), RiaStdStringTools::toInt(textK));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::formatUiTextRegionToRegion() const
{
    return std::to_string(this->regionNumber()) + " -> "
        + std::to_string(this->regionNumber2());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RifEclipseSummaryAddress::regionToRegionPairFromUiText(const std::string &s)
{
    auto sep = s.find("->");
    CVF_ASSERT(sep != std::string::npos );
    auto textReg = s.substr(0, sep);
    auto textReg2 = s.substr(sep + 2);
    
    return std::make_pair(RiaStdStringTools::toInt(textReg), RiaStdStringTools::toInt(textReg2));
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
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        {
            if (first.aquiferNumber() != second.aquiferNumber()) return false;
        }
        break;
    }
    if (first.isErrorResult() != second.isErrorResult()) return false;
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
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        {
            if (first.aquiferNumber() != second.aquiferNumber()) return first.aquiferNumber() < second.aquiferNumber();
        }
        break;

    }
    if (first.isErrorResult() != second.isErrorResult()) return first.isErrorResult() < second.isErrorResult();
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator << (QTextStream& str, const RifEclipseSummaryAddress& sobj)
{
    CVF_ASSERT(false);
    return str;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator >> (QTextStream& str, RifEclipseSummaryAddress& sobj)
{
    CVF_ASSERT(false);
    return str;
}

