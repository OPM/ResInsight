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

#include "RifGroupNetworkEnsembleStatistics.h"
#include "RifGroupNetworkSummaryData.h"
#include "RifSummaryReaderInterface.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace
{
using Statistic = RifGroupNetworkEnsembleStatistics::Statistic;
using Data      = RifGroupNetworkSummaryData;

//==================================================================================================
/// A realization with stored values
//==================================================================================================
class FakeRealization : public RifSummaryReaderInterface
{
public:
    explicit FakeRealization( std::vector<time_t> timeSteps )
        : m_timeSteps( std::move( timeSteps ) )
    {
    }

    void add( const RifEclipseSummaryAddress& address, const std::vector<double>& values )
    {
        m_values[address] = values;
        m_allResultAddresses.insert( address );
    }

    std::vector<time_t> timeSteps( const RifEclipseSummaryAddress& ) const override { return m_timeSteps; }

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
    std::vector<time_t>                                     m_timeSteps;
    std::map<RifEclipseSummaryAddress, std::vector<double>> m_values;
};

//==================================================================================================
/// A large realization with values computed on access, to avoid storing the values of hundreds of realizations
//==================================================================================================
class GeneratedRealization : public RifSummaryReaderInterface
{
public:
    GeneratedRealization( int realization, size_t timeStepCount, const std::set<RifEclipseSummaryAddress>& addresses )
        : m_realization( realization )
    {
        for ( size_t i = 0; i < timeStepCount; i++ )
        {
            m_timeSteps.push_back( static_cast<time_t>( i ) * 86400 * 30 );
        }
        m_allResultAddresses = addresses;
    }

    std::vector<time_t> timeSteps( const RifEclipseSummaryAddress& ) const override { return m_timeSteps; }

    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& address ) const override
    {
        if ( !hasAddress( address ) ) return { false, {} };

        const double        offset = static_cast<double>( std::hash<std::string>()( address.itemUiText() ) % 1000 );
        std::vector<double> values( m_timeSteps.size() );
        for ( size_t i = 0; i < values.size(); i++ )
        {
            values[i] = offset + m_realization + 0.01 * static_cast<double>( i );
        }
        return { true, values };
    }

    std::string                   unitName( const RifEclipseSummaryAddress& ) const override { return "SM3/DAY"; }
    RiaDefines::EclipseUnitSystem unitSystem() const override { return RiaDefines::EclipseUnitSystem::UNITS_METRIC; }
    size_t                        keywordCount() const override { return m_allResultAddresses.size(); }

private:
    int                 m_realization;
    std::vector<time_t> m_timeSteps;
};

template <typename T>
RifGroupNetworkEnsembleStatistics::ReadersProvider providerFor( const std::vector<std::unique_ptr<T>>& realizations )
{
    return [&realizations]()
    {
        std::vector<RifSummaryReaderInterface*> readers;
        for ( const auto& realization : realizations )
        {
            readers.push_back( realization.get() );
        }
        return readers;
    };
}

