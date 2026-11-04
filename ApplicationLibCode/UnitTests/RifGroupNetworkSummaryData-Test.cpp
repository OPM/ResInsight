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

#include "gtest/gtest.h"

#include "RifGroupNetworkSummaryData.h"
#include "RifSummaryReaderInterface.h"

#include <memory>

namespace
{
//==================================================================================================
class FakeSummaryReader : public RifSummaryReaderInterface
{
public:
    void add( const RifEclipseSummaryAddress& address, const std::vector<double>& values )
    {
        m_values[address] = values;
        m_allResultAddresses.insert( address );
    }

    std::vector<time_t> timeSteps( const RifEclipseSummaryAddress& ) const override { return { 100, 200, 300 }; }

    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& address ) const override
    {
        auto it = m_values.find( address );
        if ( it == m_values.end() ) return { false, {} };
        return { true, it->second };
    }

    std::string                   unitName( const RifEclipseSummaryAddress& ) const override { return "SM3/DAY"; }
    RiaDefines::EclipseUnitSystem unitSystem() const override { return RiaDefines::EclipseUnitSystem::UNITS_METRIC; }
    size_t                        keywordCount() const override { return m_values.size(); }

private:
    std::map<RifEclipseSummaryAddress, std::vector<double>> m_values;
};

using Data = RifGroupNetworkSummaryData;

Data::ReaderProvider providerFor( RifSummaryReaderInterface& reader )
{
    return [&reader]() { return &reader; };
}
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkSummaryData, valuesPerNodeType )
{
    FakeSummaryReader reader;
    reader.add( RifEclipseSummaryAddress::fieldAddress( "FOPR" ), { 1, 2, 3 } );
    reader.add( RifEclipseSummaryAddress::groupAddress( "GOPR", "G1" ), { 4, 5, 6 } );
    reader.add( RifEclipseSummaryAddress::groupAddress( "GOPRNB", "G1" ), { 7, 8, 9 } );
    reader.add( RifEclipseSummaryAddress::wellAddress( "WOPR", "P1" ), { 10, 11, 12 } );
    reader.add( RifEclipseSummaryAddress::wellAddress( "WTHP", "P1" ), { 13, 14, 15 } );

    const Data standard( providerFor( reader ), RigGroupNetwork::TreeType::GRUPTREE );
    ASSERT_EQ( 3u, standard.timeSteps().size() );
    EXPECT_EQ( 2.0, standard.edgeValue( "FIELD", false, Data::EdgeMetric::OIL_RATE, 1 ) );
    EXPECT_EQ( 5.0, standard.edgeValue( "G1", false, Data::EdgeMetric::OIL_RATE, 1 ) );
    EXPECT_EQ( 12.0, standard.edgeValue( "P1", true, Data::EdgeMetric::OIL_RATE, 2 ) );
    EXPECT_EQ( 13.0, standard.nodeValue( "P1", true, Data::NodeMetric::PRESSURE, 0 ) );

    // The extended network uses the network rates of the groups
    const Data extended( providerFor( reader ), RigGroupNetwork::TreeType::BRANPROP );
    EXPECT_EQ( 8.0, extended.edgeValue( "G1", false, Data::EdgeMetric::OIL_RATE, 1 ) );
    EXPECT_EQ( 11.0, extended.edgeValue( "P1", true, Data::EdgeMetric::OIL_RATE, 1 ) );

    // Missing vectors and time steps give no value
    EXPECT_FALSE( standard.edgeValue( "G1", false, Data::EdgeMetric::GAS_RATE, 0 ).has_value() );
    EXPECT_FALSE( standard.edgeValue( "G1", false, Data::EdgeMetric::OIL_RATE, 3 ).has_value() );
    EXPECT_FALSE( standard.nodeValue( "G1", false, Data::NodeMetric::BHP, 0 ).has_value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkSummaryData, classification )
{
    FakeSummaryReader reader;
    reader.add( RifEclipseSummaryAddress::groupAddress( "GOPR", "PROD" ), { 0, 5, 0 } );
    reader.add( RifEclipseSummaryAddress::groupAddress( "GWIR", "INJ" ), { 0, 0, 1 } );
    reader.add( RifEclipseSummaryAddress::groupAddress( "GOPR", "IDLE" ), { 0, 0, 0 } );
    reader.add( RifEclipseSummaryAddress::wellAddress( "WSTAT", "I1" ), { 2, 2, 2 } );
    reader.add( RifEclipseSummaryAddress::wellAddress( "WOPR", "I1" ), { 1, 1, 1 } );
    reader.add( RifEclipseSummaryAddress::wellAddress( "WOPR", "P1" ), { 1, 1, 1 } );

    const Data data( providerFor( reader ), RigGroupNetwork::TreeType::GRUPTREE );

    EXPECT_TRUE( data.category( "PROD", false ).isProducer );
    EXPECT_FALSE( data.category( "PROD", false ).isInjector );
    EXPECT_TRUE( data.category( "INJ", false ).isInjector );
    EXPECT_TRUE( data.category( "IDLE", false ).isOther() );

    // WSTAT is used for wells when present
    EXPECT_TRUE( data.category( "I1", true ).isInjector );
    EXPECT_FALSE( data.category( "I1", true ).isProducer );
    EXPECT_TRUE( data.category( "P1", true ).isProducer );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkSummaryData, noReader )
{
    const Data data( nullptr, RigGroupNetwork::TreeType::GRUPTREE );
    EXPECT_TRUE( data.timeSteps().empty() );
    EXPECT_FALSE( data.edgeValue( "FIELD", false, Data::EdgeMetric::OIL_RATE, 0 ).has_value() );
    EXPECT_TRUE( data.category( "FIELD", false ).isOther() );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkSummaryData, readerReplacedOrRemoved )
{
    auto first = std::make_unique<FakeSummaryReader>();
    first->add( RifEclipseSummaryAddress::fieldAddress( "FOPR" ), { 1, 2, 3 } );

    RifSummaryReaderInterface* current = first.get();
    const Data                 data( [&current]() { return current; }, RigGroupNetwork::TreeType::GRUPTREE );
    EXPECT_EQ( 2.0, data.edgeValue( "FIELD", false, Data::EdgeMetric::OIL_RATE, 1 ) );

    // A replaced reader gives the new values, not the cached values of the deleted reader
    auto second = std::make_unique<FakeSummaryReader>();
    second->add( RifEclipseSummaryAddress::fieldAddress( "FOPR" ), { 4, 5, 6 } );
    current = second.get();
    first.reset();
    EXPECT_EQ( 5.0, data.edgeValue( "FIELD", false, Data::EdgeMetric::OIL_RATE, 1 ) );

    current = nullptr;
    second.reset();
    EXPECT_TRUE( data.timeSteps().empty() );
    EXPECT_FALSE( data.edgeValue( "FIELD", false, Data::EdgeMetric::OIL_RATE, 1 ).has_value() );
}
