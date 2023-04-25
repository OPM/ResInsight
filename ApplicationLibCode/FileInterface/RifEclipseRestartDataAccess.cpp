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
void RifRestartReportKeywords::appendKeyword( const std::string& keyword, size_t itemCount, RifKeywordItemCount::KeywordDataType dataType )
{
    auto it = m_keywordNameAndItemCount.find( keyword );

    if ( it == m_keywordNameAndItemCount.end() )
    {
        m_keywordNameAndItemCount[keyword] = RifKeywordItemCount( keyword, itemCount, dataType );
    }
    else
    {
        it->second.addItemCount( itemCount );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRestartReportKeywords::appendKeyword( const RifRestartReportKeywords& other )
{
    for ( const auto& [keyword, keywordInfo] : other.m_keywordNameAndItemCount )
    {
        appendKeyword( keyword, keywordInfo.itemCount(), keywordInfo.dataType() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifKeywordItemCount> RifRestartReportKeywords::keywordItemCount() const
{
    std::vector<RifKeywordItemCount> tmp;
    for ( const auto& [keyword, info] : m_keywordNameAndItemCount )
    {
        tmp.push_back( info );
    }
    return tmp;
    ;
}
