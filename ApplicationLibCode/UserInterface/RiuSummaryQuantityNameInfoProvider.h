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

#include <string>
#include <unordered_map>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuSummaryQuantityNameInfoProvider
{
public:
    static RiuSummaryQuantityNameInfoProvider* instance();

    RifEclipseSummaryAddress::SummaryVarCategory categoryFromVectorName( const std::string& vectorName,
                                                                         bool               exactMatch = false ) const;
    std::string longNameFromVectorName( const std::string& vectorName, bool returnVectorNameIfNotFound = false ) const;

private:
    class RiuSummaryQuantityInfo
    {
    public:
        RiuSummaryQuantityInfo()
            : category( RifEclipseSummaryAddress::SUMMARY_INVALID )
        {
        }
        RiuSummaryQuantityInfo( RifEclipseSummaryAddress::SummaryVarCategory category, const std::string& longName )
            : category( category )
            , longName( longName )
        {
        }

        RifEclipseSummaryAddress::SummaryVarCategory category;
        std::string                                  longName;
    };

private:
    RiuSummaryQuantityNameInfoProvider();

    RiuSummaryQuantityInfo quantityInfo( const std::string& vectorName, bool exactMatch = false ) const;

    static std::unordered_map<std::string, RiuSummaryQuantityInfo> createInfoForEclipseKeywords();
    static std::unordered_map<std::string, RiuSummaryQuantityInfo> createInfoFor6xKeywords();

private:
    std::unordered_map<std::string, RiuSummaryQuantityInfo> m_summaryToDescMap;
};
