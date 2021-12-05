/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifMultipleSummaryReaders::RifMultipleSummaryReaders()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifMultipleSummaryReaders::addReader( RifSummaryReaderInterface* reader )
{
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
const std::vector<time_t>& RifMultipleSummaryReaders::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    for ( const auto r : m_readers )
    {
        if ( r->hasAddress( resultAddress ) ) return r->timeSteps( resultAddress );
    }

    static std::vector<time_t> empty;

    return empty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifMultipleSummaryReaders::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    for ( const auto r : m_readers )
    {
        if ( r->hasAddress( resultAddress ) ) return r->values( resultAddress, values );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifMultipleSummaryReaders::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    for ( const auto r : m_readers )
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

    for ( const auto& reader : m_readers )
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
}
