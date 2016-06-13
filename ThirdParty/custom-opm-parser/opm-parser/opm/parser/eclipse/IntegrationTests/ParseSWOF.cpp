/*
  Copyright (C) 2013 by Andreas Lauser

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
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>

#include <opm/parser/eclipse/EclipseState/Tables/SwofTable.hpp>

using namespace Opm;

// the data which ought to be parsed
const char *parserData =
    "TABDIMS\n"
    "-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT\n"
    "        1     1     30     1     1     1 /\n"
    "\n"
    "--  S_w k_rw k_row p_cow\n"
    "SWOF\n"
    "    0.1  2*     0.0\n"
    "    0.2 0.1 1.0 1.0\n"
    "    0.3  1* 0.9 2.0\n"
    "    0.4 0.3  1* 3.0\n"
    "    0.5 0.5 0.5 4.0\n"
    "    0.6 0.6 0.4  1*\n"
    "    0.7 0.8 0.3 6.0\n"
    "    0.8 0.9 0.2 7.0\n"
    "    0.9 0.5 0.1 8.0\n"
    "    1.0  1* 0.1 9.0 /\n";

static void check_parser(ParserPtr parser) {
    DeckPtr deck =  parser->parseString(parserData, ParseContext());
    const auto& kw1 = deck->getKeyword("SWOF");
    BOOST_CHECK_EQUAL(1U , kw1.size());

    const auto& record0 = kw1.getRecord(0);
    BOOST_CHECK_EQUAL(1U , record0.size());

    const auto& item0 = record0.getItem(0);
    BOOST_CHECK_EQUAL(10U * 4, item0.size());
}

static void check_SwofTable(ParserPtr parser) {
    DeckPtr deck =  parser->parseString(parserData, ParseContext());
    Opm::SwofTable swofTable(deck->getKeyword("SWOF").getRecord(0).getItem(0));

    BOOST_CHECK_EQUAL(10U, swofTable.getSwColumn().size());
    BOOST_CHECK_CLOSE(0.1, swofTable.getSwColumn()[0], 1e-8);
    BOOST_CHECK_CLOSE(1.0, swofTable.getSwColumn().back(), 1e-8);

    BOOST_CHECK_CLOSE(0.1, swofTable.getKrwColumn()[0], 1e-8);
    BOOST_CHECK_CLOSE(0.1, swofTable.getKrwColumn()[1], 1e-8);
    BOOST_CHECK_CLOSE(0.2, swofTable.getKrwColumn()[2], 1e-8);
    BOOST_CHECK_CLOSE(0.3, swofTable.getKrwColumn()[3], 1e-8);
    BOOST_CHECK_CLOSE(0.5, swofTable.getKrwColumn().back(), 1e-8);

    BOOST_CHECK_CLOSE(1.0, swofTable.getKrowColumn()[0], 1e-8);
    BOOST_CHECK_CLOSE(0.9, swofTable.getKrowColumn()[2], 1e-8);
    BOOST_CHECK_CLOSE(0.7, swofTable.getKrowColumn()[3], 1e-8);
    BOOST_CHECK_CLOSE(0.5, swofTable.getKrowColumn()[4], 1e-8);

    BOOST_CHECK_CLOSE(4.0e5, swofTable.getPcowColumn()[4], 1e-8);
    BOOST_CHECK_CLOSE(5.0e5, swofTable.getPcowColumn()[5], 1e-8);
    BOOST_CHECK_CLOSE(6.0e5, swofTable.getPcowColumn()[6], 1e-8);

    BOOST_CHECK_CLOSE(0.10, swofTable.evaluate("KRW", -0.1), 1e-8);
    BOOST_CHECK_CLOSE(0.15, swofTable.evaluate("KRW", 0.25), 1e-8);
    BOOST_CHECK_CLOSE(0.50, swofTable.evaluate("KRW", 1.1), 1e-8);
}

BOOST_AUTO_TEST_CASE( parse_SWOF_OK ) {
    ParserPtr parser(new Parser(/*addDefault=*/true));

    check_parser( parser );
    check_SwofTable(parser);
}
