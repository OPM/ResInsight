/*
  Copyright 2019 Equinor ASA.

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

#define BOOST_TEST_MODULE DeckValueTests

#include <vector>

#include <boost/test/unit_test.hpp>

#include <opm/common/utility/numeric/cmp.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckValue.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(DeckValueTest) {

    const DeckValue value0;
    BOOST_CHECK(value0.is_default());
    BOOST_CHECK(!value0.is_compatible<int>());
    BOOST_CHECK(!value0.is_compatible<std::string>());
    BOOST_CHECK(!value0.is_compatible<double>());
    BOOST_CHECK_THROW( value0.get<int>(), std::invalid_argument);
    BOOST_CHECK_THROW( value0.get<std::string>(), std::invalid_argument);
    BOOST_CHECK_THROW( value0.get<double>(), std::invalid_argument);

    DeckValue value1(10);
    BOOST_CHECK(!value1.is_default());
    BOOST_CHECK(value1.is_compatible<int>());
    BOOST_CHECK(value1.is_compatible<double>());
    BOOST_CHECK(!value1.is_compatible<std::string>());
    BOOST_CHECK_EQUAL( value1.get<int>(), 10);
    BOOST_CHECK_EQUAL( value1.get<double>(), 10);

    DeckValue value2(10.0);
    BOOST_CHECK(value2.is_compatible<double>());
    BOOST_CHECK(!value2.is_compatible<int>());
    BOOST_CHECK(!value2.is_compatible<std::string>());
    BOOST_CHECK_EQUAL( value2.get<double>(), 10);
    BOOST_CHECK_THROW( value2.get<std::string>(), std::invalid_argument);
    BOOST_CHECK_THROW( value2.get<int>(), std::invalid_argument);

    DeckValue value3("FUBHP");
    BOOST_CHECK(!value3.is_compatible<double>());
    BOOST_CHECK(value3.is_compatible<std::string>());
    BOOST_CHECK_EQUAL( value3.get<std::string>(), std::string("FUBHP"));
    BOOST_CHECK_THROW( value3.get<double>(), std::invalid_argument);
    BOOST_CHECK_THROW( value3.get<int>(), std::invalid_argument);


}


BOOST_AUTO_TEST_CASE(DeckKeywordConstructor) {

    Parser parser;
    Deck deck;

    UnitSystem& unit_default = deck.getDefaultUnitSystem();
    UnitSystem  unit_active(UnitSystem::UnitType::UNIT_TYPE_LAB);

    const ParserKeyword& big_model = parser.getKeyword("BIGMODEL");
    BOOST_CHECK_THROW( DeckKeyword( big_model, {{DeckValue("WORD_A")}}, unit_active, unit_default ), std::invalid_argument );

    const ParserKeyword& box = parser.getKeyword("BOX");
    std::vector< DeckValue > record1 = {DeckValue(1), DeckValue(2), DeckValue(3), DeckValue(4), DeckValue(5), DeckValue(6)};
    DeckKeyword dkw(box, {record1}, unit_active, unit_default);
    BOOST_CHECK_NO_THROW( DeckKeyword( box, {record1}, unit_active, unit_default ) );
    BOOST_CHECK_THROW( DeckKeyword( box, {{ record1, record1 }}, unit_default, unit_active), std::invalid_argument );

    const ParserKeyword& addreg = parser.getKeyword("ADDREG");

    BOOST_CHECK_NO_THROW( DeckKeyword( addreg, {{ DeckValue("WORD_A") }}, unit_active, unit_default ) );
    BOOST_CHECK_THROW( DeckKeyword( addreg, {{DeckValue("WORD_A"), DeckValue(77), DeckValue(16.25), DeckValue("WORD_B")}}, unit_default, unit_active ) , std::invalid_argument);

    std::vector< DeckValue > record = {DeckValue("WORD_A"), DeckValue(16.25), DeckValue(77), DeckValue("WORD_B")};
    DeckKeyword deck_kw(addreg, {record}, unit_active, unit_default);

    BOOST_CHECK_EQUAL( deck_kw.size(), 1U );
    BOOST_CHECK_MESSAGE( !deck_kw.empty(), "Deck keyword must not be empty" );

    const DeckRecord& deck_record = deck_kw.getRecord(0);
    BOOST_CHECK_EQUAL( deck_record.size(), 4U );

    const auto& array = deck_record.getItem( 0 );
    const auto& shift = deck_record.getItem( 1 );
    const auto& number = deck_record.getItem( 2 );
    const auto& name = deck_record.getItem( 3 );

    BOOST_CHECK_EQUAL( array.get<std::string>(0), "WORD_A" );
    BOOST_CHECK_EQUAL( shift.get<double>(0), 16.25 );
    BOOST_CHECK_EQUAL( number.get<int>(0), 77 );
    BOOST_CHECK_EQUAL( name.get<std::string>(0), "WORD_B" );

    //checking default values:
    record = {DeckValue("WORD_A"), DeckValue(), DeckValue(77)};
    DeckKeyword deck_kw1(addreg, {record}, unit_active, unit_default);

    const DeckRecord& deck_record1 = deck_kw1.getRecord(0);
    const auto& shift1 = deck_record1.getItem( 1 );
    const auto& name1 = deck_record1.getItem( 3 );
    BOOST_CHECK_EQUAL( shift1.get<double>(0), 0 );
    BOOST_CHECK_EQUAL( name1.get<std::string>(0), "M" );

    //check that int can substitute double
    BOOST_CHECK_NO_THROW( DeckKeyword(addreg, {{DeckValue("WORD_A"), DeckValue(5), DeckValue(77)}}, unit_active, unit_default   ) );

    //Check correct SI conversion
    const ParserKeyword& delayact = parser.getKeyword("DELAYACT");
    DeckKeyword delayact_kw( delayact, {{DeckValue("ABC"), DeckValue("DEF"), DeckValue(1.0), DeckValue(8)}}, unit_active, unit_default );
    const auto& deck_record2 = delayact_kw.getRecord(0);
    const auto& delay = deck_record2.getItem( 2 );
    BOOST_CHECK_EQUAL( delay.get<double>(0), 1.0 );
    BOOST_CHECK_EQUAL( delay.getSIDouble(0), 3600.0 );


}


BOOST_AUTO_TEST_CASE(DeckKeywordVectorInt) {

   Parser parser;
   Deck deck;

   UnitSystem& unit_default = deck.getDefaultUnitSystem();
   UnitSystem  unit_active(UnitSystem::UnitType::UNIT_TYPE_LAB);

   const ParserKeyword& hbnum = parser.getKeyword("HBNUM");
   const ParserKeyword& box = parser.getKeyword("BOX");

   std::vector<int> data = {0, 1, 2, 3, 4, 5, 6, 7, 8};
   BOOST_CHECK_THROW( DeckKeyword(box, data), std::invalid_argument );
   DeckKeyword hbnum_kw(hbnum, data);
   BOOST_CHECK(hbnum_kw.isDataKeyword());
   BOOST_CHECK_EQUAL(hbnum_kw.getDataSize(), 9U);
   BOOST_CHECK( hbnum_kw.getIntData() == data );

   std::vector<double> data_double = {1.1, 2.2};
   BOOST_CHECK_THROW(DeckKeyword(hbnum, data_double, unit_active, unit_default), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(DeckKeywordVectorDouble) {

   Parser parser;
   Deck deck;

   UnitSystem& unit_default = deck.getDefaultUnitSystem();
   UnitSystem  unit_active(UnitSystem::UnitType::UNIT_TYPE_LAB);

   const ParserKeyword& zcorn = parser.getKeyword("ZCORN");  //vector of dim length
   const ParserKeyword& box = parser.getKeyword("BOX");

   std::vector<double> data = {1.1, 2.2, 3.3};

   BOOST_CHECK_THROW(DeckKeyword(box, data, unit_active, unit_default), std::invalid_argument);
   DeckKeyword zcorn_kw(zcorn, data, unit_active, unit_default);
   BOOST_CHECK(zcorn_kw.isDataKeyword());
   BOOST_CHECK_EQUAL(zcorn_kw.getDataSize(), 3U);
   BOOST_CHECK( zcorn_kw.getRawDoubleData() == data );
   std::vector<double> SI_data = zcorn_kw.getSIDoubleData();
   BOOST_CHECK( cmp::scalar_equal<double>(SI_data[0], 0.011) );
   BOOST_CHECK( cmp::scalar_equal<double>(SI_data[1], 0.022) );
   BOOST_CHECK( cmp::scalar_equal<double>(SI_data[2], 0.033) );

}






BOOST_AUTO_TEST_CASE(ValueStatus) {
    const std::string deck_string = R"(
PERMX
  100* /
)";

    Parser parser;
    Deck deck = parser.parseString(deck_string);
    const auto& permx = deck["PERMX"].back();
    const auto& status = permx.getValueStatus();
    BOOST_CHECK_EQUAL(status.size(), 100U);
    for (const auto& vs : status) {
        BOOST_CHECK(!value::has_value(vs));
        BOOST_CHECK(vs == value::status::empty_default);
    }
}


