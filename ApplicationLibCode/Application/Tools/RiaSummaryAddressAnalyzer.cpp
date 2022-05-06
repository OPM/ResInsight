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

#include "RiaSummaryAddressAnalyzer.h"
#include "RiaStdStringTools.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryAddressAnalyzer::RiaSummaryAddressAnalyzer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressAnalyzer::appendAddresses( const std::vector<RifEclipseSummaryAddress>& allAddresses )
{
    for ( const auto& adr : allAddresses )
    {
        analyzeSingleAddress( adr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressAnalyzer::appendAddresses( const std::set<RifEclipseSummaryAddress>& allAddresses )
{
    for ( const auto& adr : allAddresses )
    {
        analyzeSingleAddress( adr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::quantities() const
{
    return m_quantities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::quantityNamesWithHistory() const
{
    assignCategoryToQuantities();

    return m_quantitiesWithMatchingHistory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::quantityNamesNoHistory() const
{
    assignCategoryToQuantities();

    return m_quantitiesNoMatchingHistory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaSummaryAddressAnalyzer::quantityNameForTitle() const
{
    if ( quantities().size() == 1 )
    {
        return *quantities().begin();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::wellNames() const
{
    return keysInMap( m_wellNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::groupNames() const
{
    return keysInMap( m_groupNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryAddressAnalyzer::regionNumbers() const
{
    return keysInMap( m_regionNumbers );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::wellCompletions( const std::string& wellName ) const
{
    std::set<std::string> connections;

    for ( const auto& conn : m_wellCompletions )
    {
        if ( conn.first == wellName )
        {
            connections.insert( conn.second );
        }
    }

    return connections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryAddressAnalyzer::wellSegmentNumbers( const std::string& wellName ) const
{
    std::set<int> segmentNumberForWell;

    for ( const auto& wellSegment : m_wellSegmentNumbers )
    {
        if ( wellName.empty() || std::get<0>( wellSegment ) == wellName )
        {
            segmentNumberForWell.insert( std::get<1>( wellSegment ) );
        }
    }

    return segmentNumberForWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::blocks() const
{
    return keysInMap( m_blocks );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryAddressAnalyzer::aquifers() const
{
    return keysInMap( m_aquifers );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress::SummaryVarCategory> RiaSummaryAddressAnalyzer::categories() const
{
    return m_categories;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<RifEclipseSummaryAddress>> RiaSummaryAddressAnalyzer::addressesGroupedByObject() const
{
    auto wellAdr    = valuesInMap( m_wellNames );
    auto groupAdr   = valuesInMap( m_groupNames );
    auto regionAdr  = valuesInMap( m_regionNumbers );
    auto blockAdr   = valuesInMap( m_blocks );
    auto aquiferAdr = valuesInMap( m_aquifers );

    std::vector<std::vector<RifEclipseSummaryAddress>> groupedByObject;
    groupedByObject.insert( groupedByObject.end(), wellAdr.begin(), wellAdr.end() );
    groupedByObject.insert( groupedByObject.end(), groupAdr.begin(), groupAdr.end() );
    groupedByObject.insert( groupedByObject.end(), regionAdr.begin(), regionAdr.end() );
    groupedByObject.insert( groupedByObject.end(), blockAdr.begin(), blockAdr.end() );
    groupedByObject.insert( groupedByObject.end(), aquiferAdr.begin(), aquiferAdr.end() );

    return groupedByObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSummaryAddressAnalyzer::identifierTexts( RifEclipseSummaryAddress::SummaryVarCategory category,
                                                                 const std::string& secondaryIdentifier ) const
{
    std::vector<QString> identifierStrings;

    if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        auto keys = keysInMap( m_regionNumbers );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::number( key ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        auto keys = keysInMap( m_wellNames );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::fromStdString( key ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        auto keys = keysInMap( m_groupNames );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::fromStdString( key ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_BLOCK )
    {
        auto keys = keysInMap( m_blocks );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::fromStdString( key ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT )
    {
        auto segmentNumbers = wellSegmentNumbers( secondaryIdentifier );
        for ( const auto& segment : segmentNumbers )
        {
            identifierStrings.push_back( QString::number( segment ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION )
    {
        auto connections = wellCompletions( secondaryIdentifier );
        for ( const auto& conn : connections )
        {
            identifierStrings.push_back( QString::fromStdString( conn ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        auto keys = keysInMap( m_aquifers );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::number( key ) );
        }
    }

    return identifierStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress>
    RiaSummaryAddressAnalyzer::addressesForCategory( const std::set<RifEclipseSummaryAddress>&    addresses,
                                                     RifEclipseSummaryAddress::SummaryVarCategory category )
{
    std::vector<RifEclipseSummaryAddress> filteredAddresses;

    for ( const auto& adr : addresses )
    {
        if ( adr.category() == category )
        {
            filteredAddresses.push_back( adr );
        }
    }

    return filteredAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaSummaryAddressAnalyzer::correspondingHistorySummaryCurveName( const std::string& curveName )
{
    static std::string historyIdentifier = "H";

    if ( RiaStdStringTools::endsWith( curveName, historyIdentifier ) )
    {
        std::string candidate = curveName.substr( 0, curveName.size() - 1 );
        return candidate;
    }

    return curveName + historyIdentifier;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressAnalyzer::clear()
{
    m_quantities.clear();
    m_wellNames.clear();
    m_groupNames.clear();
    m_regionNumbers.clear();
    m_categories.clear();
    m_wellCompletions.clear();
    m_wellSegmentNumbers.clear();
    m_blocks.clear();
    m_aquifers.clear();

    m_quantitiesNoMatchingHistory.clear();
    m_quantitiesWithMatchingHistory.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressAnalyzer::assignCategoryToQuantities() const
{
    if ( !m_quantities.empty() )
    {
        if ( m_quantitiesWithMatchingHistory.empty() && m_quantitiesNoMatchingHistory.empty() )
        {
            computeQuantityNamesWithHistory();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressAnalyzer::computeQuantityNamesWithHistory() const
{
    m_quantitiesNoMatchingHistory.clear();
    m_quantitiesWithMatchingHistory.clear();

    const std::string historyIdentifier( "H" );

    for ( const auto& s : m_quantities )
    {
        std::string correspondingHistoryCurve = correspondingHistorySummaryCurveName( s );

        if ( m_quantities.find( correspondingHistoryCurve ) != m_quantities.end() )
        {
            // Insert the curve name without H
            if ( RiaStdStringTools::endsWith( s, historyIdentifier ) )
            {
                m_quantitiesWithMatchingHistory.insert( correspondingHistoryCurve );
            }
            else
            {
                m_quantitiesWithMatchingHistory.insert( s );
            }
        }
        else
        {
            m_quantitiesNoMatchingHistory.insert( s );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressAnalyzer::analyzeSingleAddress( const RifEclipseSummaryAddress& address )
{
    const std::string& wellName = address.wellName();

    if ( !wellName.empty() )
    {
        m_wellNames.insert( { wellName, address } );
    }

    if ( !address.vectorName().empty() )
    {
        m_quantities.insert( address.vectorName() );
    }

    if ( !address.groupName().empty() )
    {
        m_groupNames.insert( { address.groupName(), address } );
    }

    if ( address.regionNumber() != -1 )
    {
        m_regionNumbers.insert( { address.regionNumber(), address } );
    }

    if ( address.category() == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION )
    {
        auto wellNameAndCompletion = std::make_pair( wellName, address.blockAsString() );
        m_wellCompletions.insert( wellNameAndCompletion );
    }
    else if ( address.category() == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT )
    {
        auto wellNameAndSegment = std::make_pair( wellName, address.wellSegmentNumber() );
        m_wellSegmentNumbers.insert( wellNameAndSegment );
    }
    else if ( address.category() == RifEclipseSummaryAddress::SUMMARY_BLOCK )
    {
        auto text = address.blockAsString();

        m_blocks.insert( { text, address } );
    }
    else if ( address.category() == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        m_aquifers.insert( { address.aquiferNumber(), address } );
    }

    if ( address.category() != RifEclipseSummaryAddress::SUMMARY_INVALID )
    {
        m_categories.insert( address.category() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryAddressAnalyzer::keysInMap( const std::multimap<std::string, RifEclipseSummaryAddress>& map )
{
    std::set<std::string> keys;
    for ( const auto& [key, value] : map )
    {
        keys.insert( key );
    }
    return keys;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryAddressAnalyzer::keysInMap( const std::multimap<int, RifEclipseSummaryAddress>& map )
{
    std::set<int> keys;
    for ( const auto& [key, value] : map )
    {
        keys.insert( key );
    }
    return keys;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<RifEclipseSummaryAddress>>
    RiaSummaryAddressAnalyzer::valuesInMap( const std::multimap<std::string, RifEclipseSummaryAddress>& map )
{
    std::vector<std::vector<RifEclipseSummaryAddress>> groupedAddresses;

    auto uniqueKeys = keysInMap( map );
    for ( const auto& key : uniqueKeys )
    {
        auto range = map.equal_range( key );

        std::vector<RifEclipseSummaryAddress> addresses;
        for ( auto i = range.first; i != range.second; ++i )
        {
            addresses.push_back( i->second );
        }
        groupedAddresses.push_back( addresses );
    }

    return groupedAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<RifEclipseSummaryAddress>>
    RiaSummaryAddressAnalyzer::valuesInMap( const std::multimap<int, RifEclipseSummaryAddress>& map )
{
    std::vector<std::vector<RifEclipseSummaryAddress>> groupedAddresses;

    auto uniqueKeys = keysInMap( map );
    for ( const auto& key : uniqueKeys )
    {
        auto range = map.equal_range( key );

        std::vector<RifEclipseSummaryAddress> addresses;
        for ( auto i = range.first; i != range.second; ++i )
        {
            addresses.push_back( i->second );
        }
        groupedAddresses.push_back( addresses );
    }

    return groupedAddresses;
}
