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

#define BOOST_TEST_MODULE ParserWellProbe
#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(ParseWellProbe) {
    const char *validDeckString =
        "WBHP\n"
        "/\n"
        "\n"
        "WOPR\n"
        "/\n";

    ParserPtr parser(new Parser());

    // TODO: for some reason, the parser does not seem to throw here. Investigate
/*
    const char *invalidDeckString =
        "WELL_PROBE\n"
        "/\n";
    BOOST_CHECK_THROW(parser->parseString(invalidDeckString), std::invalid_argument);
*/

    DeckPtr deck = parser->parseString(validDeckString, ParseContext());
    BOOST_CHECK( !deck->hasKeyword("WELL_PROBE"));
    BOOST_CHECK( deck->hasKeyword("WBHP"));
    BOOST_CHECK( deck->hasKeyword("WOPR"));
    BOOST_CHECK( !deck->hasKeyword("WWPR"));
}
