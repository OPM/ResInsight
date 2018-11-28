/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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
//  for more details.B
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RifEclipseSummaryAddress.h"

#include <map>
#include <string>


class RiuSummaryVectorInfo
{
public:
    RiuSummaryVectorInfo() : category(RifEclipseSummaryAddress::SUMMARY_INVALID) {}
    RiuSummaryVectorInfo(RifEclipseSummaryAddress::SummaryVarCategory category, const std::string& longName)
        : category(category), longName(longName) {}

    RifEclipseSummaryAddress::SummaryVarCategory    category;
    std::string                                     longName;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuSummaryVectorDescriptionMap
{
public:
    static RiuSummaryVectorDescriptionMap* instance();

    RiuSummaryVectorInfo                         vectorInfo(const std::string& vectorName);
    std::string                                  vectorLongName(const std::string& vectorName,
                                                                bool returnVectorNameIfNotFound = false);

private:
    RiuSummaryVectorDescriptionMap();
    void populateFieldToInfoMap();

private:
    std::map<std::string, RiuSummaryVectorInfo> m_summaryToDescMap;
};
