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
        SUMMARY_WELL_SEGMENT_RIVER, 
        SUMMARY_BLOCK,              
        SUMMARY_BLOCK_LGR,          
    };

public:
    RifEclipseSummaryAddress(const std::string& ertSummaryVarId);
    RifEclipseSummaryAddress(SummaryVarCategory category, const std::string& simulationItemName, const std::string& quantityName);
    


    SummaryVarCategory  category() const;
    std::string         simulationItemName() const;
    std::string         quantityName() const;
    
    std::string         ertSummaryVarId() const;
    
    static std::string  categoryName(SummaryVarCategory category);


private:
    static std::string  toErtSummaryVarId(SummaryVarCategory category, const std::string& simulationItemName, const std::string& quantityName);
    static void         fromErtSummaryVarId(const std::string& ertSummaryVarId, SummaryVarCategory* category, std::string* simulationItemName, std::string* quantityName);

    static std::string          prefixForCategory(SummaryVarCategory category);
    static SummaryVarCategory   categoryFromErtSummaryVarId(const std::string& ertSummaryVarId);

private:
    std::string         m_ertSummaryVarId;
    std::string         m_simulationItemName;

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
