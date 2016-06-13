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

#define BOOST_TEST_MODULE PORVTESTS
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>

static Opm::Eclipse3DProperties getProps(Opm::DeckPtr deck) {
    return Opm::Eclipse3DProperties(*deck, *new Opm::TableManager(*deck), *new Opm::EclipseGrid(deck));
}

static Opm::DeckPtr createCARTDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}





static Opm::DeckPtr createDeckWithPORO() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "PORO\n"
        "100*0.10 100*0.20 100*0.30 100*0.40 100*0.50 100*0.60 100*0.70 100*0.80 100*0.90 100*1.0 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}



static Opm::DeckPtr createDeckWithPORVPORO() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "BOX \n"
        "1 10 1 10 1 1 /\n"
        "PORV\n"
        "100*77 /\n"
        "ENDBOX \n"
        "PORO\n"
        "100*0.10 100*0.20 100*0.30 100*0.40 100*0.50 100*0.60 100*0.70 100*0.80 100*0.90 100*1.0 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createDeckWithMULTPV() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "BOX \n"
        "1 10 1 10 1 1 /\n"
        "PORV\n"
        "100*77 /\n"
        "ENDBOX \n"
        "PORO\n"
        "100*0.10 100*0.20 100*0.30 100*0.40 100*0.50 100*0.60 100*0.70 100*0.80 100*0.90 100*1.0 /\n"
        "EDIT\n"
        "BOX \n"
        "1 5 1 5 1 1 /\n"
        "MULTPV\n"
        "25*10 / \n"
        "ENDBOX \n"
        "BOX \n"
        "1 10 1 10 10 10 /\n"
        "MULTPV\n"
        "100*10 / \n"
        "ENDBOX \n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createDeckWithBOXPORV() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "PORV\n"
        "1000*123.456 /\n"
        "BOX \n"
        "2 3 2 3 2 3 /\n"
        "PORV\n"
        "8*789.012 /\n"
        "ENDBOX\n"
        "MULTPV\n"
        "1000*10 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createDeckWithNTG() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "PORO\n"
        "1000*0.20 /\n"
        "BOX \n"
        "1 1 1 1 1 1 /\n"
        "PORV \n"
        "1*10 /\n"
        "ENDBOX\n"
        "MULTPV\n"
        "1000*10 /\n"
        "NTG\n"
        "1000*2 /\n"
        "EDIT\n"
        "\n";


    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}

