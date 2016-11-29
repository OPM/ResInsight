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

#define BOOST_TEST_MODULE ParserIntegrationTests
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;



BOOST_AUTO_TEST_CASE( parse_EQUIL_OK ) {
    ParserPtr parser(new Parser());
    std::string pvtgFile("testdata/integration_tests/RSVD/RSVD.txt");
    DeckPtr deck =  parser->parseFile(pvtgFile, ParseContext());
    const auto& kw1 = deck->getKeyword("RSVD" , 0);
    BOOST_CHECK_EQUAL( 6U , kw1.size() );


    const auto& rec1 = kw1.getRecord(0);
    const auto& rec3 = kw1.getRecord(2);

    const auto& item1       = rec1.getItem("table");
    BOOST_CHECK( fabs(item1.getSIDouble(0) - 2382) < 0.001);

    const auto& item3       = rec3.getItem("table");
    BOOST_CHECK( fabs(item3.getSIDouble(7) - 106.77) < 0.001);
}
