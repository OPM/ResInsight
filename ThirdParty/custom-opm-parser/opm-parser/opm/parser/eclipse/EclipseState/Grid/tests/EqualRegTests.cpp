/*
  Copyright 2014 Statoil ASA.

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

#include <stdexcept>
#include <iostream>
#include <boost/filesystem.hpp>
#include <cstdio>

#define BOOST_TEST_MODULE EqualRegTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>


static Opm::DeckPtr createDeckInvalidArray() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "EQUALREG\n"
        "  MISSING 10 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createDeckInvalidRegion() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "EQUALREG\n"
        "  MISSING 10 10 MX / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createDeckInvalidValue() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "EQUALREG\n"
        "  SATNUM 0.2 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createDeckUnInitialized() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "REGIONS\n"
        "EQUALREG\n"
        "  SATNUM 2 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createValidIntDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "GRIDOPTS\n"
        " 'YES'   2 /"
        "\n"
        "DIMENS\n"
        " 5 5 1 /\n"
        "GRID\n"
        "FLUXNUM \n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "/\n"
        "MULTNUM \n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "/\n"
        "EQUALREG\n"
        "  SATNUM 11 1    M / \n"
        "  SATNUM 20 2      / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createValidPERMXDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "GRIDOPTS\n"
        " 'YES'   2 /"
        "\n"
        "DIMENS\n"
        " 5 5 1 /\n"
        "GRID\n"
        "MULTNUM \n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "/\n"
        "BOX\n"
        "  1 2  1 5 1 1 / \n"
        "PERMZ\n"
        "  10*1 /\n"
        "ENDBOX\n"
        "BOX\n"
        "  3 5  1 5 1 1 / \n"
        "PERMZ\n"
        "  15*2 /\n"
        "ENDBOX\n"
        "EQUALREG\n"
        "  PERMX 1 1     / \n"
        "  PERMX 2 2     / \n"
        "/\n"
        "EQUALS\n"
        "   PERMY 1 1 2 1 5 1 1 / \n"
        "   PERMY 2 3 5 1 5 1 1 / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}




BOOST_AUTO_TEST_CASE(InvalidArrayThrows) {
    Opm::DeckPtr deck = createDeckInvalidArray();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck, Opm::ParseContext()) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(InvalidRegionThrows) {
    Opm::DeckPtr deck = createDeckInvalidRegion();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck, Opm::ParseContext()) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(ExpectedIntThrows) {
    Opm::DeckPtr deck = createDeckInvalidValue();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck, Opm::ParseContext()) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(UnInitializedVectorThrows) {
    Opm::DeckPtr deck = createDeckUnInitialized();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck, Opm::ParseContext()) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(IntSetCorrectly) {
    Opm::DeckPtr deck = createValidIntDeck();
    Opm::TableManager tm(*deck);
    Opm::EclipseGrid eg(deck);
    Opm::Eclipse3DProperties props(*deck, tm, eg);
    auto& property = props.getIntGridProperty("SATNUM");
    for (size_t j = 0; j < 5; j++)
        for (size_t i = 0; i < 5; i++) {
            if (i < 2)
                BOOST_CHECK_EQUAL(11, property.iget(i, j, 0));
            else
                BOOST_CHECK_EQUAL(20, property.iget(i, j, 0));
        }
}

BOOST_AUTO_TEST_CASE(UnitAppliedCorrectly) {
    Opm::DeckPtr deck = createValidPERMXDeck();
    Opm::TableManager tm(*deck);
    Opm::EclipseGrid eg(deck);
    Opm::Eclipse3DProperties props(*deck, tm, eg);
    const auto& permx = props.getDoubleGridProperty("PERMX");
    const auto& permy = props.getDoubleGridProperty("PERMY");
    const auto& permz = props.getDoubleGridProperty("PERMZ");
    for (size_t g = 0; g < 25; g++) {
        BOOST_CHECK_EQUAL(permz.iget(g), permx.iget(g));
        BOOST_CHECK_EQUAL(permy.iget(g), permx.iget(g));
    }
}