const RifEclipseSummaryAddress wopr = RifEclipseSummaryAddress::wellAddress( "WOPR", "P1" );
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkEnsembleStatistics, statistics )
{
    std::vector<std::unique_ptr<FakeRealization>> realizations;
    for ( double value : { 5.0, 3.0, 9.0, 1.0, 7.0, 2.0, 8.0, 4.0, 6.0 } )
    {
        auto realization = std::make_unique<FakeRealization>( std::vector<time_t>{ 100, 200 } );
        realization->add( wopr, { value, 2 * value } );
        realizations.push_back( std::move( realization ) );
    }

    const RifGroupNetworkEnsembleStatistics statistics( providerFor( realizations ) );
    EXPECT_EQ( 9u, statistics.realizationCount() );
    ASSERT_EQ( 2u, statistics.timeSteps().size() );

    const auto expect = [&]( Statistic statistic, double first, double second )
    {
        const auto* values = statistics.values( wopr, statistic );
        ASSERT_NE( nullptr, values );
        ASSERT_EQ( 2u, values->size() );
        EXPECT_DOUBLE_EQ( first, ( *values )[0] );
        EXPECT_DOUBLE_EQ( second, ( *values )[1] );
    };

    expect( Statistic::MEAN, 5.0, 10.0 );
    expect( Statistic::MINIMUM, 1.0, 2.0 );
    expect( Statistic::MAXIMUM, 9.0, 18.0 );
    expect( Statistic::P50, 5.0, 10.0 );

    // P10 is the high value, as for the ensemble statistics curves
    expect( Statistic::P10, 9.0, 18.0 );
    expect( Statistic::P90, 1.0, 2.0 );

    EXPECT_EQ( nullptr, statistics.values( RifEclipseSummaryAddress::wellAddress( "WOPR", "MISSING" ), Statistic::MEAN ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkEnsembleStatistics, differentTimeSteps )
{
    std::vector<std::unique_ptr<FakeRealization>> realizations;

    realizations.push_back( std::make_unique<FakeRealization>( std::vector<time_t>{ 100, 200, 300 } ) );
    realizations.back()->add( wopr, { 1, 2, 3 } );

    // Ends early
    realizations.push_back( std::make_unique<FakeRealization>( std::vector<time_t>{ 100, 200 } ) );
    realizations.back()->add( wopr, { 11, 12 } );

    // Other report steps, starting later
    realizations.push_back( std::make_unique<FakeRealization>( std::vector<time_t>{ 150, 300 } ) );
    realizations.back()->add( wopr, { 21, 22 } );

    // Without the vector
    realizations.push_back( std::make_unique<FakeRealization>( std::vector<time_t>{ 100, 200, 300 } ) );
    realizations.back()->add( RifEclipseSummaryAddress::fieldAddress( "FOPR" ), { 0, 0, 0 } );

    const RifGroupNetworkEnsembleStatistics statistics( providerFor( realizations ) );

    const std::vector<std::time_t> expectedTimeSteps = { 100, 150, 200, 300 };
    EXPECT_EQ( expectedTimeSteps, statistics.timeSteps() );

    // The value of the next time step is used between two time steps of a realization
    const auto* mean = statistics.values( wopr, Statistic::MEAN );
    ASSERT_NE( nullptr, mean );
    EXPECT_DOUBLE_EQ( ( 1.0 + 11.0 ) / 2, ( *mean )[0] );
    EXPECT_DOUBLE_EQ( ( 2.0 + 12.0 + 21.0 ) / 3, ( *mean )[1] );
    EXPECT_DOUBLE_EQ( ( 2.0 + 12.0 + 22.0 ) / 3, ( *mean )[2] );
    EXPECT_DOUBLE_EQ( ( 3.0 + 22.0 ) / 2, ( *mean )[3] );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkEnsembleStatistics, readersReplaced )
{
    std::vector<std::unique_ptr<FakeRealization>> realizations;
    realizations.push_back( std::make_unique<FakeRealization>( std::vector<time_t>{ 100 } ) );
    realizations.back()->add( wopr, { 1 } );

    const RifGroupNetworkEnsembleStatistics statistics( providerFor( realizations ) );
    ASSERT_NE( nullptr, statistics.values( wopr, Statistic::MEAN ) );
    EXPECT_DOUBLE_EQ( 1.0, ( *statistics.values( wopr, Statistic::MEAN ) )[0] );

    // A reloaded realization gives new values when the statistics are used again
    realizations.back() = std::make_unique<FakeRealization>( std::vector<time_t>{ 100 } );
    realizations.back()->add( wopr, { 3 } );
    realizations.push_back( std::make_unique<FakeRealization>( std::vector<time_t>{ 100 } ) );
    realizations.back()->add( wopr, { 5 } );

    statistics.prefetch( { wopr } );
    EXPECT_EQ( 2u, statistics.realizationCount() );
    EXPECT_DOUBLE_EQ( 4.0, ( *statistics.values( wopr, Statistic::MEAN ) )[0] );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkEnsembleStatistics, summaryDataForStatistic )
{
    std::vector<std::unique_ptr<FakeRealization>> realizations;
    for ( double value : { 1.0, 3.0 } )
    {
        auto realization = std::make_unique<FakeRealization>( std::vector<time_t>{ 100, 200 } );
        realization->add( wopr, { value, value } );
        realization->add( RifEclipseSummaryAddress::wellAddress( "WMCTL", "P1" ), { value, value } );
        realization->add( RifEclipseSummaryAddress::wellAddress( "WTHP", "P1" ), { 10 * value, 10 * value } );
        realizations.push_back( std::move( realization ) );
    }

    auto statistics = std::make_shared<RifGroupNetworkEnsembleStatistics>( providerFor( realizations ) );

    RifSummaryReaderInterface* reference = realizations.front().get();
    const Data data( [reference]() { return reference; }, RigGroupNetwork::TreeType::GRUPTREE, statistics, Statistic::MAXIMUM );
    EXPECT_TRUE( data.isStatistic() );
    EXPECT_EQ( 2u, data.timeSteps().size() );

    data.prefetch( { { "P1", true } }, { Data::EdgeMetric::OIL_RATE }, { Data::NodeMetric::PRESSURE, Data::NodeMetric::CONTROL_MODE } );
    EXPECT_EQ( 3.0, data.edgeValue( "P1", true, Data::EdgeMetric::OIL_RATE, 1 ) );
    EXPECT_EQ( 30.0, data.nodeValue( "P1", true, Data::NodeMetric::PRESSURE, 0 ) );
    EXPECT_EQ( "SM3/DAY", data.edgeUnit( Data::EdgeMetric::OIL_RATE ) );

    // Discrete values are not combined
    EXPECT_FALSE( data.nodeValue( "P1", true, Data::NodeMetric::CONTROL_MODE, 0 ).has_value() );
    EXPECT_FALSE( data.edgeValue( "P2", true, Data::EdgeMetric::OIL_RATE, 0 ).has_value() );
}

//--------------------------------------------------------------------------------------------------
/// The statistics of a large ensemble are computed once for all nodes, and changing the time step only reads cached values
//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkEnsembleStatistics, DISABLED_performanceOfLargeEnsemble )
{
    constexpr int    realizationCount = 400;
    constexpr int    nodeCount        = 300;
    constexpr size_t timeStepCount    = 500;

    std::set<RifEclipseSummaryAddress> addresses;
    std::vector<Data::NodeId>          nodes;
    for ( int i = 0; i < nodeCount; i++ )
    {
        const std::string well = "W" + std::to_string( i );
        nodes.emplace_back( well, true );
        addresses.insert( RifEclipseSummaryAddress::wellAddress( "WOPR", well ) );
        addresses.insert( RifEclipseSummaryAddress::wellAddress( "WTHP", well ) );
    }

    std::vector<std::unique_ptr<GeneratedRealization>> realizations;
    for ( int i = 0; i < realizationCount; i++ )
    {
        realizations.push_back( std::make_unique<GeneratedRealization>( i, timeStepCount, addresses ) );
    }

    auto statistics = std::make_shared<RifGroupNetworkEnsembleStatistics>( providerFor( realizations ) );

    RifSummaryReaderInterface* reference = realizations.front().get();
    const Data                 data( [reference]() { return reference; }, RigGroupNetwork::TreeType::GRUPTREE, statistics, Statistic::P50 );

    const auto start = std::chrono::steady_clock::now();
    data.prefetch( nodes, { Data::EdgeMetric::OIL_RATE }, { Data::NodeMetric::PRESSURE } );
    const auto prefetched = std::chrono::steady_clock::now();

    double sum = 0.0;
    for ( size_t timeStep = 0; timeStep < timeStepCount; timeStep++ )
    {
        for ( const auto& [name, isWell] : nodes )
        {
            sum += data.edgeValue( name, isWell, Data::EdgeMetric::OIL_RATE, timeStep ).value_or( 0.0 );
            sum += data.nodeValue( name, isWell, Data::NodeMetric::PRESSURE, timeStep ).value_or( 0.0 );
        }
    }
    const auto end = std::chrono::steady_clock::now();

    const auto prefetchMs  = std::chrono::duration_cast<std::chrono::milliseconds>( prefetched - start ).count();
    const auto timeStepsMs = std::chrono::duration_cast<std::chrono::milliseconds>( end - prefetched ).count();
    std::cout << "Statistics of " << realizationCount << " realizations, " << addresses.size() << " vectors and " << timeStepCount
              << " time steps: " << prefetchMs << " ms. Values of all nodes at all " << timeStepCount << " time steps: " << timeStepsMs
              << " ms\n";

    EXPECT_GT( sum, 0.0 );

    // The median of the realization offsets 0..399 is 199.5
    const auto* median = statistics->values( RifEclipseSummaryAddress::wellAddress( "WOPR", "W0" ), Statistic::P50 );
    const auto* mean   = statistics->values( RifEclipseSummaryAddress::wellAddress( "WOPR", "W0" ), Statistic::MEAN );
    ASSERT_NE( nullptr, median );
    ASSERT_NE( nullptr, mean );
    EXPECT_NEAR( ( *mean )[0], ( *median )[0], 1e-9 );

    // Loose limits, to catch a computation per time step or a sequential read without failing on slow build machines
    EXPECT_LT( prefetchMs, 30000 );
    EXPECT_LT( timeStepsMs / static_cast<long long>( timeStepCount ), 50 );
}
