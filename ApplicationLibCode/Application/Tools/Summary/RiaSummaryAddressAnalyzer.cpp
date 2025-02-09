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

#include "Summary/RiaSummaryAddressAnalyzer.h"
#include "RiaStdStringTools.h"

#include "Summary/RiaSummaryCurveDefinition.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"

#include <QString>

using namespace RifEclipseSummaryAddressDefines;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryAddressAnalyzer::RiaSummaryAddressAnalyzer()
    : m_onlyCrossPlotCurves( false )
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
void RiaSummaryAddressAnalyzer::appendAddresses( const std::vector<RiaSummaryCurveAddress>& addresses )
{
    // RiaSummaryCurveAddress can be used to represent cross plot curves. Count curves, and set the flag m_onlyCrossPlotCurves to true if
    // all curves are cross plot curves

    size_t crossPlotCurveCount = 0;

    for ( const auto& adr : addresses )
    {
        auto adrX = adr.summaryAddressX();
        if ( adrX.isValid() && adrX.category() != SummaryCategory::SUMMARY_TIME )
        {
            crossPlotCurveCount++;
        }

        // Use Y address first, to make sure the ordering of cross plot names is correct
        analyzeSingleAddress( adr.summaryAddressY() );
        analyzeSingleAddress( adr.summaryAddressX() );
    }

    m_onlyCrossPlotCurves = ( crossPlotCurveCount == addresses.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RiaSummaryAddressAnalyzer::quantities() const
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
bool RiaSummaryAddressAnalyzer::isSingleQuantityIgnoreHistory() const
{
    if ( quantities().size() == 1 ) return true;

    if ( quantities().size() == 2 && quantityNamesWithHistory().size() == 1 )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryAddressAnalyzer::onlyCrossPlotCurves() const
{
    return m_onlyCrossPlotCurves;
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

    if ( quantities().size() == 2 && m_onlyCrossPlotCurves )
    {
        // We have a cross plot with only one curve

        std::string title;
        for ( const auto& quantity : quantities() )
        {
            if ( !title.empty() ) title += " | ";
            title += quantity;
        }

        return title;
    }

    if ( quantities().size() == 2 && quantityNamesWithHistory().size() == 1 )
    {
        return *quantityNamesWithHistory().begin();
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
std::set<std::string> RiaSummaryAddressAnalyzer::networkNames() const
{
    return keysInMap( m_networkNames );
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
std::set<std::string> RiaSummaryAddressAnalyzer::wellConnections( const std::string& wellName ) const
{
    std::set<std::string> connections;

    for ( const auto& conn : m_wellConnections )
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
std::set<RifEclipseSummaryAddressDefines::SummaryCategory> RiaSummaryAddressAnalyzer::categories() const
{
    return keysInMap( m_categories );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<RifEclipseSummaryAddress>> RiaSummaryAddressAnalyzer::addressesGroupedByObject() const
{
    auto wellAdr    = valuesInMap( m_wellNames );
    auto groupAdr   = valuesInMap( m_groupNames );
    auto networkAdr = valuesInMap( m_networkNames );
    auto regionAdr  = valuesInMap( m_regionNumbers );
    auto blockAdr   = valuesInMap( m_blocks );
    auto aquiferAdr = valuesInMap( m_aquifers );

    std::vector<std::vector<RifEclipseSummaryAddress>> groupedByObject;
    groupedByObject.insert( groupedByObject.end(), wellAdr.begin(), wellAdr.end() );
    groupedByObject.insert( groupedByObject.end(), groupAdr.begin(), groupAdr.end() );
    groupedByObject.insert( groupedByObject.end(), networkAdr.begin(), networkAdr.end() );
    groupedByObject.insert( groupedByObject.end(), regionAdr.begin(), regionAdr.end() );
    groupedByObject.insert( groupedByObject.end(), blockAdr.begin(), blockAdr.end() );
    groupedByObject.insert( groupedByObject.end(), aquiferAdr.begin(), aquiferAdr.end() );

    groupedByObject.push_back( m_otherCategory );

    return groupedByObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSummaryAddressAnalyzer::identifierTexts( RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                                 const std::string& secondaryIdentifier ) const
{
    std::vector<QString> identifierStrings;

    if ( category == SummaryCategory::SUMMARY_REGION )
    {
        auto keys = keysInMap( m_regionNumbers );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::number( key ) );
        }
    }
    else if ( category == SummaryCategory::SUMMARY_WELL )
    {
        auto keys = keysInMap( m_wellNames );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::fromStdString( key ) );
        }
    }
    else if ( category == SummaryCategory::SUMMARY_GROUP )
    {
        auto keys = keysInMap( m_groupNames );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::fromStdString( key ) );
        }
    }
    else if ( category == SummaryCategory::SUMMARY_NETWORK )
    {
        auto keys = keysInMap( m_networkNames );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::fromStdString( key ) );
        }
    }
    else if ( category == SummaryCategory::SUMMARY_BLOCK )
    {
        auto keys = keysInMap( m_blocks );
        for ( const auto& key : keys )
        {
            identifierStrings.push_back( QString::fromStdString( key ) );
        }
    }
    else if ( category == SummaryCategory::SUMMARY_WELL_SEGMENT )
    {
        auto segmentNumbers = wellSegmentNumbers( secondaryIdentifier );
        for ( const auto& segment : segmentNumbers )
        {
            identifierStrings.push_back( QString::number( segment ) );
        }
    }
    else if ( category == SummaryCategory::SUMMARY_WELL_CONNECTION )
    {
        auto connections = wellConnections( secondaryIdentifier );
        for ( const auto& conn : connections )
        {
            identifierStrings.push_back( QString::fromStdString( conn ) );
        }
    }
    else if ( category == SummaryCategory::SUMMARY_AQUIFER )
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
std::vector<RifEclipseSummaryAddress> RiaSummaryAddressAnalyzer::addressesForCategory( const std::set<RifEclipseSummaryAddress>& addresses,
                                                                                       RifEclipseSummaryAddressDefines::SummaryCategory category )
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
std::set<std::string> RiaSummaryAddressAnalyzer::vectorNamesForCategory( RifEclipseSummaryAddressDefines::SummaryCategory category )
{
    auto it = m_categories.find( category );
    if ( it != m_categories.end() ) return it->second;

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressAnalyzer::clear()
{
    m_quantities.clear();
    m_wellNames.clear();
    m_groupNames.clear();
    m_networkNames.clear();
    m_regionNumbers.clear();
    m_categories.clear();
    m_wellConnections.clear();
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

        if ( std::find( m_quantities.begin(), m_quantities.end(), correspondingHistoryCurve ) != m_quantities.end() )
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
    if ( !address.isValid() ) return;

    if ( address.category() == SummaryCategory::SUMMARY_TIME )
    {
        // A time address has no other information than SummaryCategory::SUMMARY_TIME
        return;
    }

    const std::string& wellName = address.wellName();

    if ( !wellName.empty() )
    {
        m_wellNames.insert( { wellName, address } );
    }

    if ( !address.vectorName().empty() )
    {
        // The ordering of the quantities is used when creating titles of plots
        if ( std::find( m_quantities.begin(), m_quantities.end(), address.vectorName() ) == m_quantities.end() )
        {
            m_quantities.push_back( address.vectorName() );
        }
    }

    if ( !address.groupName().empty() )
    {
        m_groupNames.insert( { address.groupName(), address } );
    }

    if ( !address.networkName().empty() )
    {
        m_networkNames.insert( { address.networkName(), address } );
    }

    if ( address.category() == SummaryCategory::SUMMARY_REGION || address.category() == SummaryCategory::SUMMARY_REGION_2_REGION )
    {
        if ( address.regionNumber() != -1 )
        {
            m_regionNumbers.insert( { address.regionNumber(), address } );
        }
    }

    if ( address.category() == SummaryCategory::SUMMARY_WELL_CONNECTION )
    {
        auto wellNameAndCompletion = std::make_pair( wellName, address.blockAsString() );
        m_wellConnections.insert( wellNameAndCompletion );
    }
    else if ( address.category() == SummaryCategory::SUMMARY_WELL_SEGMENT )
    {
        auto wellNameAndSegment = std::make_pair( wellName, address.wellSegmentNumber() );
        m_wellSegmentNumbers.insert( wellNameAndSegment );
    }
    else if ( address.category() == SummaryCategory::SUMMARY_BLOCK )
    {
        auto text = address.blockAsString();

        m_blocks.insert( { text, address } );
    }
    else if ( address.category() == SummaryCategory::SUMMARY_AQUIFER )
    {
        m_aquifers.insert( { address.aquiferNumber(), address } );
    }
    else if ( address.category() == SummaryCategory::SUMMARY_FIELD || address.category() == SummaryCategory::SUMMARY_MISC )
    {
        m_otherCategory.push_back( address );
    }

    if ( address.category() != SummaryCategory::SUMMARY_INVALID )
    {
        if ( m_categories.count( address.category() ) == 0 )
        {
            m_categories[address.category()] = { address.vectorName() };
        }
        else
            m_categories[address.category()].insert( address.vectorName() );
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
std::set<RifEclipseSummaryAddressDefines::SummaryCategory>
    RiaSummaryAddressAnalyzer::keysInMap( const std::map<RifEclipseSummaryAddressDefines::SummaryCategory, std::set<std::string>>& map )
{
    std::set<RifEclipseSummaryAddressDefines::SummaryCategory> keys;
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
