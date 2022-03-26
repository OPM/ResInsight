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

#include "RifProjectSummaryDataWriter.h"

#include "RifSummaryReaderInterface.h"

#include "opm/common/utility/TimeService.hpp"
#include "opm/io/eclipse/EclOutput.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

#include "cafAssert.h"

#include <numeric>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifProjectSummaryDataWriter::RifProjectSummaryDataWriter()
    : m_timeStepCount( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifProjectSummaryDataWriter::importFromSourceSummaryReader( const RifSummaryReaderInterface* reader )
{
    if ( !reader ) return;

    std::string keyword        = "TIME";
    auto        summaryAddress = RifEclipseSummaryAddress::miscAddress( keyword );
    if ( reader->hasAddress( summaryAddress ) )
    {
        auto timeSteps = reader->timeSteps( summaryAddress );
        if ( !timeSteps.empty() )
        {
            Opm::TimeStampUTC ts( timeSteps.front() );
            m_startTime = { ts.day(), ts.month(), ts.year(), ts.hour(), ts.minutes(), ts.seconds(), 0 };
        }

        std::vector<double> values;
        reader->values( summaryAddress, &values );

        const auto& unitString = reader->unitName( summaryAddress );

        m_keywords.push_back( keyword );
        m_units.push_back( unitString );

        std::vector<float> floatValues;
        floatValues.reserve( values.size() );
        for ( const auto& v : values )
        {
            floatValues.push_back( v );
        }

        m_values.push_back( floatValues );

        m_timeStepCount = values.size();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifProjectSummaryDataWriter::importFromProjectSummaryFile( const std::string& projectSummaryFileName )
{
    try
    {
        Opm::EclIO::ExtESmry sourceSummary( projectSummaryFileName );

        Opm::TimeStampUTC ts( std::chrono::system_clock::to_time_t( sourceSummary.startdate() ) );
        m_startTime = { ts.day(), ts.month(), ts.year(), ts.hour(), ts.minutes(), ts.seconds(), 0 };

        auto keywords = sourceSummary.keywordList();
        for ( const auto& keyword : keywords )
        {
            const auto& values     = sourceSummary.get( keyword );
            const auto& unitString = sourceSummary.get_unit( keyword );

            m_keywords.push_back( keyword );
            m_units.push_back( unitString );
            m_values.push_back( values );

            if ( m_timeStepCount == 0 )
            {
                m_timeStepCount = values.size();
            }
        }
    }
    catch ( ... )
    {
        std::string txt = "Error detected during import of data from " + projectSummaryFileName;
        m_errorMessages.push_back( txt );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifProjectSummaryDataWriter::setData( const std::vector<std::string>&        keywords,
                                           const std::vector<std::string>&        units,
                                           const std::vector<std::vector<float>>& values )
{
    if ( keywords.empty() ) return;

    CAF_ASSERT( keywords.size() == units.size() );
    CAF_ASSERT( keywords.size() == values.size() );

    for ( size_t i = 0; i < keywords.size(); i++ )
    {
        auto existingIndex = indexForKeyword( keywords[i] );
        if ( existingIndex == -1 )
        {
            m_keywords.push_back( keywords[i] );
            m_units.push_back( units[i] );
            m_values.push_back( values[i] );
        }
        else
        {
            // Overwrite existing data

            m_keywords[existingIndex] = keywords[i];
            m_units[existingIndex]    = units[i];
            m_values[existingIndex]   = values[i];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifProjectSummaryDataWriter::writeDataToFile( const std::string& fileName )
{
    // Reference to other locations writing to ESMRY files
    // ESmry::make_esmry_file()
    // ExtSmryOutput::write()

    try
    {
        // The ExtESmry reader supports only binary mode, set formatted to false
        bool                  formatted = false;
        Opm::EclIO::EclOutput outFile( fileName, formatted, std::ios::out );

        outFile.write<int>( "START", m_startTime );
        outFile.write( "KEYCHECK", m_keywords );
        outFile.write( "UNITS", m_units );

        {
            // Bool array 1 means RSTEP, 0 means no RSTEP
            // Dummy values, but required by the reader
            std::vector<int> intValues( m_timeStepCount, 1 );
            outFile.write<int>( "RSTEP", intValues );
        }

        {
            // TSTEP represents time steps
            // Dummy values, but required by the reader
            std::vector<int> intValues;
            intValues.resize( m_timeStepCount );
            std::iota( intValues.begin(), intValues.end(), 0 );
            outFile.write<int>( "TSTEP", intValues );
        }

        for ( size_t i = 0; i < static_cast<size_t>( m_keywords.size() ); i++ )
        {
            std::string vect_name = "V" + std::to_string( i );
            outFile.write<float>( vect_name, m_values[i] );
        }
    }
    catch ( ... )
    {
        std::string txt = "Error detected during export of data to " + fileName;
        m_errorMessages.push_back( txt );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifProjectSummaryDataWriter::errorMessages() const
{
    return m_errorMessages;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifProjectSummaryDataWriter::clearErrorMessages()
{
    m_errorMessages.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifProjectSummaryDataWriter::indexForKeyword( const std::string& keyword ) const
{
    for ( int i = 0; i < static_cast<int>( m_keywords.size() ); i++ )
    {
        if ( m_keywords[i] == keyword ) return i;
    }

    return -1;
}
