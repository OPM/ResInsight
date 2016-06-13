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
#include <iostream>
#include <boost/filesystem.hpp>


#define BOOST_TEST_MODULE DynamicVectorTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicVector.hpp>



BOOST_AUTO_TEST_CASE(CreateDynamicTest) {
    boost::gregorian::date startDate( 2010 , boost::gregorian::Jan , 1);
    Opm::TimeMapPtr timeMap(new Opm::TimeMap(boost::posix_time::ptime(startDate)));
    Opm::DynamicVector<double> vector(timeMap , 9.99);

    BOOST_CHECK_EQUAL( vector[0] , 9.99 );
    BOOST_CHECK_THROW( vector[1] , std::range_error);
}



BOOST_AUTO_TEST_CASE(DynamicVectorSet) {
    boost::gregorian::date startDate( 2010 , boost::gregorian::Jan , 1);
    Opm::TimeMapPtr timeMap(new Opm::TimeMap(boost::posix_time::ptime(startDate)));
    Opm::DynamicVector<int> state(timeMap , 137);
    for (size_t i = 0; i < 4; i++)
        timeMap->addTStep( boost::posix_time::hours( (i+1) * 24 ));

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
    BOOST_CHECK_THROW( state[5] = 99 , std::range_error);
}



BOOST_AUTO_TEST_CASE(DynamicVectorPtr) {
    boost::gregorian::date startDate( 2010 , boost::gregorian::Jan , 1);
    Opm::TimeMapPtr timeMap(new Opm::TimeMap(boost::posix_time::ptime(startDate)));
    Opm::DynamicVector<int> * state = new Opm::DynamicVector<int>( timeMap , 137 );
    for (size_t i = 0; i < 4; i++)
        timeMap->addTStep( boost::posix_time::hours( (i+1) * 24 ));

    BOOST_CHECK_EQUAL( 137 , state->iget(0) );
    BOOST_CHECK_EQUAL( 137 , state->iget(1) );
    BOOST_CHECK_EQUAL( 137 , state->iget(2) );
    BOOST_CHECK_EQUAL( 137 , state->iget(3) );
    BOOST_CHECK_EQUAL( 137 , state->iget(4) );

    state->iset(2 , 99);
    BOOST_CHECK_EQUAL( 137 , state->iget(1) );
    BOOST_CHECK_EQUAL(  99 , state->iget(2) );
    BOOST_CHECK_EQUAL( 137 , state->iget(3) );

    state->iset(0,88);
    BOOST_CHECK_EQUAL( 88 , state->iget(0));
    BOOST_CHECK_THROW( state->iset(5 , 99) , std::range_error);

    delete state;
}