BOOST_AUTO_TEST_CASE(PORV_cartesianDeck) {
    /* Check that an exception is raised if we try to create a PORV field without PORO. */
    Opm::DeckPtr deck = createCARTDeck();
    const auto props = getProps(deck);
    const auto& poro = props.getDoubleGridProperty("PORO");
    BOOST_CHECK(poro.containsNaN());
    BOOST_CHECK_THROW(props.getDoubleGridProperty("PORV"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(PORV_initFromPoro) {
    /* Check that the PORV field is correctly calculated from PORO. */
    Opm::DeckPtr deck = createDeckWithPORO();
    const auto props = getProps(deck);
    const auto& poro = props.getDoubleGridProperty("PORO");
    BOOST_CHECK( !poro.containsNaN() );

    const auto& porv = props.getDoubleGridProperty("PORV");
    double cell_volume = 0.25 * 0.25 * 0.25;

    BOOST_CHECK_CLOSE( cell_volume * 0.10 , porv.iget(0,0,0) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 0.10 , porv.iget(9,9,0) , 0.001);

    BOOST_CHECK_CLOSE( cell_volume * 0.50 , porv.iget(0,0,4) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 0.50 , porv.iget(9,9,4) , 0.001);

    BOOST_CHECK_CLOSE( cell_volume * 1.00 , porv.iget(0,0,9) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 1.00 , porv.iget(9,9,9) , 0.001);
}

BOOST_AUTO_TEST_CASE(PORV_initFromPoroWithCellVolume) {
    /* Check that explicit PORV and CellVOlume * PORO can be combined. */
    Opm::DeckPtr deck = createDeckWithPORVPORO();
    const auto props = getProps(deck);
    const auto& porv = props.getDoubleGridProperty("PORV");
    double cell_volume = 0.25 * 0.25 * 0.25;

    BOOST_CHECK_CLOSE( 77.0 , porv.iget(0,0,0) , 0.001);
    BOOST_CHECK_CLOSE( 77.0 , porv.iget(9,9,0) , 0.001);

    BOOST_CHECK_CLOSE( cell_volume * 0.50 , porv.iget(0,0,4) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 0.50 , porv.iget(9,9,4) , 0.001);

    BOOST_CHECK_CLOSE( cell_volume * 1.00 , porv.iget(0,0,9) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 1.00 , porv.iget(9,9,9) , 0.001);
}

BOOST_AUTO_TEST_CASE(PORV_multpv) {
    /* Check that MULTPV is correctly accounted for. */
    Opm::DeckPtr deck = createDeckWithMULTPV();
    const auto props = getProps(deck);
    const auto& porv = props.getDoubleGridProperty("PORV");
    double cell_volume = 0.25 * 0.25 * 0.25;

    BOOST_CHECK_CLOSE( 770.0 , porv.iget(0,0,0) , 0.001);
    BOOST_CHECK_CLOSE( 770.0 , porv.iget(4,4,0) , 0.001);
    BOOST_CHECK_CLOSE( 77.0 , porv.iget(9,9,0) , 0.001);

    BOOST_CHECK_CLOSE( cell_volume * 0.50 , porv.iget(0,0,4) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 0.50 , porv.iget(9,9,4) , 0.001);

    BOOST_CHECK_CLOSE( cell_volume * 0.90 , porv.iget(0,0,8) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 0.90 , porv.iget(9,9,8) , 0.001);

    BOOST_CHECK_CLOSE( cell_volume * 10.00 , porv.iget(0,0,9) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * 10.00 , porv.iget(9,9,9) , 0.001);
}

BOOST_AUTO_TEST_CASE(PORV_mutipleBoxAndMultpv) {
    /* Check that MULTIPLE Boxed PORV and MULTPV statements work */
    Opm::DeckPtr deck = createDeckWithBOXPORV();
    const auto props = getProps(deck);
    const auto& porv = props.getDoubleGridProperty("PORV");

    BOOST_CHECK_CLOSE( 1234.56 , porv.iget(0,0,0) , 0.001);
    BOOST_CHECK_CLOSE( 1234.56 , porv.iget(9,9,9) , 0.001);

    BOOST_CHECK_CLOSE( 7890.12 , porv.iget(1,1,1) , 0.001);
    BOOST_CHECK_CLOSE( 7890.12 , porv.iget(2,2,2) , 0.001);

}

BOOST_AUTO_TEST_CASE(PORV_multpvAndNtg) {
    /* Check that MULTIPLE Boxed PORV and MULTPV statements work and NTG */
    Opm::DeckPtr deck = createDeckWithNTG();
    const auto props = getProps(deck);
    const auto& porv = props.getDoubleGridProperty("PORV");
    double cell_volume = 0.25 * 0.25 * 0.25;
    double poro = 0.20;
    double multpv = 10;
    double NTG = 2;
    double PORV = 10;

    BOOST_CHECK_CLOSE( PORV * multpv                 , porv.iget(0,0,0) , 0.001);
    BOOST_CHECK_CLOSE( cell_volume * poro*multpv*NTG , porv.iget(9,9,9) , 0.001);
}



static Opm::DeckPtr createDeckNakedGRID() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "PORO\n"
        "100*0.10 100*0.20 100*0.30 100*0.40 100*0.50 100*0.60 100*0.70 100*0.80 100*0.90 100*1.0 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


BOOST_AUTO_TEST_CASE(NAKED_GRID_THROWS) {
    /* Check that MULTIPLE Boxed PORV and MULTPV statements work and NTG */
    Opm::DeckPtr deck = createDeckNakedGRID();
    const auto props = getProps(deck);
    BOOST_CHECK_THROW( props.getDoubleGridProperty("PORV") , std::invalid_argument );
}

static Opm::DeckPtr createDeckWithPOROZero() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "PORO\n"
        "100*0.10 100*0.20 100*0.30 100*0.40 100*0.50 100*0.60 100*0.70 100*0.80 100*0.90 100*1.0 /\n"
        "EQUALS\n"
        "  ACTNUM 0 1 10 1 10 2 2 /\n"
        "/\n"
        "EQUALS\n"
        "  PORO 0 1 10 1 10 3 3 /\n"
        "/\n"
        "EQUALS\n"
        "  NTG 0 1 10 1 10 4 4 /\n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}

BOOST_AUTO_TEST_CASE(PORO_ZERO_ACTNUM_CORRECT) {
    /* Check that MULTIPLE Boxed PORV and MULTPV statements work and NTG */
    Opm::DeckPtr deck = createDeckWithPOROZero();
    Opm::EclipseState state( deck , Opm::ParseContext());
    auto grid = state.getInputGrid( );

    /* Top layer is active */
    BOOST_CHECK( grid->cellActive( 0,0,0 ));

    /* Layer k=1 is inactive due to EQUAL ACTNUM */
    BOOST_CHECK( !grid->cellActive(0,0,1));

    /* Layer k = 2 is inactive due t PORO = 0 */
    BOOST_CHECK( !grid->cellActive(0,0,2));

    /* Layer k = 3 is inactive due t NTG = 0 */
    BOOST_CHECK( !grid->cellActive(0,0,2));

    BOOST_CHECK_EQUAL( grid->getNumActive() , 700U );
}
