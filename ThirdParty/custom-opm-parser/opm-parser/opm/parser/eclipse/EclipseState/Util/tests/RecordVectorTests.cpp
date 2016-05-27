/*
  Copyright 2014 Statoil ASA.

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

#include <stdexcept>
#include <iostream>
#include <boost/filesystem.hpp>

#define BOOST_TEST_MODULE ScheduleTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Util/RecordVector.hpp>

BOOST_AUTO_TEST_CASE( check_empty) {
    Opm::RecordVector<int> vector;
    BOOST_CHECK_EQUAL( 0U , vector.size());
    BOOST_CHECK_THROW( vector.get(0) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE( check_add ) {
    Opm::RecordVector<int> vector;
    vector.push_back(10);
    BOOST_CHECK_EQUAL( 1U , vector.size());
    BOOST_CHECK_EQUAL( 10 , vector.get(0));
    BOOST_CHECK_EQUAL( 10 , vector.get(10));

    vector.push_back(20);
    BOOST_CHECK_EQUAL( 2U , vector.size());
    BOOST_CHECK_EQUAL( 10 , vector.get(0));
    BOOST_CHECK_EQUAL( 20 , vector.get(1));
    BOOST_CHECK_EQUAL( 20 , vector.get(10));
}
