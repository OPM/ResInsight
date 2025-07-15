////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RifSummaryReaderAggregator.h"

#include "RifReaderEclipseSummary.h"

#include "cafAssert.h"

#include <memory>
#include <ranges>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderAggregator ::RifSummaryReaderAggregator( const std::vector<std::string>& filesOrderedByStartOfHistory )
    : m_fileNames( filesOrderedByStartOfHistory )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifSummaryReaderAggregator ::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_aggregatedTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RifSummaryReaderAggregator ::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    std::vector<double> values;
    for ( const auto& reader : m_summaryReaders )
    {
        auto [isOk, readerValues] = reader->values( resultAddress );

        if ( readerValues.empty() )
        {
            // When a well is introduced, no data is present before the time step the well is introduced
            // Add values of zero for this interval
            //
            // This issue was reported for resdata, but it is not relevant now as the low level file readers only handle
            // a single file.
            // https://github.com/OPM/ResInsight/issues/7065

            std::vector<double> zeros( reader->timeSteps( {} ).size(), 0.0 );
            readerValues = zeros;
        }

        auto valueCount = timeStepCount( reader.get() );
        readerValues.resize( valueCount );

        values.insert( values.end(), readerValues.begin(), readerValues.end() );
    }

    CAF_ASSERT( m_aggregatedTimeSteps.size() == values.size() );

    return { true, values };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifSummaryReaderAggregator ::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( !m_summaryReaders.empty() ) return m_summaryReaders.front()->unitName( resultAddress );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifSummaryReaderAggregator ::unitSystem() const
{
    if ( !m_summaryReaders.empty() ) return m_summaryReaders.front()->unitSystem();

    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifSummaryReaderAggregator ::timeStepCount( RifSummaryReaderInterface* reader ) const
{
    auto it = m_valueCountForReader.find( reader );
    if ( it != m_valueCountForReader.end() )
    {
        return it->second;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSummaryReaderAggregator ::createReadersAndImportMetaData( RiaThreadSafeLogger* threadSafeLogger )
{
    for ( const auto& fileName : m_fileNames )
    {
        auto candidate = std::make_unique<RifReaderEclipseSummary>();
        auto result    = candidate->open( QString::fromStdString( fileName ), threadSafeLogger );
        if ( result )
        {
            m_summaryReaders.push_back( std::move( candidate ) );
        }
    }

    calculateOverlappingTimeSteps();

    // Aggregate result addresses
    for ( const auto& reader : m_summaryReaders )
    {
        {
            auto resultAddresses = reader->allResultAddresses();
            m_allResultAddresses.insert( resultAddresses.begin(), resultAddresses.end() );
        }

        {
            auto errorResultAddresses = reader->allErrorAddresses();
            m_allErrorAddresses.insert( errorResultAddresses.begin(), errorResultAddresses.end() );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
// Detect any overlapping time steps. Always use data from summary reader with the newest data, so do processing
// from the last reader to the first
//
//--------------------------------------------------------------------------------------------------
void RifSummaryReaderAggregator ::calculateOverlappingTimeSteps()
{
    if ( m_summaryReaders.empty() ) return;

    time_t cutOffTime = 0;

    for ( const auto& it : std::ranges::reverse_view( m_summaryReaders ) )
    {
        if ( cutOffTime != 0 ) break; // Stop when we have found a valid cut-off time

        auto currentReader    = it.get();
        auto currentTimeSteps = currentReader->timeSteps( {} );

        m_valueCountForReader[currentReader] = currentTimeSteps.size();

        if ( !currentTimeSteps.empty() )
        {
            cutOffTime = currentTimeSteps.front();
        }
    }

    if ( cutOffTime == 0 )
    {
        return;
    }

    for ( int i = static_cast<int>( m_summaryReaders.size() - 2 ); i >= 0; i-- )
    {
        auto currentReader    = m_summaryReaders.at( static_cast<size_t>( i ) ).get();
        auto currentTimeSteps = currentReader->timeSteps( {} );

        size_t timeStepIndex = 0;
        for ( auto t : currentTimeSteps )
        {
            if ( t < cutOffTime ) timeStepIndex++;
        }

        m_valueCountForReader[currentReader] = timeStepIndex;

        if ( !currentTimeSteps.empty() && currentTimeSteps.front() < cutOffTime )
        {
            cutOffTime = currentTimeSteps.front();
        }
    }

    // Create a vector of increasing time steps with no overlapping time steps
    for ( const auto& reader : m_summaryReaders )
    {
        auto currentTimeSteps = reader->timeSteps( {} );
        auto valueCount       = m_valueCountForReader[reader.get()];

        if ( currentTimeSteps.empty() || valueCount == 0 ) continue;

        currentTimeSteps.resize( valueCount );

        m_aggregatedTimeSteps.insert( m_aggregatedTimeSteps.end(), currentTimeSteps.begin(), currentTimeSteps.end() );
    }
}
