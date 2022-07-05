/*
   Copyright 2020 Statoil ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include "config.h"

#define BOOST_TEST_MODULE OpmInputError_format

#include <fmt/format.h>

#include <boost/test/unit_test.hpp>

#include <exception>

#include <opm/common/utility/OpmInputError.hpp>

namespace {

const Opm::KeywordLocation location { "MXUNSUPP", "FILENAME.DAT", 42 } ;
const std::string error_string { "Error encountered" } ;

}

BOOST_AUTO_TEST_CASE(simple) {
    const std::string expected { "MXUNSUPP@FILENAME.DAT:42" } ;

    const std::string format_string { "{keyword}@{file}:{line}" } ;
    const std::string formatted { Opm::OpmInputError::format(format_string, location) } ;

    BOOST_CHECK_EQUAL(formatted, expected);
}

BOOST_AUTO_TEST_CASE(positional) {
    const std::string expected { "MXUNSUPP@FILENAME.DAT:42: Error encountered" } ;

    const std::string format_string { fmt::format("{{keyword}}@{{file}}:{{line}}: {}", error_string) } ;
    const std::string formatted { Opm::OpmInputError::format(format_string, location) } ;

    BOOST_CHECK_EQUAL(formatted, expected);
}

BOOST_AUTO_TEST_CASE(exception_init) {
    const std::string expected { R"(Problem with keyword MXUNSUPP
In FILENAME.DAT line 42.
Internal error: Runtime Error)" };

    const std::string formatted { Opm::OpmInputError(std::runtime_error("Runtime Error"), location).what() } ;

    BOOST_CHECK_EQUAL(formatted, expected);
}

BOOST_AUTO_TEST_CASE(exception_nest) {
    const std::string expected { R"(Problem with keyword MXUNSUPP
In FILENAME.DAT line 42.
Internal error: Runtime Error)" };

    try {
        try {
            throw std::runtime_error("Runtime Error");
        } catch (const std::exception& e) {
            std::throw_with_nested(Opm::OpmInputError(e, location));
        }
    } catch (const Opm::OpmInputError& opm_error) {
        BOOST_CHECK_EQUAL(opm_error.what(), expected);
    }
}

const Opm::KeywordLocation location2 { "MZUNSUPP", "FILENAME.DAT", 45 } ;

BOOST_AUTO_TEST_CASE(exception_multi_1) {
    const std::string expected { R"(Problem with keyword MXUNSUPP
In FILENAME.DAT line 42
Runtime Error)" } ;

    const std::string formatted { Opm::OpmInputError("Runtime Error", location).what() } ;

    BOOST_CHECK_EQUAL(formatted, expected);
}

BOOST_AUTO_TEST_CASE(exception_multi_2) {
    const std::string expected { R"(Problem with keywords 
  MXUNSUPP in FILENAME.DAT, line 42
  MZUNSUPP in FILENAME.DAT, line 45
Runtime Error)" } ;

    const std::string formatted { Opm::OpmInputError("Runtime Error", location, location2).what() } ;

    BOOST_CHECK_EQUAL(formatted, expected);
}
