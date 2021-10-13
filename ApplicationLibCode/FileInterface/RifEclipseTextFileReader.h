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

#pragma once

#include "RifReaderInterface.h"

class RifEclipseKeywordContent
{
public:
    std::string        keyword;
    std::string        content;
    std::vector<float> values;
    size_t             offset;
};

//==================================================================================================
//
// File interface for Eclipse output files
//
//==================================================================================================
class RifEclipseTextFileReader
{
public:
    // Settings in Preferences will be used to forward to either file-direct or memory mapped file reader
    static std::vector<RifEclipseKeywordContent> readKeywordAndValues( const std::string& filename );

    // Read data directly from file
    static std::vector<RifEclipseKeywordContent> readKeywordAndValuesFile( const std::string& filename );

    // Read data using memory mapped file
    static std::vector<RifEclipseKeywordContent> readKeywordAndValuesMemoryMappedFile( const std::string& filename );

    // Data import function, public to be able to use from unit test
    static std::pair<std::string, std::vector<float>>
        readKeywordAndValues( const std::string_view& stringData, const size_t startOffset, size_t& bytesRead );

private:
    static constexpr const char* m_whiteSpace = " \t\n\r\f\v";

    // TODO: Make private or move to separate file, now public to be able to test code
public:
    static std::string_view readLine( const std::string_view& source, const size_t offset, size_t& bytesRead );

    // trim from end of string (right)
    static std::string& rtrim( std::string& s, const char* t = m_whiteSpace );

    // trim from beginning of string (left)
    static std::string& ltrim( std::string& s, const char* t = m_whiteSpace );

    // trim from both ends of string (right then left)
    static std::string& trim( std::string& s, const char* t = m_whiteSpace );

    // Parse string data for Eclipse keywords
    static std::vector<RifEclipseKeywordContent> parseStringData( const std::string_view& stringData );
};
