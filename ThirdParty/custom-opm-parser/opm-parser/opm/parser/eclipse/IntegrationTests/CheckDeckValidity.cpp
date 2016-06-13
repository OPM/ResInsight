/*
  Copyright 2014 Andreas Lauser

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
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/checkDeck.hpp>

BOOST_AUTO_TEST_CASE( KeywordInCorrectSection ) {
    Opm::ParserConstPtr parser(new Opm::Parser);

    {
        const char *correctDeckString =
            "RUNSPEC\n"
            "DIMENS\n"
            "3 3 3 /\n"
            "GRID\n"
            "DXV\n"
            "1 1 1 /\n"
            "DYV\n"
            "1 1 1 /\n"
            "DZV\n"
            "1 1 1 /\n"
            "TOPS\n"
            "9*100 /\n"
            "BOX\n"
            "1 3 1 3 1 3 /\n"
            "PROPS\n"
            "SOLUTION\n"
            "SCHEDULE\n";

        auto deck = parser->parseString(correctDeckString, Opm::ParseContext());
        BOOST_CHECK(Opm::checkDeck(deck, parser));
    }

    {
        // wrong section ordering
        const char *correctDeckString =
            "GRID\n"
            "RUNSPEC\n"
            "PROPS\n"
            "SOLUTION\n"
            "SCHEDULE\n";

        auto deck = parser->parseString(correctDeckString, Opm::ParseContext());
        BOOST_CHECK(!Opm::checkDeck(deck, parser));
        BOOST_CHECK(Opm::checkDeck(deck, parser, ~Opm::SectionTopology));
    }

    {
        // the BOX keyword is in a section where it's not supposed to be
        const char *incorrectDeckString =
            "RUNSPEC\n"
            "BOX\n"
            "1 3 1 3 1 3 /\n"
            "DIMENS\n"
            "3 3 3 /\n"
            "GRID\n"
            "DXV\n"
            "1 1 1 /\n"
            "DYV\n"
            "1 1 1 /\n"
            "DZV\n"
            "1 1 1 /\n"
            "TOPS\n"
            "9*100 /\n"
            "PROPS\n"
            "SOLUTION\n"
            "SCHEDULE\n";

        auto deck = parser->parseString(incorrectDeckString, Opm::ParseContext());
        BOOST_CHECK(!Opm::checkDeck(deck, parser));

        // this is supposed to succeed as we don't ensure that all keywords are in the
        // correct section
        BOOST_CHECK(Opm::checkDeck(deck, parser, Opm::SectionTopology));

        // this fails because of the incorrect BOX keyword
        BOOST_CHECK(!Opm::checkDeck(deck, parser, Opm::SectionTopology | Opm::KeywordSection));
    }
}
