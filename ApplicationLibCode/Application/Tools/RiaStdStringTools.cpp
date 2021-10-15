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

#include "RiaStdStringTools.h"

#include "fast_float/include/fast_float/fast_float.h"

#include <cctype>
#include <charconv>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::trimString( const std::string& s )
{
    auto sCopy = s.substr( 0, s.find_last_not_of( ' ' ) + 1 );
    sCopy      = sCopy.substr( sCopy.find_first_not_of( ' ' ) );

    return sCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::isNumber( const std::string& s, char decimalPoint )
{
    if ( s.empty() ) return false;
    if ( findCharMatchCount( s, decimalPoint ) > 1 ) return false;
    if ( findCharMatchCount( s, '-' ) > 1 ) return false;
    if ( findCharMatchCount( s, 'e' ) > 1 ) return false;
    if ( findCharMatchCount( s, 'E' ) > 1 ) return false;

    std::string matchChars( "0123456789eE-+" );
    matchChars.append( 1, decimalPoint );

    auto it = s.find_first_not_of( matchChars );

    return ( it == std::string::npos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int16_t RiaStdStringTools::toInt16( const std::string& s )
{
    return (int16_t)toInt( s );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaStdStringTools::toInt( const std::string& s )
{
    int intValue = -1;

    try
    {
        intValue = std::stoi( s );
    }
    catch ( ... )
    {
    }

    return intValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaStdStringTools::toDouble( const std::string& s )
{
    double doubleValue = -1.0;

    char* end;
    doubleValue = std::strtod( s.data(), &end );

    return doubleValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::containsAlphabetic( const std::string& s )
{
    return std::find_if( s.begin(), s.end(), []( char c ) { return isalpha( c ); } ) != s.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::startsWithAlphabetic( const std::string& s )
{
    if ( s.empty() ) return false;

    return isalpha( s[0] ) != 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::toDouble( const std::string_view& s, double& value )
{
    auto resultObject = fast_float::from_chars( s.data(), s.data() + s.size(), value );

    return ( resultObject.ec == std::errc() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::toInt( const std::string_view& s, int& value )
{
    auto resultObject = std::from_chars( s.data(), s.data() + s.size(), value );

    return ( resultObject.ec == std::errc() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::toUpper( const std::string& s )
{
    auto strCopy( s );
    std::transform( strCopy.begin(), strCopy.end(), strCopy.begin(), []( unsigned char c ) { return std::toupper( c ); } );

    return strCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::endsWith( const std::string& mainStr, const std::string& toMatch )
{
    if ( mainStr.size() >= toMatch.size() &&
         mainStr.compare( mainStr.size() - toMatch.size(), toMatch.size(), toMatch ) == 0 )
        return true;
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RiaStdStringTools::splitString( const std::string& s, char delimiter )
{
    std::vector<std::string> words;

    splitByDelimiter( s, words, delimiter );

    return words;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::joinStrings( const std::vector<std::string>& s, char delimiter )
{
    std::string delimiterString( 1, delimiter );

    return join( s.begin(), s.end(), delimiterString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaStdStringTools::findCharMatchCount( const std::string& s, char c )
{
    size_t count = 0;
    size_t pos   = 0;
    while ( ( pos = s.find_first_of( c, pos + 1 ) ) != std::string::npos )
    {
        count++;
    }
    return count;
}

//--------------------------------------------------------------------------------------------------
/// Function to find Levenshtein Distance between two strings (x and y).
/// Adapted from pseudocode from wikipedia: https://en.wikipedia.org/wiki/Levenshtein_distance
/// Implementation is the Wagner-Fischer variant: https://en.wikipedia.org/wiki/Wagner-Fischer_algorithm
///
/// Return value is higher when strings are more "different", and zero when strings are equal.
//--------------------------------------------------------------------------------------------------
int RiaStdStringTools::computeEditDistance( const std::string& x, const std::string& y )
{
    // for all i and j, T[i,j] will hold the Levenshtein distance between
    // the first i characters of x and the first j characters of y
    int m = static_cast<int>( x.length() );
    int n = static_cast<int>( y.length() );

    std::vector<std::vector<int>> T( m + 1, std::vector<int>( n + 1, 0 ) );

    // source prefixes can be transformed into empty string by
    // dropping all characters
    for ( int i = 1; i <= m; i++ )
        T[i][0] = i;

    // target prefixes can be reached from empty source prefix
    // by inserting every character
    for ( int j = 1; j <= n; j++ )
        T[0][j] = j;

    // fill the lookup table in bottom-up manner
    for ( int i = 1; i <= m; i++ )
    {
        for ( int j = 1; j <= n; j++ )
        {
            int substitutionCost;
            if ( x[i - 1] == y[j - 1] )
                substitutionCost = 0;
            else
                substitutionCost = 1;

            int deletion    = T[i - 1][j] + 1;
            int insertion   = T[i][j - 1] + 1;
            int replacement = T[i - 1][j - 1] + substitutionCost;
            T[i][j]         = std::min( std::min( deletion, insertion ), replacement );
        }
    }

    // The distance between the two full strings as the last value computed.
    return T[m][n];
}
