/*
  Copyright 2016 Statoil ASA.

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
#define BOOST_TEST_MODULE FunctionalTests

#include <vector>

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Utility/Functional.hpp>
#include <iostream>

using namespace Opm;

BOOST_AUTO_TEST_CASE(iotaEqualCollections) {
    std::vector< int > vec( 5 );

    for( int i = 0; i < 5; ++i )
        vec[ i ] = i;

    fun::iota iota( 5 );
    for( auto x : iota )
        std::cout << x << " ";
    std::cout << std::endl;
    std::vector< int > vec_iota( iota.begin(), iota.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            vec_iota.begin(), vec_iota.end(),
            vec.begin(), vec.end() );
    BOOST_CHECK_EQUAL_COLLECTIONS(
            vec_iota.begin(), vec_iota.end(),
            fun::iota( 5 ).begin(), fun::iota( 5 ).end() );
    BOOST_CHECK_EQUAL_COLLECTIONS(
            vec.begin(), vec.end(),
            fun::iota( 5 ).begin(), fun::iota( 5 ).end() );
}

BOOST_AUTO_TEST_CASE(iotaForeach) {
    /* this test is mostly a syntax verification test */

    std::vector< int > vec = { 0, 1, 2, 3, 4 };

    for( auto x : fun::iota( 5 ) )
        BOOST_CHECK_EQUAL( vec[ x ], x );
}

BOOST_AUTO_TEST_CASE(iotaSize) {
    BOOST_CHECK_EQUAL( 5, fun::iota( 5 ).size() );
    BOOST_CHECK_EQUAL( 5, fun::iota( 1, 6 ).size() );
    BOOST_CHECK_EQUAL( 0, fun::iota( 0 ).size() );
    BOOST_CHECK_EQUAL( 0, fun::iota( 0, 0 ).size() );
}

BOOST_AUTO_TEST_CASE(iotaWithMap) {
    const auto plus1 = []( int x ) { return x + 1; };

    std::vector< int > vec = { 1, 2, 3, 4, 5 };
    auto vec_iota = fun::map( plus1, fun::iota( 5 ) );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            vec_iota.begin(), vec_iota.end(),
            vec.begin(), vec.end() );
}

BOOST_AUTO_TEST_CASE(iotaNegativeBegin) {
    const auto vec = { -4, -3, -2, -1, 0 };

    fun::iota iota( -4, 1 );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            vec.begin(), vec.end(),
            iota.begin(), iota.end() );
}
