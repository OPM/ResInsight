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
#include <sstream>
#include <string>
#include <vector>

//==================================================================================================
//
//==================================================================================================
class RiaStdStringTools
{
public:
    static std::string  trimString(const std::string& s);
    static bool         isNumber(const std::string& s, char decimalPoint);

    static int          toInt(const std::string& s);
    static double       toDouble(const std::string& s);
    static bool         containsAlphabetic(const std::string& s);
    static bool         containsOnlyLettersAndDigits(const std::string& s);
    static bool         startsWithAlphabetic(const std::string& s);

    static std::vector<std::string> splitStringBySpace(const std::string& s);

private:
    template <class Container>
    static void splitByDelimiter(const std::string& str, Container& cont, char delimiter = ' ');
    static size_t findCharMatchCount(const std::string& s, char c);
};

//==================================================================================================
//
//==================================================================================================
template <class Container>
void RiaStdStringTools::splitByDelimiter(const std::string& str, Container& cont, char delimiter)
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        if (token.find_first_not_of(delimiter) != std::string::npos)
        {
            cont.push_back(token);
        }
    }
}
