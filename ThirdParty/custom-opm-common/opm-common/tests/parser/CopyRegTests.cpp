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

#define BOOST_TEST_MODULE CopyRegTests
#include <boost/test/unit_test.hpp>


#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>


static Opm::Deck createDeckInvalidArray1() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "COPYREG\n"
        "  MISSING SATNUM 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}

static Opm::Deck createDeckInvalidArray2() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "COPYREG\n"
        "  SATNUM MISSING 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}


static Opm::Deck createDeckInvalidTypeMismatch() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "COPYREG\n"
        "  SATNUM PERMX 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}



static Opm::Deck createDeckInvalidRegion() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "COPYREG\n"
        "  SATNUM FLUXNUM 10 MX / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}




static Opm::Deck createDeckUnInitialized() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "REGIONS\n"
        "COPYREG\n"
        "  SATNUM FLUXNUM 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString(deckData) ;
}




BOOST_AUTO_TEST_CASE(InvalidArrayThrows1) {
    Opm::Deck deck = createDeckInvalidArray1();
    BOOST_CHECK_THROW( new Opm::EclipseState(deck) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(InvalidArrayThrows2) {
    Opm::Deck deck = createDeckInvalidArray2();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}



BOOST_AUTO_TEST_CASE(InvalidRegionThrows) {
    Opm::Deck deck = createDeckInvalidRegion();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}




BOOST_AUTO_TEST_CASE(UnInitializedVectorThrows) {
    Opm::Deck deck = createDeckUnInitialized();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(TypeMismatchThrows) {
    Opm::Deck deck = createDeckInvalidTypeMismatch();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}


