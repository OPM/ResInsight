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

#define BOOST_TEST_MODULE EqualRegTests
#include <boost/test/unit_test.hpp>


#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>


static Opm::Deck createDeckInvalidArray() {
    const char* deckData =
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
        "EQUALREG\n"
        "  MISSING 10 10 MX / \n"
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
        "EQUALREG\n"
        "  SATNUM 0.2 10 M / \n"
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
        "EQUALREG\n"
        "  SATNUM 2 10 M / \n"
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

BOOST_AUTO_TEST_CASE(UnInitializedVectorThrows) {
    Opm::Deck deck = createDeckUnInitialized();
    BOOST_CHECK_THROW( new Opm::EclipseState( deck) , std::invalid_argument );
}

