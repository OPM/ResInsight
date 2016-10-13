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

#include <opm/parser/eclipse/EclipseState/Tables/SlgofTable.hpp>

using namespace Opm;

// the data which ought to be parsed
const char *parserData =
    "TABDIMS\n"
    "-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT\n"
    "        1     1     30     1     1     1 /\n"
    "\n"
    "--  S_l k_rg k_rog p_cog\n"
    "SLGOF\n"
    "    0.1 1.0 0.0 9.0\n"
    "    0.2 0.9 0.2 8.0\n"
    "    0.3 0.8 0.3 7.0\n"
    "    0.4 0.7 0.3 6.0\n"
    "    0.5 0.6 0.4 5.0\n"
    "    0.6 0.5 0.5 4.0\n"
    "    0.7 0.3 0.8 3.0\n"
    "    0.8 0.2 0.9 2.0\n"
    "    0.9 0.1 1.0 1.0\n"
    "    1.0 0.0 1.0 0.0 /\n";

static void check_parser(ParserPtr parser) {
    DeckPtr deck =  parser->parseString(parserData, ParseContext());
    const auto& kw1 = deck->getKeyword("SLGOF");
    BOOST_CHECK_EQUAL(1U , kw1.size());

    const auto& record0 = kw1.getRecord(0);
    BOOST_CHECK_EQUAL(1U , record0.size());

    const auto& item0 = record0.getItem(0);
    BOOST_CHECK_EQUAL(10U * 4, item0.size());
}

static void check_SlgofTable(ParserPtr parser) {
    DeckPtr deck =  parser->parseString(parserData, ParseContext());
    Opm::SlgofTable slgofTable(deck->getKeyword("SLGOF").getRecord(0).getItem(0));

    BOOST_CHECK_EQUAL(10U, slgofTable.getSlColumn().size());
    BOOST_CHECK_EQUAL(0.1, slgofTable.getSlColumn()[0]);
    BOOST_CHECK_EQUAL(1.0, slgofTable.getSlColumn()[9]);
    BOOST_CHECK_EQUAL(0.0, slgofTable.getKrgColumn()[9]);
    BOOST_CHECK_EQUAL(1.0, slgofTable.getKrogColumn()[9]);
    BOOST_CHECK_EQUAL(0.0, slgofTable.getPcogColumn()[9]);
}

BOOST_AUTO_TEST_CASE( parse_SLGOF_OK ) {
    ParserPtr parser(new Parser(/*addDefault=*/true));

    check_parser( parser );
    check_SlgofTable(parser);
}
