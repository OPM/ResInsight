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
#pragma once

#include <string>
#include <map>

//==================================================================================================
//
//
//==================================================================================================
class RifEclipseSummaryAddress
{
public:

    // Based on list in ecl_smspec.c and list of types taken from Eclipse Reference Manual ecl_rm_2011.1.pdf 
    enum SummaryVarCategory
    {
        SUMMARY_INVALID,
        SUMMARY_FIELD,              
        SUMMARY_AQUIFER,           
        SUMMARY_NETWORK,           
        SUMMARY_MISC,               
        SUMMARY_REGION,             
        SUMMARY_REGION_2_REGION,    
        SUMMARY_WELL_GROUP,         
        SUMMARY_WELL,               
        SUMMARY_WELL_COMPLETION,    
        SUMMARY_WELL_LGR,           
        SUMMARY_WELL_COMPLETION_LGR,
        SUMMARY_WELL_SEGMENT,       
        SUMMARY_BLOCK,              
        SUMMARY_BLOCK_LGR,          
    };

    enum SummaryIdentifierType
    {
        INPUT_REGION_NUMBER,
        INPUT_REGION_2_REGION,
        INPUT_WELL_NAME,
        INPUT_WELL_GROUP_NAME,
        INPUT_CELL_IJK,
        INPUT_LGR_NAME,
        INPUT_SEGMENT_NUMBER,
        INPUT_VECTOR_NAME
    };

public:

    RifEclipseSummaryAddress():
        m_variableCategory(RifEclipseSummaryAddress::SUMMARY_INVALID),
        m_regionNumber(-1),
        m_regionNumber2(-1),
        m_wellSegmentNumber(-1),
        m_cellI(-1),
        m_cellJ(-1),
        m_cellK(-1)
    {
    }
    
    RifEclipseSummaryAddress(SummaryVarCategory category, 
                             const std::string& quantityName, 
                             int                regionNumber,
                             int                regionNumber2,
                             const std::string& wellGroupName,
                             const std::string& wellName,
                             int                wellSegmentNumber,
                             const std::string& lgrName,
                             int                cellI,
                             int                cellJ,
                             int                cellK): 
        m_variableCategory(category),
        m_quantityName(quantityName),
        m_regionNumber(regionNumber),
        m_regionNumber2(regionNumber2),
        m_wellGroupName(wellGroupName),
        m_wellName(wellName),
        m_wellSegmentNumber(wellSegmentNumber),
        m_lgrName(lgrName),
        m_cellI(cellI),
        m_cellJ(cellJ),
        m_cellK(cellK)
    {
    }

    RifEclipseSummaryAddress(SummaryVarCategory category,
                             std::map<SummaryIdentifierType, std::string>& identifiers);
    
    // Static specialized creation methods

    static RifEclipseSummaryAddress fieldVarAddress(const std::string& fieldVarName);

    // Access methods

    SummaryVarCategory  category() const {  return m_variableCategory; }
    const std::string&  quantityName() const {  return m_quantityName; }

    int                 regionNumber() const  { return m_regionNumber; }
    int                 regionNumber2() const { return m_regionNumber2; }

    const std::string&  wellGroupName() const { return m_wellGroupName; }
    const std::string&  wellName() const      { return m_wellName; }
    int                 wellSegmentNumber() const { return m_wellSegmentNumber; }
    const std::string&  lgrName() const       { return m_lgrName; }
    int                 cellI() const         { return m_cellI; }
    int                 cellJ() const         { return m_cellJ; }
    int                 cellK() const         { return m_cellK; }

    // Derived properties

    std::string     uiText() const;
    std::string     uiText(RifEclipseSummaryAddress::SummaryIdentifierType itemTypeInput) const;

private:

    std::string                 formatUiTextIJK() const;
    std::tuple<int, int, int>   ijkTupleFromUiText(const std::string &s);
    std::string                 formatUiTextRegionToRegion() const;
    std::pair<int, int>         regionToRegionPairFromUiText(const std::string &s);

    SummaryVarCategory  m_variableCategory;
    std::string         m_quantityName;
    int                 m_regionNumber;
    int                 m_regionNumber2;
    std::string         m_wellGroupName;
    std::string         m_wellName;
    int                 m_wellSegmentNumber;
    std::string         m_lgrName;
    int                 m_cellI;
    int                 m_cellJ;
    int                 m_cellK;
};

bool operator==(const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second);

bool operator<(const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second);