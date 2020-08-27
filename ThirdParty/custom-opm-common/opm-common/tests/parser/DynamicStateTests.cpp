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


#define BOOST_TEST_MODULE DynamicStateTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>


Opm::TimeMap make_timemap(int num) {
    std::vector<std::time_t> tp;
    for (int i = 0; i < num; i++)
        tp.push_back( Opm::asTimeT(Opm::TimeStampUTC(2010,1,i+1)));

    Opm::TimeMap timeMap{ tp };
    return timeMap;
}



BOOST_AUTO_TEST_CASE(CreateDynamicTest) {
    const std::time_t startDate = Opm::TimeMap::mkdate(2010, 1, 1);
    Opm::TimeMap timeMap({ startDate });
    Opm::DynamicState<double> state(timeMap , 9.99);
}


BOOST_AUTO_TEST_CASE(DynamicStateGetOutOfRangeThrows) {
    const std::time_t startDate = Opm::TimeMap::mkdate(2010, 1, 1);
    Opm::TimeMap timeMap({ startDate });
    Opm::DynamicState<double> state(timeMap , 9.99);
    BOOST_CHECK_THROW( state.get(1) , std::out_of_range );
}


BOOST_AUTO_TEST_CASE(DynamicStateGetDefault) {
    const std::time_t startDate = Opm::TimeMap::mkdate(2010, 1, 1);
    Opm::TimeMap timeMap( { startDate } );
    Opm::DynamicState<int> state(timeMap , 137);
    BOOST_CHECK_EQUAL( 137 , state.get(0));
    BOOST_CHECK_EQUAL( 137 , state.back() );
}


BOOST_AUTO_TEST_CASE(DynamicStateSetOutOfRangeThrows) {
    Opm::TimeMap timeMap = make_timemap(3);
    Opm::DynamicState<int> state(timeMap , 137);

    BOOST_CHECK_THROW( state.update(3 , 100) , std::out_of_range );
}


BOOST_AUTO_TEST_CASE(DynamicStateSetOK) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 137);

    state.update(2 , 23 );
    BOOST_CHECK_EQUAL( 137 , state.get(0));
    BOOST_CHECK_EQUAL( 137 , state.get(1));
    BOOST_CHECK_EQUAL( 23 , state.get(2));
    BOOST_CHECK_EQUAL( 23 , state.get(5));

    state.update(2 , 17);
    BOOST_CHECK_EQUAL( 137 , state.get(0));
    BOOST_CHECK_EQUAL( 137 , state.get(1));
    BOOST_CHECK_EQUAL( 17 , state.get(2));
    BOOST_CHECK_EQUAL( 17 , state.get(5));

    state.update(6 , 60);
    BOOST_CHECK_EQUAL( 17 , state.get(2));
    BOOST_CHECK_EQUAL( 17 , state.get(5));
    BOOST_CHECK_EQUAL( 60 , state.get(6));
    BOOST_CHECK_EQUAL( 60 , state.get(8));
    BOOST_CHECK_EQUAL( 60 , state.get(9));
    BOOST_CHECK_EQUAL( 60 , state.back());
}

BOOST_AUTO_TEST_CASE(DynamicStateAddAt) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 0);

    state.update( 10 , 77 );
    {
        const int& v1 = state.at(10);
        int v2 = state.get(10);
        BOOST_CHECK_EQUAL( v1 , 77 );
        BOOST_CHECK_EQUAL( v1 , v2 );
        BOOST_CHECK( &v1 != &v2 );
    }
}

BOOST_AUTO_TEST_CASE(DynamicStateOperatorSubscript) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 137);

    state.update( 10 , 200 );
    BOOST_CHECK_EQUAL( state[9] , 137 );
    BOOST_CHECK_EQUAL( state[0] , 137 );

}


BOOST_AUTO_TEST_CASE(DynamicStateInitial) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 137);
    Opm::DynamicState<int> state2(timeMap , 137);

    state.update( 10 , 200 );
    BOOST_CHECK_EQUAL( state[9] , 137 );
    BOOST_CHECK_EQUAL( state[0] , 137 );
    BOOST_CHECK_EQUAL( state[10] , 200 );

    state.updateInitial( 63 );
    BOOST_CHECK_EQUAL( state[9] , 63 );
    BOOST_CHECK_EQUAL( state[0] , 63 );
    BOOST_CHECK_EQUAL( state[10] , 200 );

    state.updateInitial( 73 );
    BOOST_CHECK_EQUAL( state[9] , 73 );
    BOOST_CHECK_EQUAL( state[0] , 73 );
    BOOST_CHECK_EQUAL( state[10] , 200 );


    state2.update( 10 , 200 );
    BOOST_CHECK_EQUAL( state2[9] , 137 );
    BOOST_CHECK_EQUAL( state2[0] , 137 );
    BOOST_CHECK_EQUAL( state2[10] , 200 );
    state.updateInitial( 73 );
    BOOST_CHECK_EQUAL( state2[9] , 137 );
    BOOST_CHECK_EQUAL( state2[0] , 137 );
    BOOST_CHECK_EQUAL( state2[10] , 200 );
}

