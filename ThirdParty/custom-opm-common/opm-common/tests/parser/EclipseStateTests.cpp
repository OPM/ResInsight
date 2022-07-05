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

#include <stdexcept>
#include <iostream>

#define BOOST_TEST_MODULE EclipseStateTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/SimulationConfig/ThresholdPressure.hpp>
#include <opm/input/eclipse/EclipseState/SimulationConfig/SimulationConfig.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/input/eclipse/EclipseState/Grid/Fault.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FaultCollection.hpp>
#include <opm/input/eclipse/EclipseState/Grid/TransMult.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

using namespace Opm;

inline std::string prepath() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

static Deck createDeckTOP() {
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
"1000*0.25 /\n"
"BOX\n"
"1 10 1 10 1 1 /\n"
"PORO \n"
"100*0.10 /\n"
"PERMX \n"
"100*0.25 /\n"
"ENDBOX\n"
"EDIT\n"
"OIL\n"
"\n"
"GAS\n"
"\n"
"TITLE\n"
"The title\n"
"\n"
"START\n"
"8 MAR 1998 /\n"
"\n"
"PROPS\n"
"REGIONS\n"
"SWAT\n"
"1000*1 /\n"
"SATNUM\n"
"1000*2 /\n"
"\n";

    Parser parser;
    return parser.parseString( deckData );
}

BOOST_AUTO_TEST_CASE(GetPOROTOPBased) {
    auto deck = createDeckTOP();
    EclipseState state(deck );
    const auto& fp = state.fieldProps();

    const auto& poro  = fp.get_double( "PORO" );
    const auto& permx = fp.get_double( "PERMX" );

    for (size_t i=0; i < poro.size(); i++) {
        BOOST_CHECK_EQUAL( 0.10 , poro[i]);
        BOOST_CHECK_EQUAL( 0.25 * Metric::Permeability , permx[i]);
    }
}

static Deck createDeck() {
const char *deckData =
"RUNSPEC\n"
"\n"
"DIMENS\n"
" 10 10 10 /\n"
"GRID\n"
"DX\n"
"1000*0.25 /\n"
"DY\n"
"1000*0.25 /\n"
"DZ\n"
"1000*0.25 /\n"
"TOPS\n"
"100*0.25 /\n"
"FAULTS \n"
"  'F1'  1  1  1  4   1  4  'X' / \n"
"  'F2'  5  5  1  4   1  4  'X-' / \n"
"/\n"
"MULTFLT \n"
"  'F1' 0.50 / \n"
"  'F2' 0.50 / \n"
"/\n"
"PORO\n"
"  1000*0.15 /"
"EDIT\n"
"MULTFLT /\n"
"  'F2' 0.25 / \n"
"/\n"
"OIL\n"
"\n"
"GAS\n"
"\n"
"TITLE\n"
"The title\n"
"\n"
"START\n"
"8 MAR 1998 /\n"
"\n"
"PROPS\n"
"REGIONS\n"
"SWAT\n"
"1000*1 /\n"
"SATNUM\n"
"1000*2 /\n"
"\n";

    Parser parser;
    return parser.parseString( deckData );
}


static Deck createDeckNoFaults() {
const char *deckData =
"RUNSPEC\n"
"\n"
"DIMENS\n"
" 10 10 10 /\n"
"GRID\n"
"DX\n"
"1000*0.25 /\n"
"DY\n"
"1000*0.25 /\n"
"DZ\n"
"1000*0.25 /\n"
"TOPS\n"
"100*0.25 /\n"
"PORO\n"
"  1000*0.15 /"
"PROPS\n"
"-- multiply one layer for each face\n"
"MULTX\n"
" 100*1 100*10 800*1 /\n"
"MULTX-\n"
" 200*1 100*11 700*1 /\n"
"MULTY\n"
" 300*1 100*12 600*1 /\n"
"MULTY-\n"
" 400*1 100*13 500*1 /\n"
"MULTZ\n"
" 500*1 100*14 400*1 /\n"
"MULTZ-\n"
" 600*1 100*15 300*1 /\n"
"\n";

    Parser parser;
    return parser.parseString( deckData );
}

BOOST_AUTO_TEST_CASE(CreateSchedule) {
    auto deck = createDeck();
    auto python = std::make_shared<Python>();
    EclipseState state(deck);
    Schedule schedule(deck, state, python);
    BOOST_CHECK_EQUAL(schedule.getStartTime(), asTimeT(TimeStampUTC( 1998 , 3 , 8)));
}



