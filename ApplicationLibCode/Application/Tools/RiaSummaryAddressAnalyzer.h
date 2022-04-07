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

#include <set>
#include <string>
#include <tuple>
#include <vector>

class RimSummaryCurveCollection;

class QString;

//==================================================================================================
//
//==================================================================================================
class RiaSummaryAddressAnalyzer
{
public:
    RiaSummaryAddressAnalyzer();

    void appendAddresses( const std::set<RifEclipseSummaryAddress>& allAddresses );
    void appendAddresses( const std::vector<RifEclipseSummaryAddress>& allAddresses );

    void clear();

    std::set<std::string> quantities() const;
    std::set<std::string> quantityNamesWithHistory() const;
    std::set<std::string> quantityNamesNoHistory() const;

    std::string quantityNameForTitle() const;

    std::set<std::string> wellNames() const;
    std::set<std::string> groupNames() const;
    std::set<int>         regionNumbers() const;

    std::set<std::string> wellCompletions( const std::string& wellName ) const;
    std::set<int>         wellSegmentNumbers( const std::string& wellName ) const;
    std::set<std::string> blocks() const;
    std::set<int>         aquifers() const;

    std::set<RifEclipseSummaryAddress::SummaryVarCategory> categories() const;
    std::vector<std::vector<RifEclipseSummaryAddress>>     addressesGroupedByObject() const;

    std::vector<QString> identifierTexts( RifEclipseSummaryAddress::SummaryVarCategory category,
                                          const std::string&                           secondaryIdentifier ) const;

    static std::vector<RifEclipseSummaryAddress>
        addressesForCategory( const std::set<RifEclipseSummaryAddress>&    addresses,
                              RifEclipseSummaryAddress::SummaryVarCategory category );

    static std::string correspondingHistorySummaryCurveName( const std::string& curveName );

private:
    void assignCategoryToQuantities() const;
    void computeQuantityNamesWithHistory() const;

    void analyzeSingleAddress( const RifEclipseSummaryAddress& address );

    static std::set<std::string> keysInMap( const std::multimap<std::string, RifEclipseSummaryAddress>& map );
    static std::set<int>         keysInMap( const std::multimap<int, RifEclipseSummaryAddress>& map );

    static std::vector<std::vector<RifEclipseSummaryAddress>>
        valuesInMap( const std::multimap<std::string, RifEclipseSummaryAddress>& map );

    static std::vector<std::vector<RifEclipseSummaryAddress>>
        valuesInMap( const std::multimap<int, RifEclipseSummaryAddress>& map );

private:
    std::set<std::string>         m_quantities;
    mutable std::set<std::string> m_quantitiesWithMatchingHistory;
    mutable std::set<std::string> m_quantitiesNoMatchingHistory;

    std::multimap<std::string, RifEclipseSummaryAddress> m_wellNames;
    std::multimap<std::string, RifEclipseSummaryAddress> m_groupNames;
    std::multimap<int, RifEclipseSummaryAddress>         m_regionNumbers;
    std::set<std::pair<std::string, std::string>>        m_wellCompletions;
    std::set<std::pair<std::string, int>>                m_wellSegmentNumbers;
    std::multimap<std::string, RifEclipseSummaryAddress> m_blocks;
    std::multimap<int, RifEclipseSummaryAddress>         m_aquifers;

    std::set<RifEclipseSummaryAddress::SummaryVarCategory> m_categories;
};
