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

#define BOOST_TEST_MODULE ParserTests
#include <stdexcept>
#include <boost/test/unit_test.hpp>

#include "src/opm/input/eclipse/Parser/raw/StarToken.hpp"


BOOST_AUTO_TEST_CASE(NoStarThrows) {
    BOOST_REQUIRE_THROW(Opm::StarToken st("Hei...") , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(InvalidCountThrow) {
    BOOST_REQUIRE_THROW( Opm::StarToken st("X*") , std::invalid_argument);
    BOOST_REQUIRE_THROW( Opm::StarToken st("1.25*") , std::invalid_argument);
    BOOST_REQUIRE_THROW( Opm::StarToken st("-3*") , std::invalid_argument);
    BOOST_REQUIRE_THROW( Opm::StarToken st("0*") , std::invalid_argument);
    BOOST_REQUIRE_THROW( Opm::StarToken st("*123") , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(CountCorrect) {
    Opm::StarToken st1("*");
    Opm::StarToken st2("5*");
    Opm::StarToken st3("54*");
    BOOST_CHECK(st1.countString() == "");
    BOOST_CHECK(st2.countString() == "5");
    BOOST_CHECK(st3.countString() == "54");

    BOOST_CHECK(st1.valueString() == "");
    BOOST_CHECK(st2.valueString() == "");
    BOOST_CHECK(st3.valueString() == "");

    BOOST_CHECK(!st1.hasValue());
    BOOST_CHECK(!st2.hasValue());
    BOOST_CHECK(!st3.hasValue());

    BOOST_REQUIRE_EQUAL(1U , st1.count());
    BOOST_REQUIRE_EQUAL(5U , st2.count());
    BOOST_REQUIRE_EQUAL(54U , st3.count());
}


BOOST_AUTO_TEST_CASE(NoValueGetValueThrow) {
    Opm::StarToken st1("*");
    Opm::StarToken st2("5*");
    BOOST_CHECK_EQUAL( false , st1.hasValue());
    BOOST_CHECK_EQUAL( false , st2.hasValue());
}

BOOST_AUTO_TEST_CASE(StarNoCountThrows) {
    BOOST_CHECK_THROW( Opm::StarToken st1("*10") , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(CorrectValueString) {
    Opm::StarToken st1("1*10.09");
    Opm::StarToken st2("5*20.13");
    Opm::StarToken st3("1*'123'");
    Opm::StarToken st4("1*123*456");
    BOOST_CHECK_EQUAL( true , st1.hasValue());
    BOOST_CHECK_EQUAL( true , st2.hasValue());
    BOOST_CHECK_EQUAL( true , st3.hasValue());
    BOOST_CHECK_EQUAL( true , st4.hasValue());

    BOOST_CHECK_EQUAL( "10.09" , st1.valueString());
    BOOST_CHECK_EQUAL( "20.13" , st2.valueString());
    BOOST_CHECK_EQUAL( "'123'" , st3.valueString());
    BOOST_CHECK_EQUAL( "123*456" , st4.valueString());
}

BOOST_AUTO_TEST_CASE( ContainsStar_WithStar_ReturnsTrue ) {
    std::string countString, valueString;
    BOOST_CHECK_EQUAL( true , Opm::isStarToken("*", countString, valueString) );
    BOOST_CHECK_EQUAL( true , Opm::isStarToken("*1", countString, valueString) );
    BOOST_CHECK_EQUAL( true , Opm::isStarToken("1*", countString, valueString) );
    BOOST_CHECK_EQUAL( true , Opm::isStarToken("1*2", countString, valueString) );

    BOOST_CHECK_EQUAL( false , Opm::isStarToken("12", countString, valueString) );
    BOOST_CHECK_EQUAL( false , Opm::isStarToken("'12*34'", countString, valueString) );
}

BOOST_AUTO_TEST_CASE( readValueToken_basic_validity_tests ) {
    BOOST_CHECK_THROW( Opm::readValueToken<int>( std::string( "3.3" ) ), std::invalid_argument );
    BOOST_CHECK_EQUAL( 3, Opm::readValueToken<int>( std::string( "3" ) ) );
    BOOST_CHECK_EQUAL( 3, Opm::readValueToken<int>( std::string( "+3" ) ) );
    BOOST_CHECK_EQUAL( -3, Opm::readValueToken<int>( std::string( "-3" ) ) );
    BOOST_CHECK_THROW( Opm::readValueToken<double>( std::string( "truls" ) ), std::invalid_argument );
    BOOST_CHECK_EQUAL( 0, Opm::readValueToken<double>( std::string( "0" ) ) );
    BOOST_CHECK_EQUAL( 0, Opm::readValueToken<double>( std::string( "0.0" ) ) );
    BOOST_CHECK_EQUAL( 0, Opm::readValueToken<double>( std::string( "+0.0" ) ) );
    BOOST_CHECK_EQUAL( 0, Opm::readValueToken<double>( std::string( "-0.0" ) ) );
    BOOST_CHECK_EQUAL( 0, Opm::readValueToken<double>( std::string( ".0" ) ) );
    BOOST_CHECK_THROW( Opm::readValueToken<double>( std::string( "1.0.0" ) ), std::invalid_argument );
    BOOST_CHECK_THROW( Opm::readValueToken<double>( std::string( "1g0" ) ), std::invalid_argument );
    BOOST_CHECK_THROW( Opm::readValueToken<double>( std::string( "1.23h" ) ), std::invalid_argument );
    BOOST_CHECK_THROW( Opm::readValueToken<double>( std::string( "+1.23h" ) ), std::invalid_argument );
    BOOST_CHECK_THROW( Opm::readValueToken<double>( std::string( "-1.23h" ) ), std::invalid_argument );
    BOOST_CHECK_EQUAL( 3.3, Opm::readValueToken<double>( std::string( "3.3" ) ) );
    BOOST_CHECK_CLOSE( 3.3, Opm::readValueToken<double>( std::string( "3.3e0" ) ), 1e-6 );
    BOOST_CHECK_CLOSE( 3.3, Opm::readValueToken<double>( std::string( "3.3d0" ) ), 1e-6 );
    BOOST_CHECK_CLOSE( 3.3, Opm::readValueToken<double>( std::string( "3.3E0" ) ), 1e-6 );
    BOOST_CHECK_CLOSE( 3.3, Opm::readValueToken<double>( std::string( "3.3D0" ) ), 1e-6 );
    BOOST_CHECK_EQUAL( "OLGA", Opm::readValueToken<std::string>( std::string( "OLGA" ) ) );
    BOOST_CHECK_EQUAL( "OLGA", Opm::readValueToken<std::string>( std::string( "'OLGA'" ) ) );
    BOOST_CHECK_EQUAL( "123*456", Opm::readValueToken<std::string>( std::string( "123*456" ) ) );
    BOOST_CHECK_EQUAL( "123*456", Opm::readValueToken<std::string>( std::string( "'123*456'" ) ) );
}