BOOST_AUTO_TEST_CASE( ResetGlobal ) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 137);

    state.update(5 , 100);
    BOOST_CHECK_EQUAL( state[0] , 137 );
    BOOST_CHECK_EQUAL( state[4] , 137 );
    BOOST_CHECK_EQUAL( state[5] , 100 );
    BOOST_CHECK_EQUAL( state[9] , 100 );

    state.updateInitial( 22 );
    BOOST_CHECK_EQUAL( state[0] , 22 );
    BOOST_CHECK_EQUAL( state[4] , 22 );
    BOOST_CHECK_EQUAL( state[5] , 100 );
    BOOST_CHECK_EQUAL( state[9] , 100 );

    state.globalReset( 88 );
    BOOST_CHECK_EQUAL( state[0] , 88 );
    BOOST_CHECK_EQUAL( state[4] , 88 );
    BOOST_CHECK_EQUAL( state[5] , 88 );
    BOOST_CHECK_EQUAL( state[9] , 88 );
}


BOOST_AUTO_TEST_CASE( CheckReturn ) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 137);

    BOOST_CHECK_EQUAL( false , state.update( 0 , 137 ));
    BOOST_CHECK_EQUAL( false , state.update( 3 , 137 ));
    BOOST_CHECK_EQUAL( true , state.update( 5 , 200 ));
}


BOOST_AUTO_TEST_CASE( UpdateEmptyInitial ) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 137);

    BOOST_CHECK_EQUAL( state[5] , 137 );
    state.updateInitial( 99 );
    BOOST_CHECK_EQUAL( state[5] , 99 );
}


BOOST_AUTO_TEST_CASE( find ) {
    Opm::TimeMap timeMap = make_timemap(6);
    Opm::DynamicState<int> state(timeMap , 137);

    BOOST_CHECK_EQUAL( state.find( 137 ) , 0 );
    BOOST_CHECK_EQUAL( state.find( 200 ) , -1 );
    BOOST_CHECK_EQUAL( state.find_not(137), -1);
    BOOST_CHECK_EQUAL( state.find_not(200), 0);
    state.update( 0 , 200 );
    BOOST_CHECK_EQUAL( state.find( 137 ) , -1 );
    BOOST_CHECK_EQUAL( state.find( 200 ) ,  0 );

    state.update( 2 , 300 );
    BOOST_CHECK_EQUAL( state.find( 200 ) ,  0 );
    BOOST_CHECK_EQUAL( state.find( 300 ) ,  2 );
    BOOST_CHECK_EQUAL( state.find_not( 200 ) ,  2 );

    state.update( 4 , 400 );
    BOOST_CHECK_EQUAL( state.find( 200 ) ,  0 );
    BOOST_CHECK_EQUAL( state.find( 300 ) ,  2 );
    BOOST_CHECK_EQUAL( state.find( 400 ) ,  4 );
    BOOST_CHECK_EQUAL( state.find( 500 ) ,  -1 );


    auto pred = [] (const int& elm) { return elm == 400 ;};
    BOOST_CHECK_EQUAL( state.find_if(pred), 4);
}


BOOST_AUTO_TEST_CASE( update_elm ) {
    Opm::TimeMap timeMap = make_timemap(6);
    Opm::DynamicState<int> state(timeMap , 137);
    state.update( 5, 88 );
    BOOST_CHECK_THROW( state.update_elm(10,88) , std::out_of_range );
    BOOST_CHECK_EQUAL( state[2],137 );
    BOOST_CHECK_EQUAL( state[3],137 );
    BOOST_CHECK_EQUAL( state[4],137 );

    state.update_elm(3,88);
    BOOST_CHECK_EQUAL( state[2],137 );
    BOOST_CHECK_EQUAL( state[3],88 );
    BOOST_CHECK_EQUAL( state[4],137 );

    for (auto& v : state)
        v += 2;

    BOOST_CHECK_EQUAL( state[2],139 );
    BOOST_CHECK_EQUAL( state[3],90  );
    BOOST_CHECK_EQUAL( state[4],139 );
}

BOOST_AUTO_TEST_CASE( update_equal ) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 0);
    state.update( 5, 100 );
    BOOST_REQUIRE_THROW(state.update_equal(100, 100), std::out_of_range);

    BOOST_CHECK_EQUAL(state[0], 0);
    BOOST_CHECK_EQUAL(state[4], 0);
    BOOST_CHECK_EQUAL(state[5], 100);

    state.update_equal(3,50);
    BOOST_CHECK_EQUAL(state[2], 0);
    BOOST_CHECK_EQUAL(state[3], 50);
    BOOST_CHECK_EQUAL(state[4], 50);
    BOOST_CHECK_EQUAL(state[5], 100);

    state.update_equal(4,50);
    BOOST_CHECK_EQUAL(state[4], 50);
    BOOST_CHECK_EQUAL(state[5], 100);


    state.update_equal(9,200);
    BOOST_CHECK_EQUAL(state[8] , 100);
    BOOST_CHECK_EQUAL(state[9] , 200);
    BOOST_CHECK_EQUAL(state[10], 200);
}





BOOST_AUTO_TEST_CASE( UNIQUE ) {
    Opm::TimeMap timeMap = make_timemap(11);
    Opm::DynamicState<int> state(timeMap , 13);
    auto unique0 = state.unique();
    BOOST_CHECK_EQUAL(unique0.size(), 1);
    BOOST_CHECK(unique0[0] == std::make_pair(std::size_t{0}, 13));

    state.update(3,300);
    state.update(6,600);
    auto unique1 = state.unique();
    BOOST_CHECK_EQUAL(unique1.size(), 3);
    BOOST_CHECK(unique1[0] == std::make_pair(std::size_t{0}, 13));
    BOOST_CHECK(unique1[1] == std::make_pair(std::size_t{3}, 300));
    BOOST_CHECK(unique1[2] == std::make_pair(std::size_t{6}, 600));
}

