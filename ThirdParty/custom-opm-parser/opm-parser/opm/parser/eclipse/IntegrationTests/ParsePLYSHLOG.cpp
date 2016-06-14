/*
  Copyright 2015 Statoil ASA.

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

#define BOOST_TEST_MODULE ParsePLYSHLOG
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>

using namespace Opm;



BOOST_AUTO_TEST_CASE( PARSE_PLYSHLOG_OK) {
    ParserPtr parser(new Parser());
    std::string deckFile("testdata/integration_tests/POLYMER/plyshlog.data");
    DeckPtr deck =  parser->parseFile(deckFile, ParseContext());
    const auto& kw = deck->getKeyword("PLYSHLOG");
    const auto& rec1 = kw.getRecord(0); // reference conditions

    const auto& itemRefPolyConc = rec1.getItem("REF_POLYMER_CONCENTRATION");
    const auto& itemRefSali = rec1.getItem("REF_SALINITY");
    const auto& itemRefTemp = rec1.getItem("REF_TEMPERATURE");

    BOOST_CHECK_EQUAL( true, itemRefPolyConc.hasValue(0) );
    BOOST_CHECK_EQUAL( true, itemRefSali.hasValue(0) );
    BOOST_CHECK_EQUAL( false, itemRefTemp.hasValue(0) );

    BOOST_CHECK_EQUAL( 1.0, itemRefPolyConc.get< double >(0) );
    BOOST_CHECK_EQUAL( 3.0, itemRefSali.get< double >(0) );

    const auto& rec2 = kw.getRecord(1);
    const auto& itemData = rec2.getItem(0);

    BOOST_CHECK_EQUAL( 1.e-7 , itemData.get< double >(0) );
    BOOST_CHECK_EQUAL( 1.0 , itemData.get< double >(1) );
    BOOST_CHECK_EQUAL( 1.0 , itemData.get< double >(2) );
    BOOST_CHECK_EQUAL( 1.2 , itemData.get< double >(3) );
    BOOST_CHECK_EQUAL( 1.e3 , itemData.get< double >(4) );
    BOOST_CHECK_EQUAL( 2.4 , itemData.get< double >(5) );
}
