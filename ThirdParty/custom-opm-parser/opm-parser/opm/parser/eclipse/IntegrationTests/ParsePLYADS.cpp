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

#define BOOST_TEST_MODULE ParsePLYVISC
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/filesystem/path.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>

using namespace Opm;



BOOST_AUTO_TEST_CASE( PARSE_PLYADS_OK) {
    ParserPtr parser(new Parser());
    std::string deckFile("testdata/integration_tests/POLYMER/plyads.data");
    DeckPtr deck =  parser->parseFile(deckFile);
    const auto& = deck->getKeyword("PLYADS");
    const auto& rec = kw.getRecord(0);
    auto* item = rec.getItem(0);

    BOOST_CHECK_EQUAL( 0.0 , item.get< double >(0) );
    BOOST_CHECK_EQUAL( 0.25 , item.get< double >(2) );
}
