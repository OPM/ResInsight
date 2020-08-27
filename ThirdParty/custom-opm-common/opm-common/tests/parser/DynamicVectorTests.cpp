/*
  Copyright 2013 Statoil ASA.

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

#define BOOST_TEST_MODULE DynamicVectorTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicVector.hpp>



BOOST_AUTO_TEST_CASE(CreateDynamicTest) {
    const std::time_t startDate = Opm::TimeMap::mkdate(2010, 1, 1);
    Opm::TimeMap timeMap({ startDate });
    Opm::DynamicVector<double> vector(timeMap , 9.99);

    BOOST_CHECK_EQUAL( vector[0] , 9.99 );
    BOOST_CHECK_THROW( vector[1] , std::out_of_range );
}



BOOST_AUTO_TEST_CASE(DynamicVectorSet) {
    const std::time_t startDate = Opm::TimeMap::mkdate(2010, 1, 1);
    std::vector<std::time_t> tp = { startDate };
    for (int i = 0; i < 4; i++)
        tp.push_back( Opm::asTimeT(Opm::TimeStampUTC(2010,1,i+2)));

    Opm::TimeMap timeMap{ tp };
    Opm::DynamicVector<int> state(timeMap , 137);
    BOOST_CHECK_EQUAL( 137 , state[0] );
    BOOST_CHECK_EQUAL( 137 , state[1] );
    BOOST_CHECK_EQUAL( 137 , state[2] );
    BOOST_CHECK_EQUAL( 137 , state[3] );
    BOOST_CHECK_EQUAL( 137 , state[4] );

    state[2] = 99;
    BOOST_CHECK_EQUAL( 137 , state[1] );
    BOOST_CHECK_EQUAL(  99 , state[2] );
    BOOST_CHECK_EQUAL( 137 , state[3] );

    state[0] = 88;
    BOOST_CHECK_EQUAL( 88 , state[0]);
    BOOST_CHECK_THROW( state[5] = 99 , std::out_of_range );
}



BOOST_AUTO_TEST_CASE(DynamicVectorPtr) {
    const std::time_t startDate = Opm::TimeMap::mkdate(2010, 1, 1);
    std::vector<std::time_t> tp = { startDate };
    for (int i = 0; i < 4; i++)
        tp.push_back( Opm::asTimeT(Opm::TimeStampUTC(2010,1,i+2)));

    Opm::TimeMap timeMap{ tp };
    Opm::DynamicVector<int> state( timeMap , 137 );

    BOOST_CHECK_EQUAL( 137 , state.iget(0) );
    BOOST_CHECK_EQUAL( 137 , state.iget(1) );
    BOOST_CHECK_EQUAL( 137 , state.iget(2) );
    BOOST_CHECK_EQUAL( 137 , state.iget(3) );
    BOOST_CHECK_EQUAL( 137 , state.iget(4) );

    state.iset(2 , 99);
    BOOST_CHECK_EQUAL( 137 , state.iget(1) );
    BOOST_CHECK_EQUAL(  99 , state.iget(2) );
    BOOST_CHECK_EQUAL( 137 , state.iget(3) );

    state.iset(0,88);
    BOOST_CHECK_EQUAL( 88 , state.iget(0));
    BOOST_CHECK_THROW( state.iset(5 , 99) , std::out_of_range );
}

