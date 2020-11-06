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

#include "RiaDateStringParser.h"

#include "RiaQDateTimeTools.h"
#include "RiaStdStringTools.h"

#include <algorithm>

const std::string MONTH_NAMES[] =
    { "january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december" };

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RiaDateStringParser::parseDateString( const QString& dateString )
{
    return RiaDateStringParser::parseDateString( dateString.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RiaDateStringParser::parseDateString( const std::string& dateString )
{
    int  year = -1, month = -1, day = -1;
    bool parsedOk = false;
    if ( hasSeparators( dateString ) )
    {
        parsedOk = tryParseYearFirst( dateString, year, month, day ) ||
                   tryParseDayFirst( dateString, year, month, day ) || tryParseMonthFirst( dateString, year, month, day );
    }
    if ( !parsedOk )
    {
        auto firstNumerical = dateString.find_first_of( "0123456789" );
        if ( firstNumerical != std::string::npos )
        {
            std::string subString = dateString.substr( firstNumerical );

            parsedOk = tryParseYearFirstNoSeparators( subString, year, month, day ) ||
                       tryParseDayFirstNoSeparators( subString, year, month, day ) ||
                       tryParseMonthFirstNoSeparators( subString, year, month, day );
        }
    }

    QDateTime dt;
    dt.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );
    if ( parsedOk ) dt.setDate( QDate( year, month, day ) );

    return dt;
}

//--------------------------------------------------------------------------------------------------
/// Try parse formats
/// 'yyyy mm dd'
/// 'yyyy MMM dd'
/// 'yyyy_mm_dd'
/// 'yyyy_MMM_dd'
/// 'yyyy-mm-dd'
/// 'yyyy-MMM-dd'
/// 'yyyy.MMM.dd'
/// MMM is month name (shortened)
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseYearFirst( const std::string& s, int& year, int& month, int& day )
{
    auto firstSep = s.find_first_of( separators() );
    auto lastSep  = s.find_first_of( separators(), firstSep + 1 );

    if ( firstSep == std::string::npos || lastSep == std::string::npos ) return false;

    auto sYear  = s.substr( 0, firstSep );
    auto sMonth = s.substr( firstSep + 1, lastSep - firstSep - 1 );
    auto sDay   = s.substr( lastSep + 1 );

    return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
}

//--------------------------------------------------------------------------------------------------
/// Try parse formats
/// 'dd mm yyyy'
/// 'dd MMM yyyy'
/// 'dd_mm_yyyy'
/// 'dd_MMM_yyyy'
/// 'dd-mm-yyyy'
/// 'dd-MMM-yyyy'
/// 'dd.mm.yyyy'
/// 'dd.MMM.yyyy'
/// MMM is month name (shortened)
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseDayFirst( const std::string& s, int& year, int& month, int& day )
{
    auto firstSep = s.find_first_of( separators() );
    auto lastSep  = s.find_first_of( separators(), firstSep + 1 );

    if ( firstSep == std::string::npos || lastSep == std::string::npos ) return false;

    auto sDay   = s.substr( 0, firstSep );
    auto sMonth = s.substr( firstSep + 1, lastSep - firstSep - 1 );
    auto sYear  = s.substr( lastSep + 1 );

    return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseMonthFirst( const std::string& s, int& year, int& month, int& day )
{
    auto firstSep = s.find_first_of( separators() );
    auto lastSep  = s.find_first_of( separators(), firstSep + 1 );

    if ( firstSep == std::string::npos || lastSep == std::string::npos ) return false;

    auto sMonth = s.substr( 0, firstSep );
    auto sDay   = s.substr( firstSep + 1, lastSep - firstSep - 1 );
    auto sYear  = s.substr( lastSep + 1 );

    return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseYearFirstNoSeparators( const std::string& s, int& year, int& month, int& day )
{
    if ( s.length() == 8 )
    {
        // Four digit year
        auto sYear  = s.substr( 0, 4 );
        auto sMonth = s.substr( 4, 2 );
        auto sDay   = s.substr( 6, 2 );
        return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
    }
    else if ( s.length() == 6 )
    {
        // Two digit year
        auto sYear  = s.substr( 0, 2 );
        auto sMonth = s.substr( 2, 2 );
        auto sDay   = s.substr( 4, 2 );

        return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseDayFirstNoSeparators( const std::string& s, int& year, int& month, int& day )
{
    if ( s.length() == 8 )
    {
        // Four digit year
        auto sDay   = s.substr( 0, 2 );
        auto sMonth = s.substr( 2, 2 );
        auto sYear  = s.substr( 4, 4 );

        return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
    }
    else if ( s.length() == 6 )
    {
        // Two digit year
        auto sDay   = s.substr( 0, 2 );
        auto sMonth = s.substr( 2, 2 );
        auto sYear  = s.substr( 4, 2 );

        return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseMonthFirstNoSeparators( const std::string& s, int& year, int& month, int& day )
{
    if ( s.length() == 8 )
    {
        // Four digit year
        auto sMonth = s.substr( 0, 2 );
        auto sDay   = s.substr( 2, 2 );
        auto sYear  = s.substr( 4, 4 );

        return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
    }
    else if ( s.length() == 6 )
    {
        // Two digit year
        auto sMonth = s.substr( 0, 2 );
        auto sDay   = s.substr( 2, 2 );
        auto sYear  = s.substr( 4, 2 );

        return tryParseYear( sYear, year ) && tryParseMonth( sMonth, month ) && tryParseDay( sDay, day );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseYear( const std::string& s, int& year )
{
    if ( RiaStdStringTools::containsAlphabetic( s ) ) return false;

    auto today = QDate::currentDate();
    int  y     = RiaStdStringTools::toInt( s );
    if ( y < 100 )
    {
        if ( y > 70 )
            y += 1900;
        else
            y += 2000;
    }

    if ( y > 1970 && y <= today.year() + 100 ) // Support dates 100 years into the future.
    {
        year = y;

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseMonth( const std::string& s, int& month )
{
    if ( RiaStdStringTools::containsAlphabetic( s ) )
    {
        auto sMonth = s;
        sMonth      = trimString( sMonth );
        std::transform( sMonth.begin(), sMonth.end(), sMonth.begin(), []( const char c ) -> char {
            return (char)::tolower( c );
        } );

        for ( int i = 0; i < 12; i++ )
        {
            if ( MONTH_NAMES[i].compare( 0, sMonth.size(), sMonth ) == 0 )
            {
                month = i + 1;
                return true;
            }
        }
    }
    else
    {
        int m = RiaStdStringTools::toInt( s );
        if ( m >= 1 && m <= 12 )
        {
            month = m;

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::tryParseDay( const std::string& s, int& day )
{
    if ( RiaStdStringTools::containsAlphabetic( s ) ) return false;

    int d = RiaStdStringTools::toInt( s );
    if ( d >= 1 && d <= 31 )
    {
        day = d;

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDateStringParser::separators()
{
    return " -_.";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDateStringParser::hasSeparators( const std::string& s )
{
    auto firstSep = s.find_first_of( separators() );
    return firstSep != std::string::npos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDateStringParser::trimString( const std::string& s )
{
    auto sCopy = s.substr( 0, s.find_last_not_of( ' ' ) + 1 );
    sCopy      = sCopy.substr( sCopy.find_first_not_of( ' ' ) );
    return sCopy;
}
