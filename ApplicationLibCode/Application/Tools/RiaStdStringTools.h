/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include <algorithm>
#include <iterator>
#include <numeric>
#include <set>
#include <string>
#include <vector>

//==================================================================================================
//
//==================================================================================================
class RiaStdStringTools
{
public:
    static std::string trimString( const std::string& s );
    static std::string rightTrimString( const std::string& s );
    static std::string leftTrimString( const std::string& s );
    static std::string removeWhitespace( const std::string& line );

    static bool isNumber( const std::string& s, char decimalPoint );

    static int16_t toInt16( const std::string& s );
    static int     toInt( const std::string& s );
    static double  toDouble( const std::string& s );
    static bool    containsAlphabetic( const std::string& s );
    static bool    startsWithAlphabetic( const std::string& s );

    static std::string formatThousandGrouping( long value );

    // Conversion using fastest known approach
    static bool toDouble( const std::string_view& s, double& value );
    static bool toInt( const std::string_view& s, int& value );

    static std::string toUpper( const std::string& s );

    static bool endsWith( const std::string& mainStr, const std::string& toMatch );

    static std::vector<std::string> splitString( const std::string& s, char delimiter );
    static std::string              joinStrings( const std::vector<std::string>& s, char delimiter );

    static int computeEditDistance( const std::string& x, const std::string& y );

    static std::string removeHtmlTags( const std::string& s );

    // Convert the string "1,2,5-8,10" to {1, 2, 5, 6, 7, 8, 10}
    static std::set<int> valuesFromRangeSelection( const std::string& s );

    // Convert the range string with support for open ended expressions. minimum and maximum value will be used to limit the ranges.
    // The input "-3,5-8,10-", min:1, max:12 will produce {1, 2, 3, 5, 6, 7, 8, 10, 11, 12}
    static std::set<int> valuesFromRangeSelection( const std::string& s, int minimumValue, int maximumValue );

private:
    template <class Container>
    static void   splitByDelimiter( const std::string& str, Container& cont, char delimiter = ' ' );
    static size_t findCharMatchCount( const std::string& s, char c );
};

//==================================================================================================
//
//==================================================================================================
template <class Container>
void RiaStdStringTools::splitByDelimiter( const std::string& str, Container& cont, char delimiter )
{
    size_t start;
    size_t end = 0;

    while ( ( start = str.find_first_not_of( delimiter, end ) ) != std::string::npos )
    {
        end = str.find( delimiter, start );
        cont.push_back( str.substr( start, end - start ) );
    }
}

template <typename InputIt>
std::string join( InputIt begin, InputIt end, const std::string& separator = ", " )
{
    auto compose_key = [&separator]( std::string& key, const std::string& key_part ) -> std::string
    { return key.empty() ? key_part : key + separator + key_part; };

    return std::accumulate( begin, end, std::string(), compose_key );
}
