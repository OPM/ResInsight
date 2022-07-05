/*
  Copyright (C) 2014 Statoil ASE

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

#define BOOST_TEST_MODULE ParserIntegrationTests
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/common/utility/OpmInputError.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;



const char *dataMissingRecord = R"(
ENDSCALE
     1*     1*     2 /

ENKRVD
100 1   2  3  4  5  6  7   200 11 22 33 44 55 66 77 /
)";



BOOST_AUTO_TEST_CASE( ParseMissingRECORD_THrows) {
    Parser parser;
    BOOST_CHECK_THROW( parser.parseString( dataMissingRecord ), OpmInputError );
}




const char *data = R"(
ENDSCALE
     1*     1*     3 /

ENKRVD
100 *   2  *  2*    6  7   200 11 22 33     3*55 10 /
100 *   2  3  4  5  6  7   200 11 22 33 44 55 66 77 /
100 *   2  3  4  5  6  7   200 11 22 33 44 55 66 *  /
)";



BOOST_AUTO_TEST_CASE( parse_DATAWithDefult_OK ) {
    Parser parser;
    auto deck = parser.parseString( data );
    const auto& keyword = deck["ENKRVD"].back();
    const auto& rec0 = keyword.getRecord(0);
    const auto& rec1 = keyword.getRecord(1);
    const auto& rec2 = keyword.getRecord(2);

    const auto& item0 = rec0.getItem(0);
    const auto& item1 = rec1.getItem(0);
    const auto& item2 = rec2.getItem(0);

    BOOST_CHECK_EQUAL( 3U , keyword.size());
    BOOST_CHECK( !item0.defaultApplied(0) );
    BOOST_CHECK( item0.defaultApplied(1) );

    BOOST_CHECK_EQUAL( 100 , item0.get< double >(0));
    BOOST_CHECK_EQUAL(  -1 , item0.get< double >(1));
    BOOST_CHECK_EQUAL(  2  , item0.get< double >(2));
    BOOST_CHECK_EQUAL( -1 , item0.get< double >(3));
    BOOST_CHECK_EQUAL( -1 , item0.get< double >(4));
    BOOST_CHECK_EQUAL( -1 , item0.get< double >(5));
    BOOST_CHECK_EQUAL( 6  , item0.get< double >(6));
    BOOST_CHECK_EQUAL( 55 , item0.get< double >(12));
    BOOST_CHECK_EQUAL( 55 , item0.get< double >(13));
    BOOST_CHECK_EQUAL( 55 , item0.get< double >(14));
    BOOST_CHECK_EQUAL( 10 , item0.get< double >(15));

    BOOST_CHECK_EQUAL( 100 , item1.get< double >(0));
    BOOST_CHECK_EQUAL(  -1  , item1.get< double >(1));

    BOOST_CHECK( !item2.defaultApplied(0) );
    BOOST_CHECK( item2.defaultApplied(1) );
}



