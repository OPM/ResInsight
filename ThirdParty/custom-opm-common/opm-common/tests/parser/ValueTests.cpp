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

#define BOOST_TEST_MODULE VALUETESTS
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Util/Value.hpp>


BOOST_AUTO_TEST_CASE( check_default_constructor ) {
    Opm::Value<int> v("Value");

    BOOST_CHECK_EQUAL( false , v.hasValue() );
    BOOST_CHECK_THROW( v.getValue() , std::logic_error );

    v.setValue( 70 );
    BOOST_CHECK_EQUAL( 70 , v.getValue());
}


BOOST_AUTO_TEST_CASE( check_value_constructor ) {
    Opm::Value<int> v("Value" , 100);

    BOOST_CHECK_EQUAL( true , v.hasValue() );
    BOOST_CHECK_EQUAL( 100 , v.getValue());
}



BOOST_AUTO_TEST_CASE( check_equal1 ) {
    Opm::Value<int> v1("v1" , 100);
    Opm::Value<int> v2("v2" , 100);

    BOOST_CHECK(v1.equal( v2 ));

    v1.setValue(110);
    BOOST_CHECK_EQUAL( false , v1.equal(v2));
}


BOOST_AUTO_TEST_CASE( check_equal2 ) {
    Opm::Value<int> v1("v1");
    Opm::Value<int> v2("v2");

    BOOST_CHECK_EQUAL(true , v1.equal( v2 ));

    v1.setValue(110);
    BOOST_CHECK_EQUAL( false , v1.equal(v2));
    v2.setValue(110);
    BOOST_CHECK_EQUAL( true , v1.equal(v2));
}


BOOST_AUTO_TEST_CASE( check_assign) {
    Opm::Value<int> v1("v1",100);
    Opm::Value<int> v2(v1);

    BOOST_CHECK(v1.equal(v2));
}
