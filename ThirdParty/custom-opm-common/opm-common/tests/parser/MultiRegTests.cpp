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
#include <cstdio>

#define BOOST_TEST_MODULE MultiRegTests
#include <boost/test/unit_test.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>

static Opm::Deck createDeckInvalidArray() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "MULTIREG\n"
        "  MISSING 10 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}


static Opm::Deck createDeckInvalidRegion() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "REGIONS\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "MULTIREG\n"
        "  SATNUM 10 10 MX / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}


static Opm::Deck createDeckInvalidValue() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "REGIONS\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "MULTIREG\n"
        "  SATNUM 0.2 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}


static Opm::Deck createDeckMissingVector() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "REGIONS\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "MULTIREG\n"
        "  SATNUM 2 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}


static Opm::Deck createDeckUnInitialized() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "REGIONS\n"
        "MULTIREG\n"
        "  SATNUM 2 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}


static Opm::Deck createValidIntDeck() {
    const char* deckData =
        "RUNSPEC\n"
        "GRIDOPTS\n"
        "  'YES'  2 /\n"
        "\n"
        "DIMENS\n"
        " 5 5 1 /\n"
        "GRID\n"
        "DX\n"
        "25*1.0 /\n"
        "DY\n"
        "25*1.0 /\n"
        "DZ\n"
        "25*1.0 /\n"
        "TOPS\n"
        "25*0.25 /\n"
        "PERMY\n"
        "   25*1.0 /\n"
        "PERMX\n"
        "   25*1.0 /\n"
        "PORO\n"
        "   25*1.0 /\n"
        "MULTNUM \n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "/\n"
        "REGIONS\n"
        "SATNUM \n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "1  1  2  2 2\n"
        "/\n"
        "OPERNUM \n"
        "1  2  3   4  5\n"
        "6  7  8   9 10\n"
        "11 12 13 14 15\n"
        "16 17 18 19 20\n"
        "21 22 23 24 25\n"
        "/\n"
        "OPERATER\n"
        " PERMX 1 MULTX  PERMY 0.50 /\n"
        " PERMX 2 COPY   PERMY /\n"
        " PORV 1 'MULTX' PORV 0.50 /\n"
        "/\n"
        "MULTIREG\n"
        "  SATNUM 11 1    M / \n"
        "  SATNUM 20 2      / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}






BOOST_AUTO_TEST_CASE(InvalidArrayThrows) {
    Opm::Deck deck = createDeckInvalidArray();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(InvalidRegionThrows) {
    Opm::Deck deck = createDeckInvalidRegion();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(ExpectedIntThrows) {
    Opm::Deck deck = createDeckInvalidValue();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(MissingRegionVectorThrows) {
    Opm::Deck deck = createDeckMissingVector();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(UnInitializedVectorThrows) {
    Opm::Deck deck = createDeckUnInitialized();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(Test_OPERATER) {
    Opm::Deck deck = createValidIntDeck();
    Opm::TableManager tm(deck);
    Opm::EclipseGrid eg(deck);
    Opm::FieldPropsManager fp(deck, Opm::Phases{true, true, true}, eg, tm);

    const auto& porv  = fp.porv(true);
    const auto& permx = fp.get_global_double("PERMX");
    const auto& permy = fp.get_global_double("PERMY");

    BOOST_CHECK_EQUAL( porv[0], 0.50 );
    BOOST_CHECK_EQUAL( porv[1], 1.00 );
    BOOST_CHECK_EQUAL( permx[0] / permy[0], 0.50 );
    BOOST_CHECK_EQUAL( permx[1], permy[1]);
}

