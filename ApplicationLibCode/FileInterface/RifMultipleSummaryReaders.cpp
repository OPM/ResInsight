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
void RifMultipleSummaryReaders::addReader( std::unique_ptr<RifSummaryReaderInterface> reader )
{
    if ( findReader( reader->serialNumber() ) != nullptr ) return;

    m_readers.push_back( std::move( reader ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RifMultipleSummaryReaders::findReader( int serialNumber ) const
{
    auto reader = std::find_if( m_readers.begin(),
                                m_readers.end(),
                                [serialNumber]( const std::unique_ptr<RifSummaryReaderInterface>& r )
                                { return r->serialNumber() == serialNumber; } );
    if ( reader != m_readers.end() )
    {
        return reader->get();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifMultipleSummaryReaders::removeReader( int serialNumber )
{
    auto reader = std::find_if( m_readers.begin(),
                                m_readers.end(),
                                [serialNumber]( const std::unique_ptr<RifSummaryReaderInterface>& r )
                                { return r->serialNumber() == serialNumber; } );

    if ( reader != m_readers.end() )
    {
        m_readers.erase( reader );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifMultipleSummaryReaders::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    for ( const auto& r : m_readers )
    {
        const auto& timeSteps = r->timeSteps( resultAddress );
        if ( !timeSteps.empty() ) return timeSteps;
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
        const auto& okAndValues = r->values( resultAddress );
        if ( okAndValues.first && !okAndValues.second.empty() ) return okAndValues;
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
        auto name = r->unitName( resultAddress );
        if ( !name.empty() ) return name;
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
void RifMultipleSummaryReaders::createAndSetAddresses()
{
    m_allErrorAddresses.clear();
    m_allResultAddresses.clear();

    for ( auto& reader : m_readers )
    {
        reader->createAndSetAddresses();

        auto resultAddresses = reader->allResultAddresses();
        m_allResultAddresses.insert( resultAddresses.begin(), resultAddresses.end() );

        auto errorResultAddresses = reader->allErrorAddresses();
        m_allErrorAddresses.insert( errorResultAddresses.begin(), errorResultAddresses.end() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifMultipleSummaryReaders::keywordCount() const
{
    for ( const auto& r : m_readers )
    {
        if ( r->keywordCount() > 0 ) return r->keywordCount();
    }

    return 0;
}
