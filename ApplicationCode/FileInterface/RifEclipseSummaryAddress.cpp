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
#include <QStringList>

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
/// Column header text format:   [<ER|ERR|ERROR>:]<VECTOR>:<CATEGORY_PARAM_NAME1>[:<CATEGORY_PARAM_NAME2>][....]
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fromEclipseTextAddress(const std::string& textAddress)
{
    QStringList names = QString().fromStdString(textAddress).split(QRegExp("[:,]"));

    bool isErrorResult = false;
    
    if (names.size() > 1 && names[0].trimmed().startsWith("ER", Qt::CaseInsensitive))
    {
        isErrorResult = true;
        names.pop_front();
    }
    else if (names.empty())
    {
        return RifEclipseSummaryAddress();
    }

    std::string quantityName = names[0].trimmed().toStdString();
    names.pop_front();

    SummaryVarCategory category = identifyCategory(quantityName);

    RifEclipseSummaryAddress address;
    switch (category)
    {
    case SUMMARY_FIELD:
        address = fieldAddress(quantityName);
        break;

    case SUMMARY_AQUIFER:
        if (names.size() > 0) address = aquiferAddress(quantityName,
                                                       RiaStdStringTools::toInt(names[0].toStdString()));
        break;

    case SUMMARY_NETWORK:
        address = networkAddress(quantityName);
        break;

    case SUMMARY_MISC:
        address = miscAddress(quantityName);
        break;

    case SUMMARY_REGION:
        if (names.size() > 0) address = regionAddress(quantityName,
                                                      RiaStdStringTools::toInt(names[0].toStdString()));
        break;

    case SUMMARY_REGION_2_REGION:
        if (names.size() > 0)
        {
            QStringList regions = names[0].trimmed().split("-");
            if (regions.size() == 2)
            {
                address = regionToRegionAddress(quantityName,
                                                RiaStdStringTools::toInt(regions[0].toStdString()),
                                                RiaStdStringTools::toInt(regions[1].toStdString()));
            }
        }
        break;

    case SUMMARY_WELL_GROUP:
        if (names.size() > 0) address = wellGroupAddress(quantityName,
                                                         names[0].toStdString());
        break;

    case SUMMARY_WELL:
        if (names.size() > 0) address = wellAddress(quantityName,
                                                    names[0].toStdString());
        break;

    case SUMMARY_WELL_COMPLETION:
        if (names.size() > 3) address = wellCompletionAddress(quantityName,
                                                              names[0].toStdString(),
                                                              RiaStdStringTools::toInt(names[1].toStdString()),
                                                              RiaStdStringTools::toInt(names[2].toStdString()),
                                                              RiaStdStringTools::toInt(names[3].toStdString()));
        break;

    case SUMMARY_WELL_LGR:
        if (names.size() > 1) address = wellLgrAddress(quantityName,
                                                       names[0].toStdString(),
                                                       names[1].toStdString());
        break;

    case SUMMARY_WELL_COMPLETION_LGR:
        if (names.size() > 4) address = wellCompletionLgrAddress(quantityName,
                                                                 names[0].toStdString(),
                                                                 names[1].toStdString(),
                                                                 RiaStdStringTools::toInt(names[2].toStdString()),
                                                                 RiaStdStringTools::toInt(names[3].toStdString()),
                                                                 RiaStdStringTools::toInt(names[4].toStdString()));
        break;

    case SUMMARY_WELL_SEGMENT:
        if (names.size() > 1) address = wellSegmentAddress(quantityName,
                                                           names[0].toStdString(),
                                                           RiaStdStringTools::toInt(names[1].toStdString()));
        break;

    case SUMMARY_BLOCK:
        if (names.size() > 2) address = blockAddress(quantityName,
                                                     RiaStdStringTools::toInt(names[0].toStdString()),
                                                     RiaStdStringTools::toInt(names[1].toStdString()),
                                                     RiaStdStringTools::toInt(names[2].toStdString()));
        break;

    case SUMMARY_BLOCK_LGR:
        if (names.size() > 3) address = blockLgrAddress(quantityName,
                                                        names[0].toStdString(),
                                                        RiaStdStringTools::toInt(names[1].toStdString()),
                                                        RiaStdStringTools::toInt(names[2].toStdString()),
                                                        RiaStdStringTools::toInt(names[3].toStdString()));
        break;

    case SUMMARY_CALCULATED:
        address =  calculatedAddress(quantityName);
        break;

    case SUMMARY_IMPORTED:
    case SUMMARY_INVALID:
        address = importedAddress(quantityName);
        break;

    default:
        break;
    }

    if (isErrorResult) address.setAsErrorResult();
    return address;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory RifEclipseSummaryAddress::identifyCategory(const std::string& quantityName)
{
    if (quantityName.size() == 0) return RifEclipseSummaryAddress::SUMMARY_INVALID;

    if (!RiaStdStringTools::containsOnlyLettersAndDigits(quantityName)) return RifEclipseSummaryAddress::SUMMARY_INVALID;

    if (quantityName.size() > 2 && quantityName[0] == 'R' && quantityName[2] == 'F')
    {
        return RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION;
    }

    char firstLetter = quantityName.at(0);

    if (firstLetter == 'A') return RifEclipseSummaryAddress::SUMMARY_AQUIFER;
    if (firstLetter == 'B') return RifEclipseSummaryAddress::SUMMARY_BLOCK;
    if (firstLetter == 'C') return RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
    if (firstLetter == 'F') return RifEclipseSummaryAddress::SUMMARY_FIELD;
    if (firstLetter == 'G') return RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
    if (firstLetter == 'N') return RifEclipseSummaryAddress::SUMMARY_NETWORK;
    if (firstLetter == 'R') return RifEclipseSummaryAddress::SUMMARY_REGION;
    if (firstLetter == 'S') return RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
    if (firstLetter == 'W') return RifEclipseSummaryAddress::SUMMARY_WELL;

    if (quantityName.size() < 2) return RifEclipseSummaryAddress::SUMMARY_INVALID;

    std::string firstTwoLetters = quantityName.substr(0, 2);

    if (firstTwoLetters == "LB") return RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR;
    if (firstTwoLetters == "LC") return RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR;
    if (firstTwoLetters == "LW") return RifEclipseSummaryAddress::SUMMARY_WELL_LGR;

    return RifEclipseSummaryAddress::SUMMARY_INVALID;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fieldAddress(const std::string& quantityName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_FIELD;
    addr.m_quantityName = quantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::aquiferAddress(const std::string& quantityName, int aquiferNumber)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_AQUIFER;
    addr.m_quantityName = quantityName;
    addr.m_aquiferNumber = aquiferNumber;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::networkAddress(const std::string& quantityName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_NETWORK;
    addr.m_quantityName = quantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::miscAddress(const std::string& quantityName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_MISC;
    addr.m_quantityName = quantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::regionAddress(const std::string& quantityName, int regionNumber)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_REGION;
    addr.m_quantityName = quantityName;
    addr.m_regionNumber = regionNumber;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::regionToRegionAddress(const std::string& quantityName, int regionNumber, int region2Number)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_REGION_2_REGION;
    addr.m_quantityName = quantityName;
    addr.m_regionNumber = regionNumber;
    addr.m_regionNumber2 = region2Number;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellGroupAddress(const std::string& quantityName, const std::string& wellGroupName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_GROUP;
    addr.m_quantityName = quantityName;
    addr.m_wellGroupName = wellGroupName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellAddress(const std::string& quantityName, const std::string& wellName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL;
    addr.m_quantityName = quantityName;
    addr.m_wellName = wellName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellCompletionAddress(const std::string& quantityName, const std::string& wellName, int i, int j, int k)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_COMPLETION;
    addr.m_quantityName = quantityName;
    addr.m_wellName = wellName;
    addr.m_cellI = i;
    addr.m_cellJ = j;
    addr.m_cellK = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellLgrAddress(const std::string& quantityName, const std::string& lgrName, const std::string& wellName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_LGR;
    addr.m_quantityName = quantityName;
    addr.m_lgrName = lgrName;
    addr.m_wellName = wellName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellCompletionLgrAddress(const std::string& quantityName, const std::string& lgrName, const std::string& wellName, int i, int j, int k)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_COMPLETION_LGR;
    addr.m_quantityName = quantityName;
    addr.m_lgrName = lgrName;
    addr.m_wellName = wellName;
    addr.m_cellI = i;
    addr.m_cellJ = j;
    addr.m_cellK = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellSegmentAddress(const std::string& quantityName, const std::string& wellName, int segmentNumber)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_SEGMENT;
    addr.m_quantityName = quantityName;
    addr.m_wellName = wellName;
    addr.m_wellSegmentNumber = segmentNumber;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::blockAddress(const std::string& quantityName, int i, int j, int k)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_BLOCK;
    addr.m_quantityName = quantityName;
    addr.m_cellI = i;
    addr.m_cellJ = j;
    addr.m_cellK = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::blockLgrAddress(const std::string& quantityName, const std::string& lgrName, int i, int j, int k)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_BLOCK_LGR;
    addr.m_quantityName = quantityName;
    addr.m_lgrName = lgrName;
    addr.m_cellI = i;
    addr.m_cellJ = j;
    addr.m_cellK = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::calculatedAddress(const std::string& quantityName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_CALCULATED;
    addr.m_quantityName = quantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::importedAddress(const std::string& quantityName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_IMPORTED;
    addr.m_quantityName = quantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::ensembleStatisticsAddress(const std::string& quantityName)
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_ENSEMBLE_STATISTICS;
    addr.m_quantityName = quantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::uiText() const
{
    std::string text;
    
    if (m_isErrorResult) text += "ERR:";

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
    case RifEclipseSummaryAddress::INPUT_VECTOR_NAME: return quantityName();
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
    QStringList ijk = QString().fromStdString(s).trimmed().split(QRegExp("[ ,.:;]"));
    
    if (ijk.size() != 3) return std::make_tuple(-1, -1, -1);

    return std::make_tuple(RiaStdStringTools::toInt(ijk[0].trimmed().toStdString()),
                           RiaStdStringTools::toInt(ijk[1].trimmed().toStdString()),
                           RiaStdStringTools::toInt(ijk[2].trimmed().toStdString()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::formatUiTextRegionToRegion() const
{
    return std::to_string(this->regionNumber()) + " - "
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
bool operator!=(const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second)
{
    return !operator==(first, second);
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

