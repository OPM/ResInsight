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

#include "RifGroupNetworkEnsembleStatistics.h"

#include "RiaLogging.h"

#include "RifSummaryReaderInterface.h"

#include "RigStatisticsMath.h"

#include <QString>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <set>

namespace
{
// The values of the realizations are read in chunks of addresses, to limit the memory used for large ensembles
constexpr size_t maxChunkValueCount = 16 * 1024 * 1024;
constexpr size_t maxChunkSize       = 64;

std::vector<std::time_t> realizationTimeSteps( const RifSummaryReaderInterface& reader )
{
    const auto& addresses = reader.allResultAddresses();
    if ( addresses.empty() ) return {};

    const auto fopr = RifEclipseSummaryAddress::fieldAddress( "FOPR" );
    return reader.timeSteps( addresses.contains( fopr ) ? fopr : *addresses.begin() );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifGroupNetworkEnsembleStatistics::RifGroupNetworkEnsembleStatistics( ReadersProvider readersProvider )
    : m_readersProvider( std::move( readersProvider ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::time_t>& RifGroupNetworkEnsembleStatistics::timeSteps() const
{
    updateRealizations();
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifGroupNetworkEnsembleStatistics::realizationCount() const
{
    updateRealizations();
    return m_realizations.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifGroupNetworkEnsembleStatistics::prefetch( const std::vector<RifEclipseSummaryAddress>& addresses ) const
{
    updateRealizations();

    std::set<RifEclipseSummaryAddress> missing;
    for ( const auto& address : addresses )
    {
        if ( address.isValid() && !m_cache.contains( address ) ) missing.insert( address );
    }
    if ( missing.empty() ) return;

    computeStatistics( std::vector<RifEclipseSummaryAddress>( missing.begin(), missing.end() ) );
}

//--------------------------------------------------------------------------------------------------
/// The cached values are returned without asking for the readers, as this is called for each node of the network
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RifGroupNetworkEnsembleStatistics::values( const RifEclipseSummaryAddress& address, Statistic statistic ) const
{
    if ( !address.isValid() ) return nullptr;

    auto it = m_cache.find( address );
    if ( it == m_cache.end() )
    {
        prefetch( { address } );
        it = m_cache.find( address );
        if ( it == m_cache.end() ) return nullptr;
    }

    if ( !it->second ) return nullptr;
    return &( *it->second )[static_cast<size_t>( statistic )];
}

//--------------------------------------------------------------------------------------------------
/// Update the time steps and the realizations if any reader has changed. The serial number identifies the reader object, so a
/// replaced reader is detected even if it is allocated at the address of the previous one.
//--------------------------------------------------------------------------------------------------
void RifGroupNetworkEnsembleStatistics::updateRealizations() const
{
    const std::vector<RifSummaryReaderInterface*> readers = m_readersProvider ? m_readersProvider()
                                                                              : std::vector<RifSummaryReaderInterface*>();

    std::vector<int> serialNumbers;
    serialNumbers.reserve( readers.size() );
    for ( const auto* reader : readers )
    {
        serialNumbers.push_back( reader ? reader->serialNumber() : -1 );
    }

    if ( serialNumbers == m_serialNumbers && !m_realizations.empty() )
    {
        // The reader objects are the same, but the pointers are refreshed as they are only valid until the next provider call
        for ( size_t i = 0; i < readers.size(); i++ )
        {
            m_realizations[i].reader = readers[i];
        }
        return;
    }

    m_serialNumbers = serialNumbers;
    m_cache.clear();
    m_realizations.clear();
    m_timeSteps.clear();

    std::vector<std::vector<std::time_t>> allTimeSteps( readers.size() );
    std::set<std::time_t>                 unionOfTimeSteps;
    for ( size_t i = 0; i < readers.size(); i++ )
    {
        if ( !readers[i] ) continue;

        // Create the addresses here, as the readers are read from multiple threads later
        readers[i]->createAddressesIfRequired();
        allTimeSteps[i] = realizationTimeSteps( *readers[i] );
        unionOfTimeSteps.insert( allTimeSteps[i].begin(), allTimeSteps[i].end() );
    }

    m_timeSteps.assign( unionOfTimeSteps.begin(), unionOfTimeSteps.end() );

    m_realizations.resize( readers.size() );
    for ( size_t i = 0; i < readers.size(); i++ )
    {
        Realization& realization = m_realizations[i];
        realization.reader       = readers[i];

        const auto& steps      = allTimeSteps[i];
        realization.valueCount = steps.size();
        realization.valueIndices.assign( m_timeSteps.size(), -1 );
        if ( steps.empty() ) continue;

        if ( steps == m_timeSteps )
        {
            for ( size_t t = 0; t < m_timeSteps.size(); t++ )
            {
                realization.valueIndices[t] = static_cast<int>( t );
            }
            continue;
        }

        for ( size_t t = 0; t < m_timeSteps.size(); t++ )
        {
            const std::time_t time = m_timeSteps[t];
            if ( time < steps.front() || time > steps.back() ) continue;

            // The first time step at or after the time, as step right interpolation
            const auto it               = std::lower_bound( steps.begin(), steps.end(), time );
            realization.valueIndices[t] = static_cast<int>( it - steps.begin() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Each reader is used by one thread only, as the readers are not thread safe
//--------------------------------------------------------------------------------------------------
void RifGroupNetworkEnsembleStatistics::computeStatistics( const std::vector<RifEclipseSummaryAddress>& addresses ) const
{
    const size_t realizationCount = m_realizations.size();
    const size_t timeStepCount    = m_timeSteps.size();
    if ( realizationCount == 0 || timeStepCount == 0 )
    {
        for ( const auto& address : addresses )
        {
            m_cache[address] = std::nullopt;
        }
        return;
    }

    const size_t chunkSize = std::clamp<size_t>( maxChunkValueCount / ( realizationCount * timeStepCount ), 1, maxChunkSize );

    for ( size_t chunkStart = 0; chunkStart < addresses.size(); chunkStart += chunkSize )
    {
        const size_t chunkEnd     = std::min( chunkStart + chunkSize, addresses.size() );
        const int    addressCount = static_cast<int>( chunkEnd - chunkStart );

        // The values of each address and realization: values[address][realization]
        std::vector<std::vector<std::vector<double>>> values( addressCount, std::vector<std::vector<double>>( realizationCount ) );
        std::atomic<int>                              failedReadCount = 0;

#pragma omp parallel for schedule( dynamic )
        for ( int realizationIndex = 0; realizationIndex < static_cast<int>( realizationCount ); realizationIndex++ )
        {
            const Realization& realization = m_realizations[realizationIndex];
            if ( !realization.reader || realization.valueCount == 0 ) continue;

            for ( int addressIndex = 0; addressIndex < addressCount; addressIndex++ )
            {
                const auto& address = addresses[chunkStart + addressIndex];
                try
                {
                    if ( !realization.reader->hasAddress( address ) ) continue;

                    auto [isOk, vectorValues] = realization.reader->values( address );
                    if ( isOk && vectorValues.size() == realization.valueCount )
                    {
                        values[addressIndex][realizationIndex] = std::move( vectorValues );
                    }
                }
                catch ( ... )
                {
                    // A realization that fails to read is left out of the statistics
                    failedReadCount++;
                }
            }
        }

        if ( failedReadCount > 0 )
        {
            RiaLogging::warning( QString( "Production network: Failed to read %1 summary vector(s) from realizations while computing "
                                          "ensemble statistics. The statistics are based on the realizations that could be read." )
                                     .arg( failedReadCount.load() )
                                     .toStdString() );
        }

        std::vector<std::optional<Statistics>> statistics( addressCount );

#pragma omp parallel for schedule( dynamic )
        for ( int addressIndex = 0; addressIndex < addressCount; addressIndex++ )
        {
            statistics[addressIndex] = statisticsForAddress( m_realizations, values[addressIndex], timeStepCount );
        }

        for ( int addressIndex = 0; addressIndex < addressCount; addressIndex++ )
        {
            m_cache[addresses[chunkStart + addressIndex]] = std::move( statistics[addressIndex] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RifGroupNetworkEnsembleStatistics::Statistics>
    RifGroupNetworkEnsembleStatistics::statisticsForAddress( const std::vector<Realization>&         realizations,
                                                             const std::vector<std::vector<double>>& realizationValues,
                                                             size_t                                  timeStepCount )
{
    const bool hasValues =
        std::any_of( realizationValues.begin(), realizationValues.end(), []( const std::vector<double>& values ) { return !values.empty(); } );
    if ( !hasValues ) return std::nullopt;

    Statistics result;
    for ( auto& values : result )
    {
        values.assign( timeStepCount, HUGE_VAL );
    }

    const std::vector<double> percentiles = { 0.1, 0.5, 0.9 };

    std::vector<double> samples;
    samples.reserve( realizations.size() );

    for ( size_t t = 0; t < timeStepCount; t++ )
    {
        samples.clear();
        for ( size_t r = 0; r < realizations.size(); r++ )
        {
            const auto& values = realizationValues[r];
            const int   index  = realizations[r].valueIndices[t];
            if ( values.empty() || index < 0 ) continue;

            const double value = values[index];
            if ( std::isfinite( value ) ) samples.push_back( value );
        }
        if ( samples.empty() ) continue;

        double sum      = 0.0;
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        for ( double value : samples )
        {
            sum += value;
            minValue = std::min( minValue, value );
            maxValue = std::max( maxValue, value );
        }

        result[static_cast<size_t>( Statistic::MEAN )][t]    = sum / static_cast<double>( samples.size() );
        result[static_cast<size_t>( Statistic::MINIMUM )][t] = minValue;
        result[static_cast<size_t>( Statistic::MAXIMUM )][t] = maxValue;

        // The same percentile style as the ensemble statistics curves (RimEnsembleStatisticsCase)
        const auto percentileValues =
            RigStatisticsMath::calculatePercentiles( samples, percentiles, RigStatisticsMath::PercentileStyle::SWITCHED );
        if ( percentileValues && percentileValues->size() == percentiles.size() )
        {
            result[static_cast<size_t>( Statistic::P10 )][t] = ( *percentileValues )[0];
            result[static_cast<size_t>( Statistic::P50 )][t] = ( *percentileValues )[1];
            result[static_cast<size_t>( Statistic::P90 )][t] = ( *percentileValues )[2];
        }
    }

    return result;
}
