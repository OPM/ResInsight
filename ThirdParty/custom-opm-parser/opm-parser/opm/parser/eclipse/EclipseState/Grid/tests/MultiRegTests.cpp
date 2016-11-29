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

#define BOOST_TEST_MODULE MultiRegTests
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


static Opm::DeckPtr createDeckInvalidArray() {
    const char *deckData =
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
        "REGIONS\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "MULTIREG\n"
        "  SATNUM 10 10 MX / \n"
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
        "REGIONS\n"
        "SATNUM\n"
        "  1000*1 /\n"
        "MULTIREG\n"
        "  SATNUM 0.2 10 M / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createDeckMissingVector() {
    const char *deckData =
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
        "MULTIREG\n"
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
        "  'YES'  2 /\n"
        "\n"
        "DIMENS\n"
        " 5 5 1 /\n"
        "GRID\n"
        "DX\n"
        "25*0.25 /\n"
        "DY\n"
        "25*0.25 /\n"
        "DZ\n"
        "25*0.25 /\n"
        "TOPS\n"
        "25*0.25 /\n"
        "REGIONS\n"
        "SATNUM \n"
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
        "MULTIREG\n"
        "  SATNUM 11 1    M / \n"
        "  SATNUM 20 2      / \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}






BOOST_AUTO_TEST_CASE(InvalidArrayThrows) {
    Opm::DeckPtr deck = createDeckInvalidArray();
    BOOST_CHECK_THROW( new Opm::EclipseState( *deck, Opm::ParseContext()) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(InvalidRegionThrows) {
    Opm::DeckPtr deck = createDeckInvalidRegion();
    BOOST_CHECK_THROW( new Opm::EclipseState( *deck, Opm::ParseContext()) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(ExpectedIntThrows) {
    Opm::DeckPtr deck = createDeckInvalidValue();
    BOOST_CHECK_THROW( new Opm::EclipseState( *deck, Opm::ParseContext()) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(MissingRegionVectorThrows) {
    Opm::DeckPtr deck = createDeckMissingVector();
    BOOST_CHECK_THROW( new Opm::EclipseState( *deck, Opm::ParseContext()) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(UnInitializedVectorThrows) {
    Opm::DeckPtr deck = createDeckUnInitialized();
    BOOST_CHECK_THROW( new Opm::EclipseState( *deck, Opm::ParseContext()) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(IntSetCorrectly) {
    Opm::DeckPtr deck = createValidIntDeck();
    Opm::TableManager tm(*deck);
    Opm::EclipseGrid eg(deck);
    Opm::Eclipse3DProperties props(*deck, tm, eg);

    const auto& property = props.getIntGridProperty("SATNUM");
    for (size_t j = 0; j < 5; j++)
        for (size_t i = 0; i < 5; i++) {
            if (i < 2)
                BOOST_CHECK_EQUAL(11, property.iget(i, j, 0));
            else
                BOOST_CHECK_EQUAL(40, property.iget(i, j, 0));
        }
}
