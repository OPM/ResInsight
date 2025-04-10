/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RifEclipseSummaryAddress.h"
#include "Summary/RiaSummaryCurveAddress.h"

#include <set>
#include <string>
#include <tuple>
#include <vector>

class RimSummaryCurveCollection;

class QString;

// Concept to check if a type RangeType is an input range of elements convertible to type ElementType
// Example usage:
//
// Several overloaded functions
//  myFunction(std::vector<int>....);
//  myFunciton(std::set<int>....);
//  myFunction(std::list<int>....);
//
// Can be replaced by
//  template <input_range_of<int> RangeType>
//  void myFunction(RangeType&& range)
//
// This allows you to use the same function for all input ranges of int
//
template <typename RangeType, typename ElementType>
concept input_range_of = std::ranges::input_range<RangeType> && std::convertible_to<std::ranges::range_value_t<RangeType>, ElementType>;

//==================================================================================================
//
//==================================================================================================
class RiaSummaryAddressAnalyzer
{
public:
    RiaSummaryAddressAnalyzer();

    template <input_range_of<RifEclipseSummaryAddress> AddressRange>
    void appendAddresses( AddressRange&& addresses );

    void appendAddresses( const std::vector<RiaSummaryCurveAddress>& addresses );

    void clear();

    std::vector<std::string> quantities() const;

    bool isSingleQuantityIgnoreHistory() const;

    bool onlyCrossPlotCurves() const;

    std::string quantityNameForTitle() const;

    std::set<std::string> wellNames() const;
    std::set<std::string> groupNames() const;
    std::set<std::string> networkNames() const;
    std::set<int>         regionNumbers() const;

    std::set<std::string> wellConnections( const std::string& wellName ) const;
    std::set<int>         wellSegmentNumbers( const std::string& wellName ) const;
    std::set<std::string> blocks() const;
    std::set<int>         aquifers() const;
    std::set<int>         wellCompletionNumbers( const std::string& wellName ) const;

    std::set<RifEclipseSummaryAddressDefines::SummaryCategory> categories() const;
    std::vector<std::vector<RifEclipseSummaryAddress>>         addressesGroupedByObject() const;

    std::vector<QString> identifierTexts( RifEclipseSummaryAddressDefines::SummaryCategory category,
                                          const std::string&                               secondaryIdentifier ) const;

    static std::vector<RifEclipseSummaryAddress> addressesForCategory( const std::set<RifEclipseSummaryAddress>&        addresses,
                                                                       RifEclipseSummaryAddressDefines::SummaryCategory category );

    std::set<std::string> vectorNamesForCategory( RifEclipseSummaryAddressDefines::SummaryCategory category );

private:
    void analyzeSingleAddress( const RifEclipseSummaryAddress& address );

    static std::set<std::string> keysInMap( const std::multimap<std::string, RifEclipseSummaryAddress>& map );
    static std::set<int>         keysInMap( const std::multimap<int, RifEclipseSummaryAddress>& map );
    static std::set<RifEclipseSummaryAddressDefines::SummaryCategory>
        keysInMap( const std::map<RifEclipseSummaryAddressDefines::SummaryCategory, std::set<std::string>>& map );

    static std::vector<std::vector<RifEclipseSummaryAddress>> valuesInMap( const std::multimap<std::string, RifEclipseSummaryAddress>& map );

    static std::vector<std::vector<RifEclipseSummaryAddress>> valuesInMap( const std::multimap<int, RifEclipseSummaryAddress>& map );

private:
    std::vector<std::string> m_quantities;

    std::vector<RifEclipseSummaryAddress>                m_otherCategory;
    std::multimap<std::string, RifEclipseSummaryAddress> m_wellNames;
    std::multimap<std::string, RifEclipseSummaryAddress> m_groupNames;
    std::multimap<std::string, RifEclipseSummaryAddress> m_networkNames;
    std::multimap<int, RifEclipseSummaryAddress>         m_regionNumbers;
    std::set<std::pair<std::string, std::string>>        m_wellConnections;
    std::set<std::pair<std::string, int>>                m_wellSegmentNumbers;
    std::set<std::pair<std::string, int>>                m_wellCompletionNumbers;
    std::multimap<std::string, RifEclipseSummaryAddress> m_blocks;
    std::multimap<int, RifEclipseSummaryAddress>         m_aquifers;

    std::map<RifEclipseSummaryAddressDefines::SummaryCategory, std::set<std::string>> m_categories;

    bool m_onlyCrossPlotCurves;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <input_range_of<RifEclipseSummaryAddress> AddressRange>
void RiaSummaryAddressAnalyzer::appendAddresses( AddressRange&& addresses )
{
    for ( const auto& adr : addresses )
    {
        analyzeSingleAddress( adr );
    }
}
