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
#include "RiaLogging.h"

#include "fast_float/include/fast_float/fast_float.h"

#include <QString>

#include <charconv>
#include <regex>
#include <sstream>

const std::string WHITESPACE = " \n\r\t\f\v";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::leftTrimString( const std::string& s )
{
    size_t start = s.find_first_not_of( WHITESPACE );
    return ( start == std::string::npos ) ? "" : s.substr( start );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::rightTrimString( const std::string& s )
{
    size_t end = s.find_last_not_of( WHITESPACE );
    return ( end == std::string::npos ) ? "" : s.substr( 0, end + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::trimString( const std::string& s )
{
    return rightTrimString( leftTrimString( s ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::removeWhitespace( const std::string& line )
{
    std::string s = line;
    s.erase( std::remove_if( s.begin(), s.end(), isspace ), s.end() );
    return s;
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
std::string RiaStdStringTools::formatThousandGrouping( long value )
{
    class my_punct : public std::numpunct<char>
    {
    protected:
        char        do_decimal_point() const override { return '.'; }
        char        do_thousands_sep() const override { return ' '; }
        std::string do_grouping() const override { return std::string( "\3" ); }
    };

    std::ostringstream os;
    os.imbue( std::locale( os.getloc(), new my_punct ) );
    fixed( os );
    os << value;
    return os.str();
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
    return mainStr.size() >= toMatch.size() && mainStr.compare( mainStr.size() - toMatch.size(), toMatch.size(), toMatch ) == 0;
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
    std::string joinedString;

    for ( const auto& str : s )
    {
        if ( !joinedString.empty() )
        {
            joinedString += delimiter;
        }

        joinedString += str;
    }

    return joinedString;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::removeHtmlTags( const std::string& s )
{
    std::regex  html_tags( "<.*?>" ); // Matches any HTML tag
    std::string result = std::regex_replace( s, html_tags, "" );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaStdStringTools::valuesFromRangeSelection( const std::string& s )
{
    try
    {
        std::set<int>      result;
        std::istringstream stringStream( s );
        std::string        token;

        while ( std::getline( stringStream, token, ',' ) )
        {
            token = RiaStdStringTools::trimString( token );

            std::istringstream tokenStream( token );
            int                startIndex, endIndex;
            char               dash;

            if ( tokenStream >> startIndex )
            {
                if ( tokenStream >> dash && dash == '-' && tokenStream >> endIndex )
                {
                    if ( startIndex > endIndex )
                    {
                        // If start is greater than end, swap them
                        std::swap( startIndex, endIndex );
                    }

                    for ( int i = startIndex; i <= endIndex; ++i )
                    {
                        result.insert( i );
                    }
                }
                else
                {
                    result.insert( startIndex );
                }
            }
        }

        return result;
    }
    catch ( const std::exception& e )
    {
        QString str = QString( "Failed to convert text string \" %1 \" to list of integers : " ).arg( QString::fromStdString( s ) ) +
                      QString::fromStdString( e.what() );
        RiaLogging::error( str );
    }
    catch ( ... )
    {
        QString str =
            QString( "Failed to convert text string \" %1 \" to list of integers : Caught unknown exception" ).arg( QString::fromStdString( s ) );
        RiaLogging::error( str );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaStdStringTools::valuesFromRangeSelection( const std::string& s, int minVal, int maxVal )
{
    try
    {
        std::set<int>     result;
        std::stringstream stringStream( s );
        std::string       token;

        while ( std::getline( stringStream, token, ',' ) )
        {
            token = RiaStdStringTools::trimString( token );

            // Check for range
            size_t dashPos = token.find( '-' );
            if ( dashPos != std::string::npos )
            {
                int startIndex = ( dashPos == 0 ) ? minVal : std::stoi( token.substr( 0, dashPos ) );
                int endIndex   = ( dashPos == token.size() - 1 ) ? maxVal : std::stoi( token.substr( dashPos + 1 ) );
                if ( startIndex > endIndex )
                {
                    // If start is greater than end, swap them
                    std::swap( startIndex, endIndex );
                }
                for ( int i = startIndex; i <= endIndex; ++i )
                {
                    result.insert( i );
                }
            }
            else
            {
                // Check for individual numbers
                result.insert( std::stoi( token ) );
            }
        }

        return result;
    }
    catch ( const std::exception& e )
    {
        QString str = QString( "Failed to convert text string \" %1 \" to list of integers : " ).arg( QString::fromStdString( s ) ) +
                      QString::fromStdString( e.what() );
        RiaLogging::error( str );
    }
    catch ( ... )
    {
        QString str =
            QString( "Failed to convert text string \" %1 \" to list of integers : Caught unknown exception" ).arg( QString::fromStdString( s ) );
        RiaLogging::error( str );
    }

    return {};
}
