/*
  Copyright 2022 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <regex>

#include <opm/common/utility/shmatch.hpp>


bool Opm::shmatch(const std::string& pattern, const std::string& symbol) {
    // Shell patterns should implicitly be interpreted as anchored at beginning
    // and end.
    std::string re_pattern = "^" + pattern + "$";

    {
        // Shell wildcard '*' should be regular expression arbitrary character '.'
        // repeated arbitrary number of times '*'
        std::regex re("\\*");
        re_pattern = std::regex_replace(re_pattern, re, ".*");
    }

    {
        // Shell wildcard '?' should be regular expression one arbitrary character
        // '.'
        std::regex re("\\?");
        re_pattern = std::regex_replace(re_pattern, re, ".");
    }

    std::regex regexp(re_pattern);
    return std::regex_search(symbol, regexp);
}

