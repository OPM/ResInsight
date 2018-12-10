/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RifEclipseRestartDataAccess.h"


//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseRestartDataAccess::RifEclipseRestartDataAccess()
{
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseRestartDataAccess::~RifEclipseRestartDataAccess()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifRestartReportKeywords::RifRestartReportKeywords()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifRestartReportKeywords::appendKeyword(const std::string& keyword, size_t itemCount, int globalIndex)
{
    m_keywordNameAndItemCount.push_back(RifKeywordLocation(keyword, itemCount, globalIndex));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, size_t> > RifRestartReportKeywords::keywordsWithAggregatedItemCount()
{
    std::vector<std::pair<std::string, size_t> > tmp;

    for (auto uni : uniqueKeywords())
    {
        size_t sum = 0;
        for (auto loc : objectsForKeyword(uni))
        {
            sum += loc.itemCount();
        }

        tmp.push_back(std::make_pair(uni, sum));
    }

    return tmp;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifKeywordLocation> RifRestartReportKeywords::objectsForKeyword(const std::string& keyword)
{
    std::vector<RifKeywordLocation> tmp;

    for (auto a : m_keywordNameAndItemCount)
    {
        if (a.keyword() == keyword)
        {
            tmp.push_back(a);
        }
    }

    return tmp;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::string> RifRestartReportKeywords::uniqueKeywords()
{
    std::set<std::string> unique;

    for (auto a : m_keywordNameAndItemCount)
    {
        unique.insert(a.keyword());
    }

    return unique;
}