static Deck createDeckSimConfig() {
const std::string& inputStr = "RUNSPEC\n"
                "EQLOPTS\n"
                "THPRES /\n "
                "DIMENS\n"
                "10 3 4 /\n"
                "\n"
                "GRID\n"
                "DX\n"
                "120*0.25 /\n"
                "DY\n"
                "120*0.25 /\n"
                "DZ\n"
                "120*0.25 /\n"
                "TOPS\n"
                "30*0.25 /\n"
                "PORO\n"
                "  120*0.15/ \n"
                "REGIONS\n"
                "EQLNUM\n"
                "10*1 10*2 100*3 /\n "
                "\n"
                "SOLUTION\n"
                "THPRES\n"
                "1 2 12.0/\n"
                "1 3 5.0/\n"
                "2 3 7.0/\n"
                "/\n"
                "\n";


    Parser parser;
    return parser.parseString( inputStr );
}

BOOST_AUTO_TEST_CASE(CreateSimulationConfig) {

    auto deck = createDeckSimConfig();
    EclipseState state(deck);
    const auto& simConf = state.getSimulationConfig();

    BOOST_CHECK(simConf.useThresholdPressure());
    BOOST_CHECK_EQUAL(simConf.getThresholdPressure().size(), 3);
}

BOOST_AUTO_TEST_CASE(PhasesCorrect) {
    auto deck = createDeck();
    EclipseState state( deck );
    const auto& phases = state.runspec().phases();
    BOOST_CHECK(  phases.active( Phase::OIL ) );
    BOOST_CHECK(  phases.active( Phase::GAS ) );
    BOOST_CHECK( !phases.active( Phase::WATER ) );
}

BOOST_AUTO_TEST_CASE(TitleCorrect) {
    auto deck = createDeck();
    EclipseState state( deck );

    BOOST_CHECK_EQUAL( state.getTitle(), "The title" );
}

BOOST_AUTO_TEST_CASE(IntProperties) {
    auto deck = createDeck();
    EclipseState state( deck );

    BOOST_CHECK_EQUAL( false, state.fieldProps().supported<int>( "NONO" ) );
    BOOST_CHECK_EQUAL( true,  state.fieldProps().supported<int>( "SATNUM" ) );
    BOOST_CHECK_EQUAL( true,  state.fieldProps().has_int( "SATNUM" ) );
}


BOOST_AUTO_TEST_CASE(GetProperty) {
    auto deck = createDeck();
    EclipseState state(deck);

    const auto& satnum = state.fieldProps().get_global_int("SATNUM");
    BOOST_CHECK_EQUAL(1000U , satnum.size() );
    for (size_t i=0; i < satnum.size(); i++)
        BOOST_CHECK_EQUAL( 2 , satnum[i]);
}

BOOST_AUTO_TEST_CASE(GetTransMult) {
    auto deck = createDeck();
    EclipseState state( deck );
    const auto& transMult = state.getTransMult();

    BOOST_CHECK_EQUAL( 1.0, transMult.getMultiplier( 1, 0, 0, FaceDir::XPlus ) );
    BOOST_CHECK_THROW( transMult.getMultiplier( 1000, FaceDir::XPlus ), std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(GetFaults) {
    auto deck = createDeck();
    EclipseState state( deck );
    const auto& faults = state.getFaults();

    BOOST_CHECK( faults.hasFault( "F1" ) );
    BOOST_CHECK( faults.hasFault( "F2" ) );

    const auto& F1 = faults.getFault( "F1" );
    const auto& F2 = faults.getFault( "F2" );

    BOOST_CHECK_EQUAL( 0.50, F1.getTransMult() );
    BOOST_CHECK_EQUAL( 0.25, F2.getTransMult() );

    const auto& transMult = state.getTransMult();
    BOOST_CHECK_EQUAL( transMult.getMultiplier( 0, 0, 0, FaceDir::XPlus ), 0.50 );
    BOOST_CHECK_EQUAL( transMult.getMultiplier( 4, 3, 0, FaceDir::XMinus ), 0.25 );
    BOOST_CHECK_EQUAL( transMult.getMultiplier( 4, 3, 0, FaceDir::ZPlus ), 1.00 );
}


BOOST_AUTO_TEST_CASE(FaceTransMults) {
    auto deck = createDeckNoFaults();
    EclipseState state(deck);
    const auto& transMult = state.getTransMult();

    for (int i = 0; i < 10; ++ i) {
        for (int j = 0; j < 10; ++ j) {
            for (int k = 0; k < 10; ++ k) {
                if (k == 1)
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::XPlus), 10.0);
                else
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::XPlus), 1.0);

                if (k == 2)
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::XMinus), 11.0);
                else
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::XMinus), 1.0);

                if (k == 3)
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::YPlus), 12.0);
                else
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::YPlus), 1.0);

                if (k == 4)
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::YMinus), 13.0);
                else
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::YMinus), 1.0);

                if (k == 5)
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::ZPlus), 14.0);
                else
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::ZPlus), 1.0);

                if (k == 6)
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::ZMinus), 15.0);
                else
                    BOOST_CHECK_EQUAL(transMult.getMultiplier(i, j, k, FaceDir::ZMinus), 1.0);
            }
        }
    }
}


