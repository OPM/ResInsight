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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RiaStdStringTools::trimString(const std::string& s)
{
    auto sCopy = s.substr(0, s.find_last_not_of(' ') + 1);
    sCopy = sCopy.substr(sCopy.find_first_not_of(' '));

    return sCopy;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::isNumber(const std::string& s, char decimalPoint)
{
    if (s.size() == 0) return false;
    if (findCharMatchCount(s, decimalPoint) > 1) return false;
    if (findCharMatchCount(s, '-') > 1) return false;
    if (findCharMatchCount(s, 'e') > 1) return false;
    if (findCharMatchCount(s, 'E') > 1) return false;

    std::string matchChars("0123456789eE-");
    matchChars.append(1, decimalPoint);
    return (s.find_first_not_of(matchChars) == std::string::npos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiaStdStringTools::toInt(const std::string& s)
{
    int intValue = -1;

    try
    {
        intValue = std::stoi(s);
    }
    catch (...)
    {
    }

    return intValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RiaStdStringTools::toDouble(const std::string& s)
{
    double doubleValue = -1.0;

    char* end;
    doubleValue = std::strtod(s.data(), &end);

    return doubleValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::containsAlphabetic(const std::string& s)
{
    return std::find_if(s.begin(), s.end(), [](char c) { return isalpha(c); }) != s.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::containsOnlyLettersAndDigits(const std::string& s)
{
    return std::find_if(s.begin(), s.end(), [](char c) { return !isalpha(c) && !isdigit(c); }) == s.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaStdStringTools::startsWithAlphabetic(const std::string& s)
{
    if (s.empty()) return false;

    return isalpha(s[0]) != 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RiaStdStringTools::splitStringBySpace(const std::string& s)
{
    std::vector<std::string> words;

    splitByDelimiter(s, words);

    return words;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RiaStdStringTools::findCharMatchCount(const std::string& s, char c)
{
    size_t count = 0;
    size_t pos = 0;
    while ((pos = s.find_first_of(c, pos + 1)) != std::string::npos)
    {
        count++;
    }
    return count;
}
