/*
  Copyright 2020 Equinor ASA.

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

#include <algorithm>
#include <numeric>
#include <utility>

#include <fmt/format.h>
#include <opm/common/utility/OpmInputError.hpp>

namespace Opm {

namespace {

template<typename ... Args>
std::string formatImpl(const std::string& msg_format, const KeywordLocation& loc, const Args& ...arguments) {
    return fmt::format(msg_format,
        arguments...,
        fmt::arg("keyword", loc.keyword),
        fmt::arg("file", loc.filename),
        fmt::arg("line", loc.lineno)
    );
}

}

std::string OpmInputError::formatException(const std::exception& e, const KeywordLocation& loc) {
    const std::string defaultMessage { R"(Problem with keyword {keyword}
In {file} line {line}.
Internal error: {})" } ;

    return formatImpl(defaultMessage, loc, e.what());
}

/*
  For the format() function it is possible to have an alternative function with
  a variaditic template which can be forwarded directly to the fmt::format()
  function, that is an elegant way to pass arbitrary additional arguments. That
  will require the OpmInputError::format() to become a templated function and
  the fmtlib dependendcy will be imposed on downstream modules.
*/
std::string OpmInputError::format(const std::string& msg_format, const KeywordLocation& loc) {
    return formatImpl(msg_format, loc);
}

std::string OpmInputError::formatSingle(const std::string& reason, const KeywordLocation& location) {
    const std::string defaultMessage { R"(Problem with keyword {keyword}
In {file} line {line}
{})" } ;

    return formatImpl(defaultMessage, location, reason);
}

namespace {

std::string locationStringLine(const KeywordLocation& loc) {
    return OpmInputError::format("\n  {keyword} in {file}, line {line}", loc);
}

}

std::string OpmInputError::formatMultiple(const std::string& reason, const std::vector<KeywordLocation>& locations) {
    std::vector<std::string> locationStrings;
    std::transform(locations.begin(), locations.end(), std::back_inserter(locationStrings), &locationStringLine);
    const std::string messages { std::accumulate(locationStrings.begin(), locationStrings.end(), std::string {}) } ;

    return fmt::format(R"(Problem with keywords {}
{})", messages, reason);
}

}
