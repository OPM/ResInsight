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

#include "RiaSummaryCurveAnalyzer.h"
#include "RiaStdStringTools.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAnalyzer::RiaSummaryCurveAnalyzer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveAnalyzer::appendAddresses( const std::vector<RifEclipseSummaryAddress>& allAddresses )
{
    for ( const auto& adr : allAddresses )
    {
        analyzeSingleAddress( adr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveAnalyzer::appendAddresses( const std::set<RifEclipseSummaryAddress>& allAddresses )
{
    for ( const auto& adr : allAddresses )
    {
        analyzeSingleAddress( adr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::quantities() const
{
    return m_quantities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::quantityNamesWithHistory() const
{
    assignCategoryToQuantities();

    return m_quantitiesWithMatchingHistory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::quantityNamesNoHistory() const
{
    assignCategoryToQuantities();

    return m_quantitiesNoMatchingHistory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaSummaryCurveAnalyzer::quantityNameForTitle() const
{
    if ( quantityNamesWithHistory().size() == 1 && quantityNamesNoHistory().empty() )
    {
        return *quantityNamesWithHistory().begin();
    }

    if ( quantityNamesNoHistory().size() == 1 && quantityNamesWithHistory().empty() )
    {
        return *quantityNamesNoHistory().begin();
    }

    return std::string();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::wellNames() const
{
    return m_wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::wellGroupNames() const
{
    return m_wellGroupNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryCurveAnalyzer::regionNumbers() const
{
    return m_regionNumbers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::wellCompletions( const std::string& wellName ) const
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
std::set<int> RiaSummaryCurveAnalyzer::wellSegmentNumbers( const std::string& wellName ) const
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
std::set<std::string> RiaSummaryCurveAnalyzer::blocks() const
{
    return m_blocks;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryCurveAnalyzer::aquifers() const
{
    return m_aquifers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress::SummaryVarCategory> RiaSummaryCurveAnalyzer::categories() const
{
    return m_categories;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaSummaryCurveAnalyzer::identifierTexts( RifEclipseSummaryAddress::SummaryVarCategory category,
                                                               const std::string& secondaryIdentifier ) const
{
    std::vector<QString> identifierStrings;

    if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        for ( const auto& regionNumber : m_regionNumbers )
        {
            identifierStrings.push_back( QString::number( regionNumber ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        for ( const auto& wellName : m_wellNames )
        {
            identifierStrings.push_back( QString::fromStdString( wellName ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP )
    {
        for ( const auto& wellGroupName : m_wellGroupNames )
        {
            identifierStrings.push_back( QString::fromStdString( wellGroupName ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_BLOCK )
    {
        for ( const auto& ijkBlock : m_blocks )
        {
            identifierStrings.push_back( QString::fromStdString( ijkBlock ) );
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
        for ( const auto& aquifer : m_aquifers )
        {
            identifierStrings.push_back( QString::number( aquifer ) );
        }
    }

    return identifierStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress>
    RiaSummaryCurveAnalyzer::addressesForCategory( const std::set<RifEclipseSummaryAddress>&    addresses,
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
std::string RiaSummaryCurveAnalyzer::correspondingHistorySummaryCurveName( const std::string& curveName )
{
    static std::string historyIdentifier = "H";

    if ( RiaStdStringTools::endsWith( curveName, historyIdentifier ) )
    {
        std::string candidate = curveName.substr( 0, curveName.size() - 1 );
        return candidate;
    }
    else
    {
        return curveName + historyIdentifier;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveAnalyzer::clear()
{
    m_quantities.clear();
    m_wellNames.clear();
    m_wellGroupNames.clear();
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
void RiaSummaryCurveAnalyzer::assignCategoryToQuantities() const
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
void RiaSummaryCurveAnalyzer::computeQuantityNamesWithHistory() const
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
void RiaSummaryCurveAnalyzer::analyzeSingleAddress( const RifEclipseSummaryAddress& address )
{
    const std::string& wellName = address.wellName();

    if ( !wellName.empty() )
    {
        m_wellNames.insert( wellName );
    }

    if ( !address.quantityName().empty() )
    {
        m_quantities.insert( address.quantityName() );
    }

    if ( !address.wellGroupName().empty() )
    {
        m_wellGroupNames.insert( address.wellGroupName() );
    }

    if ( address.regionNumber() != -1 )
    {
        m_regionNumbers.insert( address.regionNumber() );
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
        m_blocks.insert( text );
    }
    else if ( address.category() == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        m_aquifers.insert( address.aquiferNumber() );
    }

    if ( address.category() != RifEclipseSummaryAddress::SUMMARY_INVALID )
    {
        m_categories.insert( address.category() );
    }
}
