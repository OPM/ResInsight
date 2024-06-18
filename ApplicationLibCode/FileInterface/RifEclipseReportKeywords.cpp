/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "RifEclipseReportKeywords.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseReportKeywords::appendKeywordCount( const std::string&                           keyword,
                                                   size_t                                       valueCount,
                                                   RifEclipseKeywordValueCount::KeywordDataType dataType )
{
    auto it = m_keywordValueCounts.find( keyword );

    if ( it == m_keywordValueCounts.end() )
    {
        m_keywordValueCounts[keyword] = RifEclipseKeywordValueCount( keyword, valueCount, dataType );
    }
    else
    {
        it->second.addValueCount( valueCount );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseReportKeywords::appendKeywordCount( const RifEclipseReportKeywords& other )
{
    for ( const auto& [keyword, keywordInfo] : other.m_keywordValueCounts )
    {
        appendKeywordCount( keyword, keywordInfo.valueCount(), keywordInfo.dataType() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseKeywordValueCount> RifEclipseReportKeywords::keywordValueCounts() const
{
    std::vector<RifEclipseKeywordValueCount> tmp;
    for ( const auto& [keyword, info] : m_keywordValueCounts )
    {
        tmp.push_back( info );
    }
    return tmp;
}
