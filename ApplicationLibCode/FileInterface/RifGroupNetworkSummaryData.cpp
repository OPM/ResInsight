/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifGroupNetworkSummaryData.h"

#include "RifSummaryReaderInterface.h"

#include <algorithm>

namespace
{
const std::string fieldNodeName = "FIELD";

// WSTAT values, see the Eclipse reference manual
constexpr int     wstatProducer        = 1;
constexpr int     wstatFirstInjector   = 2;
constexpr int     wstatLastInjector    = 4;
const std::string wellStatusVectorName = "WSTAT";
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifGroupNetworkSummaryData::NodeCategory::isOther() const
{
    return !isProducer && !isInjector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifGroupNetworkSummaryData::RifGroupNetworkSummaryData( ReaderProvider readerProvider, RigGroupNetwork::TreeType treeType )
    : m_readerProvider( std::move( readerProvider ) )
    , m_treeType( treeType )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::time_t>& RifGroupNetworkSummaryData::timeSteps() const
{
    reader();
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// The current reader. The serial number identifies the reader object, so a replaced reader is detected even if it is
/// allocated at the address of the previous one.
//--------------------------------------------------------------------------------------------------
const RifSummaryReaderInterface* RifGroupNetworkSummaryData::reader() const
{
    RifSummaryReaderInterface* currentReader = m_readerProvider ? m_readerProvider() : nullptr;
    const int                  serialNumber  = currentReader ? currentReader->serialNumber() : -1;
    if ( serialNumber == m_readerSerialNumber ) return currentReader;

    m_readerSerialNumber = serialNumber;
    m_cache.clear();
    m_nodeUnits.clear();
    m_timeSteps.clear();

    if ( currentReader )
    {
        currentReader->createAddressesIfRequired();

        const auto& addresses = currentReader->allResultAddresses();
        if ( !addresses.empty() )
        {
            const auto fopr = RifEclipseSummaryAddress::fieldAddress( "FOPR" );
            m_timeSteps     = currentReader->timeSteps( addresses.contains( fopr ) ? fopr : *addresses.begin() );
        }
    }

    return currentReader;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double>
    RifGroupNetworkSummaryData::edgeValue( const std::string& nodeName, bool isWell, EdgeMetric metric, size_t timeStepIndex ) const
{
    return valueAt( edgeAddress( nodeName, isWell, metric ), timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double>
    RifGroupNetworkSummaryData::nodeValue( const std::string& nodeName, bool isWell, NodeMetric metric, size_t timeStepIndex ) const
{
    return valueAt( nodeAddress( nodeName, isWell, metric ), timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifGroupNetworkSummaryData::edgeUnit( EdgeMetric metric ) const
{
    const auto* summaryReader = reader();
    if ( !summaryReader ) return {};
    return summaryReader->unitName( edgeAddress( fieldNodeName, false, metric ) );
}

//--------------------------------------------------------------------------------------------------
/// The unit of the first node with the vector. The result is cached, as all summary addresses are searched.
//--------------------------------------------------------------------------------------------------
std::string RifGroupNetworkSummaryData::nodeUnit( NodeMetric metric ) const
{
    const auto* summaryReader = reader();
    if ( !summaryReader ) return {};

    if ( auto it = m_nodeUnits.find( metric ); it != m_nodeUnits.end() ) return it->second;

    std::string unit;
    const auto  wellAddress  = nodeAddress( "", true, metric );
    const auto  groupAddress = nodeAddress( "", false, metric );
    for ( const auto& address : summaryReader->allResultAddresses() )
    {
        if ( address.vectorName() == wellAddress.vectorName() || address.vectorName() == groupAddress.vectorName() )
        {
            unit = summaryReader->unitName( address );
            break;
        }
    }

    m_nodeUnits[metric] = unit;
    return unit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifGroupNetworkSummaryData::NodeCategory RifGroupNetworkSummaryData::category( const std::string& nodeName, bool isWell ) const
{
    NodeCategory result;

    if ( isWell )
    {
        if ( const auto* status = values( address( wellStatusVectorName, nodeName, true ) ) )
        {
            for ( double value : *status )
            {
                const int wstat = static_cast<int>( value );
                if ( wstat == wstatProducer ) result.isProducer = true;
                if ( wstat >= wstatFirstInjector && wstat <= wstatLastInjector ) result.isInjector = true;
            }
            return result;
        }
    }

    for ( auto metric : { EdgeMetric::OIL_RATE, EdgeMetric::GAS_RATE, EdgeMetric::WATER_RATE } )
    {
        if ( hasPositiveValue( edgeAddress( nodeName, isWell, metric ) ) ) result.isProducer = true;
    }
    for ( auto metric : { EdgeMetric::WATER_INJ_RATE, EdgeMetric::GAS_INJ_RATE } )
    {
        if ( hasPositiveValue( edgeAddress( nodeName, isWell, metric ) ) ) result.isInjector = true;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifGroupNetworkSummaryData::edgeAddress( const std::string& nodeName, bool isWell, EdgeMetric metric ) const
{
    const bool        extended = m_treeType == RigGroupNetwork::TreeType::BRANPROP;
    const std::string prefix   = isWell ? "W" : ( nodeName == fieldNodeName ? "F" : "G" );

    // The extended network uses the network rates of the groups. Injection rates are kept, as nodes may also
    // be in an injection network (GNETINJE).
    const std::string networkSuffix = ( extended && prefix == "G" ) ? "NB" : "";

    switch ( metric )
    {
        case EdgeMetric::OIL_RATE:
            return address( prefix + "OPR" + networkSuffix, nodeName, isWell );
        case EdgeMetric::GAS_RATE:
            return address( prefix + "GPR" + networkSuffix, nodeName, isWell );
        case EdgeMetric::WATER_RATE:
            return address( prefix + "WPR" + networkSuffix, nodeName, isWell );
        case EdgeMetric::WATER_INJ_RATE:
            return address( prefix + "WIR", nodeName, isWell );
        case EdgeMetric::GAS_INJ_RATE:
            return address( prefix + "GIR", nodeName, isWell );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifGroupNetworkSummaryData::nodeAddress( const std::string& nodeName, bool isWell, NodeMetric metric ) const
{
    switch ( metric )
    {
        case NodeMetric::PRESSURE:
            return isWell ? RifEclipseSummaryAddress::wellAddress( "WTHP", nodeName )
                          : RifEclipseSummaryAddress::groupAddress( "GPR", nodeName );
        case NodeMetric::BHP:
            return isWell ? RifEclipseSummaryAddress::wellAddress( "WBHP", nodeName ) : RifEclipseSummaryAddress();
        case NodeMetric::CONTROL_MODE:
            return isWell ? RifEclipseSummaryAddress::wellAddress( "WMCTL", nodeName ) : RifEclipseSummaryAddress();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifGroupNetworkSummaryData::edgeMetricText( EdgeMetric metric )
{
    switch ( metric )
    {
        case EdgeMetric::OIL_RATE:
            return "Oil Rate";
        case EdgeMetric::GAS_RATE:
            return "Gas Rate";
        case EdgeMetric::WATER_RATE:
            return "Water Rate";
        case EdgeMetric::WATER_INJ_RATE:
            return "Water Injection Rate";
        case EdgeMetric::GAS_INJ_RATE:
            return "Gas Injection Rate";
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifGroupNetworkSummaryData::nodeMetricText( NodeMetric metric )
{
    switch ( metric )
    {
        case NodeMetric::PRESSURE:
            return "Pressure";
        case NodeMetric::BHP:
            return "Bottom Hole Pressure";
        case NodeMetric::CONTROL_MODE:
            return "Control Mode";
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RifGroupNetworkSummaryData::values( const RifEclipseSummaryAddress& address ) const
{
    const auto* summaryReader = reader();
    if ( !summaryReader || !address.isValid() ) return nullptr;

    auto it = m_cache.find( address );
    if ( it == m_cache.end() )
    {
        std::optional<std::vector<double>> cached;
        if ( summaryReader->hasAddress( address ) )
        {
            auto [ok, values] = summaryReader->values( address );
            if ( ok ) cached = std::move( values );
        }
        it = m_cache.emplace( address, std::move( cached ) ).first;
    }

    return it->second ? &it->second.value() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RifGroupNetworkSummaryData::valueAt( const RifEclipseSummaryAddress& address, size_t timeStepIndex ) const
{
    const auto* vectorValues = values( address );
    if ( !vectorValues || timeStepIndex >= vectorValues->size() ) return std::nullopt;
    return ( *vectorValues )[timeStepIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifGroupNetworkSummaryData::hasPositiveValue( const RifEclipseSummaryAddress& address ) const
{
    const auto* vectorValues = values( address );
    if ( !vectorValues ) return false;

    return std::any_of( vectorValues->begin(), vectorValues->end(), []( double value ) { return value > 0.0; } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifGroupNetworkSummaryData::address( const std::string& vectorName, const std::string& nodeName, bool isWell )
{
    if ( isWell ) return RifEclipseSummaryAddress::wellAddress( vectorName, nodeName );
    if ( nodeName == fieldNodeName ) return RifEclipseSummaryAddress::fieldAddress( vectorName );
    return RifEclipseSummaryAddress::groupAddress( vectorName, nodeName );
}
