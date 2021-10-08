/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RifEclipseTextFileReader.h"

#include "fast_float/include/fast_float/fast_float.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::vector<float>>
    RifEclipseTextFileReader::readKeywordAndValues( const std::string_view& stringData, const size_t offset, size_t& bytesRead )
{
    std::vector<float> values;

    const auto commentChar = '-';

    std::string keywordName;
    std::string line;
    bool        isEndTokenKeywordRead = false;

    bytesRead   = 0;
    float value = 0.0f;

    while ( !isEndTokenKeywordRead && ( offset + bytesRead < stringData.size() ) )
    {
        size_t bytesReadLine = 0;
        line                 = readLine( stringData, offset + bytesRead, bytesReadLine );
        bytesRead += bytesReadLine;

        // Check for '--', used to define start of comments
        if ( line.size() > 2 && line[0] == commentChar && line[1] == commentChar ) continue;

        if ( keywordName.empty() )
        {
            std::string candidate = trim( line );
            if ( !candidate.empty() )
            {
                keywordName = candidate;
            }
            continue;
        }

        if ( !isEndTokenKeywordRead )
        {
            size_t start = 0;
            size_t end   = 0;

            while ( ( start = line.find_first_not_of( RifEclipseTextFileReader::m_whiteSpace, end ) ) != std::string::npos )
            {
                end = line.find_first_of( RifEclipseTextFileReader::m_whiteSpace, start );

                auto resultObject = fast_float::from_chars( line.data() + start, line.data() + end, value );
                if ( resultObject.ec == std::errc() )
                {
                    values.emplace_back( value );
                }
            }
        }

        // End of keyword is defined by '/'
        if ( line.find_first_of( '/' ) != std::string::npos )
        {
            isEndTokenKeywordRead = true;
            continue;
        }
    }

    return std::make_pair( keywordName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string_view RifEclipseTextFileReader::readLine( const std::string_view& source, const size_t offset, size_t& bytesRead )
{
    if ( offset >= source.size() ) return {};

    auto start = source.find_first_not_of( RifEclipseTextFileReader::m_whiteSpace, offset );
    auto end   = source.find_first_of( '\n', start );

    if ( end != std::string::npos )
    {
        // Add 1 to skip the \n we have found
        bytesRead = end - offset + 1;
        return source.substr( start, end - start );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string& RifEclipseTextFileReader::rtrim( std::string& s, const char* t /*= ws */ )
{
    s.erase( s.find_last_not_of( t ) + 1 );
    return s;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string& RifEclipseTextFileReader::ltrim( std::string& s, const char* t /*= ws */ )
{
    s.erase( 0, s.find_first_not_of( t ) );
    return s;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string& RifEclipseTextFileReader::trim( std::string& s, const char* t /*= ws */ )
{
    return ltrim( rtrim( s, t ), t );
}
