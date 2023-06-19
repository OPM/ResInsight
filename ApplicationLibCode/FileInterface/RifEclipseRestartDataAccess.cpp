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
void RifRestartReportKeywords::appendKeywordCount( const std::string& keyword, size_t valueCount, RifKeywordValueCount::KeywordDataType dataType )
{
    auto it = m_keywordValueCounts.find( keyword );

    if ( it == m_keywordValueCounts.end() )
    {
        m_keywordValueCounts[keyword] = RifKeywordValueCount( keyword, valueCount, dataType );
    }
    else
    {
        it->second.addValueCount( valueCount );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRestartReportKeywords::appendKeywordCount( const RifRestartReportKeywords& other )
{
    for ( const auto& [keyword, keywordInfo] : other.m_keywordValueCounts )
    {
        appendKeywordCount( keyword, keywordInfo.valueCount(), keywordInfo.dataType() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifKeywordValueCount> RifRestartReportKeywords::keywordValueCounts() const
{
    std::vector<RifKeywordValueCount> tmp;
    for ( const auto& [keyword, info] : m_keywordValueCounts )
    {
        tmp.push_back( info );
    }
    return tmp;
}
