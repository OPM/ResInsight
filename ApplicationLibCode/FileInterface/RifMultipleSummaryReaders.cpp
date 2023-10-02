/////////////////////////////////////////////////////////////////////////////////
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

#include "RifMultipleSummaryReaders.h"
#include "RimCalculatedSummaryCurveReader.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifMultipleSummaryReaders::RifMultipleSummaryReaders() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifMultipleSummaryReaders::addReader( RifSummaryReaderInterface* reader )
{
    for ( auto existingReader : m_readers )
    {
        if ( existingReader.p() == reader ) return;
    }

    m_readers.push_back( reader );

    rebuildMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifMultipleSummaryReaders::removeReader( RifSummaryReaderInterface* reader )
{
    m_readers.erase( reader );
    rebuildMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifMultipleSummaryReaders::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    for ( const auto& r : m_readers )
    {
        if ( r->hasAddress( resultAddress ) ) return r->timeSteps( resultAddress );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RifMultipleSummaryReaders::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    for ( const auto& r : m_readers )
    {
        if ( r->hasAddress( resultAddress ) ) return r->values( resultAddress );
    }

    return { false, {} };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifMultipleSummaryReaders::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    for ( const auto& r : m_readers )
    {
        if ( r->hasAddress( resultAddress ) ) return r->unitName( resultAddress );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifMultipleSummaryReaders::unitSystem() const
{
    CVF_ASSERT( !m_readers.empty() );

    return m_readers.at( 0 )->unitSystem();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifMultipleSummaryReaders::rebuildMetaData()
{
    m_allErrorAddresses.clear();
    m_allResultAddresses.clear();

    for ( auto& reader : m_readers )
    {
        // TODO: hack. Find a better way to rebuild calculated summary meta data.
        auto calcReader = dynamic_cast<RifCalculatedSummaryCurveReader*>( reader.p() );
        if ( calcReader ) calcReader->buildMetaData();

        {
            auto resultAddresses = reader->allResultAddresses();
            m_allResultAddresses.insert( resultAddresses.begin(), resultAddresses.end() );
        }

        {
            auto errorResultAddresses = reader->allErrorAddresses();
            m_allErrorAddresses.insert( errorResultAddresses.begin(), errorResultAddresses.end() );
        }
    }
}
