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

#include <QDateTime>
#include <QString>
#include <string>

//==================================================================================================
//
//
//
//==================================================================================================
class RiaDateStringParser
{
public:
    static QDateTime parseDateString( const QString& dateString );
    static QDateTime parseDateString( const std::string& dateString );

private:
    static bool tryParseYearFirst( const std::string& s, int& year, int& month, int& day );
    static bool tryParseDayFirst( const std::string& s, int& year, int& month, int& day );
    static bool tryParseMonthFirst( const std::string& s, int& year, int& month, int& day );

    static bool tryParseYearFirstNoSeparators( const std::string& s, int& year, int& month, int& day );
    static bool tryParseDayFirstNoSeparators( const std::string& s, int& year, int& month, int& day );
    static bool tryParseMonthFirstNoSeparators( const std::string& s, int& year, int& month, int& day );

    static bool tryParseYear( const std::string& s, int& year );
    static bool tryParseMonth( const std::string& s, int& month );
    static bool tryParseDay( const std::string& s, int& day );

    static std::string separators();
    static bool        hasSeparators( const std::string& s );
    static std::string trimString( const std::string& s );
};
