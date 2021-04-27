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

#include "RifSummaryReaderMultipleFiles.h"

#include "RifReaderEclipseSummary.h"

#include "cafAssert.h"

#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderMultipleFiles::RifSummaryReaderMultipleFiles( const std::vector<std::string>& filesOrderedByStartOfHistory )
    : m_fileNames( filesOrderedByStartOfHistory )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifSummaryReaderMultipleFiles::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_aggregatedTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSummaryReaderMultipleFiles::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    for ( const auto& reader : m_summaryReaders )
    {
        std::vector<double> readerValues;
        reader->values( resultAddress, &readerValues );
        values->insert( values->end(), readerValues.begin(), readerValues.end() );
    }

    CAF_ASSERT( m_aggregatedTimeSteps.size() == values->size() );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifSummaryReaderMultipleFiles::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( !m_summaryReaders.empty() ) return m_summaryReaders.front()->unitName( resultAddress );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifSummaryReaderMultipleFiles::unitSystem() const
{
    if ( !m_summaryReaders.empty() ) return m_summaryReaders.front()->unitSystem();

    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSummaryReaderMultipleFiles::createReadersAndImportMetaData( RiaThreadSafeLogger* threadSafeLogger )
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

    for ( const auto& reader : m_summaryReaders )
    {
        auto readerTimeSteps = reader->timeSteps( {} );

        m_aggregatedTimeSteps.insert( m_aggregatedTimeSteps.end(), readerTimeSteps.begin(), readerTimeSteps.end() );

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
