/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RiaStdStringTools.h"
#include <fstream>
#include <string>

//#include "mio/include/mio/mmap.hpp"
#include "fast_float/include/fast_float/fast_float.h"
#include "mio/single_include/mio/mio.hpp"

std::pair<std::string, std::vector<float>> importKeyword( std::iostream& stream )
{
    std::string        keywordName;
    std::string        line;
    std::vector<float> values;
    bool               endOfKeyword = false;

    while ( !endOfKeyword && std::getline( stream, line ) )
    {
        if ( line.substr( 0, 2 ) == "--" ) continue;

        if ( keywordName.empty() )
        {
            keywordName = line;
            continue;
        }

        if ( line.find_first_of( '/' ) != std::string::npos )
        {
            endOfKeyword = true;
            continue;
        }

        if ( !endOfKeyword )
        {
            auto tokens = RiaStdStringTools::splitString( line, ' ' );

            for ( auto t : tokens )
            {
                const auto val = std::strtof( t.data(), nullptr );
                values.push_back( val );
            }
        }
    }

    return std::make_pair( keywordName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( GrdeclImporter, ReadFileFast )
{
    return;

    std::string filename =
        "e:/gitroot-ceesol/ResInsight-regression-test/ModelData/TestCase_Ascii_no_map_axis/geocell.grdecl";

    filename = "d:/scratch/R5_H25_C1_aug_grid.grdecl";

    std::stringstream buffer;
    {
        std::ifstream stream( filename );

        buffer << stream.rdbuf();
    }

    std::string         keywordName;
    std::string         line;
    std::vector<double> values;
    bool                endOfKeyword = false;

    while ( buffer.good() )
    {
        auto [keyword, values] = importKeyword( buffer );

        std::cout << keyword << " : " << values.size() << "\n";

        if ( !values.empty() )
        {
            std::cout << values.front() << " " << values.back();
            std::cout << "\n\n";
        }
    }
}

std::string_view readLine( const std::string_view& source, const size_t offset, size_t& bytesRead )
{
    if ( offset >= source.size() ) return {};

    const std::string_view tail = source.substr( offset, std::string_view::npos );
    auto                   end  = tail.find_first_of( '\n' );

    if ( end != std::string::npos )
    {
        bytesRead = end + 1;
        return tail.substr( 0, end );
    }

    return {};
}

const char* ws = " \t\n\r\f\v";

// trim from end of string (right)
inline std::string& rtrim( std::string& s, const char* t = ws )
{
    s.erase( s.find_last_not_of( t ) + 1 );
    return s;
}

// trim from beginning of string (left)
inline std::string& ltrim( std::string& s, const char* t = ws )
{
    s.erase( 0, s.find_first_not_of( t ) );
    return s;
}

// trim from both ends of string (right then left)
inline std::string& trim( std::string& s, const char* t = ws )
{
    return ltrim( rtrim( s, t ), t );
}

std::pair<std::string, std::vector<float>>
    importKeyword( const std::string_view& stream, const size_t offset, size_t& bytesRead )
{
    std::string        keywordName;
    std::string        line;
    std::vector<float> values;
    bool               endOfKeyword = false;

    size_t bytesReadLine = 0;
    bytesRead            = 0;

    while ( !endOfKeyword && ( offset + bytesRead < stream.size() ) )
    {
        line = readLine( stream, offset + bytesRead, bytesReadLine );
        bytesRead += bytesReadLine;

        if ( line.substr( 0, 2 ) == "--" ) continue;

        if ( keywordName.empty() )
        {
            std::string candidate = trim( line );
            if ( !candidate.empty() )
            {
                keywordName = candidate;
            }
            continue;
        }

        if ( line.find_first_of( '/' ) != std::string::npos )
        {
            endOfKeyword = true;
            continue;
        }

        if ( !endOfKeyword )
        {
            // const auto tokens = RiaStdStringTools::splitStringView( line, ' ' );

            size_t start;
            size_t end = 0;

            const auto delimiter = ' ';
            while ( ( start = line.find_first_not_of( delimiter, end ) ) != std::string::npos )
            {
                end = line.find( delimiter, start );
                // cont.emplace_back( str.substr( start, end - start ) );

                float value = 0.0f;
                {
                    auto answer = fast_float::from_chars( line.data() + start, line.data() + end, value );
                    if ( answer.ec == std::errc() )
                    {
                        values.emplace_back( value );
                    }
                }
            }
        }
    }

    return std::make_pair( keywordName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( GrdeclImporter, MioReadFileFast )
{
    std::string filename =
        "e:/gitroot-ceesol/ResInsight-regression-test/ModelData/TestCase_Ascii_no_map_axis/geocell.grdecl";

    filename = "d:/scratch/R5_H25_C1_aug_grid.grdecl";

    // mio::mmap_source mmap( filename );

    std::error_code  error;
    mio::mmap_sink   rw_mmap    = mio::make_mmap_sink( filename, 0, mio::map_entire_file, error );
    std::string_view stringData = rw_mmap.data();

    std::string         keywordName;
    std::string         line;
    std::vector<double> values;
    bool                endOfKeyword = false;

    size_t offset    = 0;
    size_t bytesRead = 0;

    while ( offset < stringData.size() )
    {
        auto [keyword, values] = importKeyword( stringData, offset, bytesRead );
        offset += bytesRead;

        std::cout << keyword << " : " << values.size() << "\n";

        if ( !values.empty() )
        {
            std::cout << values.front() << " " << values.back();
            std::cout << "\n\n";
        }
    }

    /*
        size_t offset = 0;

        while ( offset < stringData.length() )
        {
            size_t bytesRead = 0;
            auto   line      = readLine( stringData, offset, bytesRead );
            std::cout << line << "\n";

            offset += bytesRead;
        }
    */
}