static Deck createDeckNoGridOpts() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "PORO\n"
        "  1000*0.15 /\n"
        "FLUXNUM\n"
        "  1000*1 /\n"
        "MULTNUM\n"
        "  1000*1 /\n";

    Parser parser;
    return parser.parseString(deckData) ;
}


static Deck createDeckWithGridOpts() {
    const char *deckData =
        "RUNSPEC\n"
        "GRIDOPTS\n"
        "  'YES'   10 /"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "PORO\n"
        "  1000*0.15 /\n"
        "FLUXNUM\n"
        "  1000*1 /\n"
        "MULTNUM\n"
        "  1000*1 /\n";

    Parser parser;
    return parser.parseString( deckData );
}


BOOST_AUTO_TEST_CASE(NoGridOptsDefaultRegion) {
    auto deck = createDeckNoGridOpts();
    EclipseState state(deck);
    const auto& fp = state.fieldProps();
    const auto& multnum = fp.get_int("MULTNUM");
    const auto& fluxnum = fp.get_int("FLUXNUM");
    const auto  default_kw = fp.default_region();
    const auto& def_pro = fp.get_int(default_kw);

    BOOST_CHECK_EQUAL( &fluxnum  , &def_pro );
    BOOST_CHECK_NE( &fluxnum  , &multnum );
}


BOOST_AUTO_TEST_CASE(WithGridOptsDefaultRegion) {
    auto deck = createDeckWithGridOpts();
    EclipseState state(deck);
    const auto& fp = state.fieldProps();
    const auto& multnum = fp.get_int("MULTNUM");
    const auto& fluxnum = fp.get_int("FLUXNUM");
    const auto  default_kw = fp.default_region();
    const auto& def_pro = fp.get_int(default_kw);

    BOOST_CHECK_EQUAL( &multnum , &def_pro );
    BOOST_CHECK_NE( &fluxnum  , &multnum );
}

BOOST_AUTO_TEST_CASE(TestIOConfigBaseName) {
    Parser parser;
    auto deck = parser.parseFile(prepath() + "IOConfig/SPE1CASE2.DATA");
    EclipseState state(deck);
    const auto& io = state.cfg().io();
    BOOST_CHECK_EQUAL(io.getBaseName(), "SPE1CASE2");
    BOOST_CHECK_EQUAL(io.getOutputDir(), prepath() + "IOConfig");

    Parser parser2;
    auto deck2 = createDeckWithGridOpts();
    EclipseState state2(deck2);
    const auto& io2 = state2.cfg().io();
    BOOST_CHECK_EQUAL(io2.getBaseName(), "");
    BOOST_CHECK_EQUAL(io2.getOutputDir(), ".");
}


BOOST_AUTO_TEST_CASE(TestBox) {
    const char * regionData =
                "START             --\n"
                "10 MAI 2007 /\n"
                "RUNSPEC\n"
                "DIMENS\n"
                "2 2 1 /\n"
                "GRID\n"
                "DX\n"
                "4*0.25 /\n"
                "BOX\n"
                "1* 1 1 1 1 1 /\n"
                "DY\n"
                "4*0.25 /\n"
                "DZ\n"
                "4*0.25 /\n"
                "TOPS\n"
                "4*0.25 /\n"
                "ENDBOX\n"
                "PORO\n"
                "  4*0.15 /\n"
                "REGIONS\n"
                "OPERNUM\n"
                "3 3 1 2 /\n"
                "FIPNUM\n"
                "1 1 2 3 /\n";
    Parser parser;
    auto deck = parser.parseString(regionData);
    EclipseState state(deck);

}

