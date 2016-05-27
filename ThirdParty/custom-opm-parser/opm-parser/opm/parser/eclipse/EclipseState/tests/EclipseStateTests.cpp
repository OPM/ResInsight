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
#include <boost/filesystem.hpp>

#define BOOST_TEST_MODULE EclipseStateTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/ThresholdPressure.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/SimulationConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/checkDeck.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Fault.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultCollection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/TransMult.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/Units/ConversionFactors.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>

using namespace Opm;


static DeckPtr createDeckTOP() {
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
"PORO \n"
"100*0.10 /\n"
"PERMX \n"
"100*0.25 /\n"
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

ParserPtr parser(new Parser());
return parser->parseString(deckData, ParseContext()) ;
}

BOOST_AUTO_TEST_CASE(GetPOROTOPBased) {
    DeckPtr deck = createDeckTOP();
    EclipseState state(deck , ParseContext());
    const Eclipse3DProperties& props = state.get3DProperties();

    const GridProperty<double>& poro  = props.getDoubleGridProperty( "PORO" );
    const GridProperty<double>& permx = props.getDoubleGridProperty( "PERMX" );

    BOOST_CHECK_EQUAL(1000U , poro.getCartesianSize() );
    BOOST_CHECK_EQUAL(1000U , permx.getCartesianSize() );
    for (size_t i=0; i < poro.getCartesianSize(); i++) {
        BOOST_CHECK_EQUAL( 0.10 , poro.iget(i) );
        BOOST_CHECK_EQUAL( 0.25 * Metric::Permeability , permx.iget(i) );
    }
}

static DeckPtr createDeck() {
const char *deckData =
"RUNSPEC\n"
"\n"
"DIMENS\n"
" 10 10 10 /\n"
"GRID\n"
"FAULTS \n"
"  'F1'  1  1  1  4   1  4  'X' / \n"
"  'F2'  5  5  1  4   1  4  'X-' / \n"
"/\n"
"MULTFLT \n"
"  'F1' 0.50 / \n"
"  'F2' 0.50 / \n"
"/\n"
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

ParserPtr parser(new Parser());
return parser->parseString(deckData, ParseContext()) ;
}


static DeckPtr createDeckNoFaults() {
const char *deckData =
"RUNSPEC\n"
"\n"
"DIMENS\n"
" 10 10 10 /\n"
"GRID\n"
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

ParserPtr parser(new Parser());
return parser->parseString(deckData, ParseContext()) ;
}


BOOST_AUTO_TEST_CASE(CreateSchedule) {
DeckPtr deck = createDeck();
EclipseState state(deck , ParseContext());
ScheduleConstPtr schedule = state.getSchedule();
EclipseGridConstPtr eclipseGrid = state.getInputGrid();

BOOST_CHECK_EQUAL( schedule->getStartTime() , boost::posix_time::ptime(boost::gregorian::date(1998 , 3 , 8 )));
}



static DeckPtr createDeckSimConfig() {
const std::string& inputStr = "RUNSPEC\n"
			  "EQLOPTS\n"
			  "THPRES /\n "
			  "DIMENS\n"
			  "10 3 4 /\n"
			  "\n"
			  "GRID\n"
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


ParserPtr parser(new Parser());
return parser->parseString(inputStr, ParseContext()) ;
}


BOOST_AUTO_TEST_CASE(CreateSimulationConfig) {

DeckPtr deck = createDeckSimConfig();
EclipseState state(deck, ParseContext());
SimulationConfigConstPtr simConf = state.getSimulationConfig();

BOOST_CHECK( simConf->hasThresholdPressure() );

std::shared_ptr<const ThresholdPressure> thresholdPressure = simConf->getThresholdPressure();
BOOST_CHECK_EQUAL(thresholdPressure->size(), 3);
}



BOOST_AUTO_TEST_CASE(PhasesCorrect) {
    DeckPtr deck = createDeck();
    EclipseState state( deck, ParseContext() );
    const auto& tm = state.getTableManager();
    BOOST_CHECK(   tm.hasPhase( Phase::PhaseEnum::OIL ));
    BOOST_CHECK(   tm.hasPhase( Phase::PhaseEnum::GAS ));
    BOOST_CHECK( ! tm.hasPhase( Phase::PhaseEnum::WATER ));
}

BOOST_AUTO_TEST_CASE(TitleCorrect) {
    DeckPtr deck = createDeck();
    EclipseState state( deck, ParseContext() );

    BOOST_CHECK_EQUAL( state.getTitle(), "The title" );
}

BOOST_AUTO_TEST_CASE(IntProperties) {
    DeckPtr deck = createDeck();
    EclipseState state( deck, ParseContext() );

    BOOST_CHECK_EQUAL( false, state.get3DProperties().supportsGridProperty( "NONO" ) );
    BOOST_CHECK_EQUAL( true,  state.get3DProperties().supportsGridProperty( "SATNUM" ) );
    BOOST_CHECK_EQUAL( true,  state.get3DProperties().hasDeckIntGridProperty( "SATNUM" ) );
}


BOOST_AUTO_TEST_CASE(GetProperty) {
    DeckPtr deck = createDeck();
    EclipseState state(deck, ParseContext());

    const auto& satNUM = state.get3DProperties().getIntGridProperty( "SATNUM" );

    BOOST_CHECK_EQUAL(1000U , satNUM.getCartesianSize() );
    for (size_t i=0; i < satNUM.getCartesianSize(); i++)
        BOOST_CHECK_EQUAL( 2 , satNUM.iget(i) );

    BOOST_CHECK_THROW( satNUM.iget(100000) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(GetTransMult) {
    DeckPtr deck = createDeck();
    EclipseState state( deck, ParseContext() );
    std::shared_ptr<const TransMult> transMult = state.getTransMult();

    BOOST_CHECK_EQUAL( 1.0, transMult->getMultiplier( 1, 0, 0, FaceDir::XPlus ) );
    BOOST_CHECK_THROW( transMult->getMultiplier( 1000, FaceDir::XPlus ), std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(GetFaults) {
    DeckPtr deck = createDeck();
    EclipseState state( deck, ParseContext() );
    const auto& faults = state.getFaults();

    BOOST_CHECK( faults.hasFault( "F1" ) );
    BOOST_CHECK( faults.hasFault( "F2" ) );

    const auto& F1 = faults.getFault( "F1" );
    const auto& F2 = faults.getFault( "F2" );

    BOOST_CHECK_EQUAL( 0.50, F1.getTransMult() );
    BOOST_CHECK_EQUAL( 0.25, F2.getTransMult() );

    std::shared_ptr<const TransMult> transMult = state.getTransMult();
    BOOST_CHECK_EQUAL( transMult->getMultiplier( 0, 0, 0, FaceDir::XPlus ), 0.50 );
    BOOST_CHECK_EQUAL( transMult->getMultiplier( 4, 3, 0, FaceDir::XMinus ), 0.25 );
    BOOST_CHECK_EQUAL( transMult->getMultiplier( 4, 3, 0, FaceDir::ZPlus ), 1.00 );
}


BOOST_AUTO_TEST_CASE(FaceTransMults) {
    DeckPtr deck = createDeckNoFaults();
    EclipseState state(deck, ParseContext());
    std::shared_ptr<const TransMult> transMult = state.getTransMult();

    for (int i = 0; i < 10; ++ i) {
        for (int j = 0; j < 10; ++ j) {
            for (int k = 0; k < 10; ++ k) {
                if (k == 1)
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::XPlus), 10.0);
                else
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::XPlus), 1.0);

                if (k == 2)
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::XMinus), 11.0);
                else
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::XMinus), 1.0);

                if (k == 3)
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::YPlus), 12.0);
                else
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::YPlus), 1.0);

                if (k == 4)
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::YMinus), 13.0);
                else
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::YMinus), 1.0);

                if (k == 5)
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::ZPlus), 14.0);
                else
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::ZPlus), 1.0);

                if (k == 6)
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::ZMinus), 15.0);
                else
                    BOOST_CHECK_EQUAL(transMult->getMultiplier(i, j, k, FaceDir::ZMinus), 1.0);
            }
        }
    }
}


static DeckPtr createDeckNoGridOpts() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "FLUXNUM\n"
        "  1000*1 /\n"
        "MULTNUM\n"
        "  1000*1 /\n";

    ParserPtr parser(new Parser());
    return parser->parseString(deckData, ParseContext()) ;
}


static DeckPtr createDeckWithGridOpts() {
    const char *deckData =
        "RUNSPEC\n"
        "GRIDOPTS\n"
        "  'YES'   10 /"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "FLUXNUM\n"
        "  1000*1 /\n"
        "MULTNUM\n"
        "  1000*1 /\n";

    ParserPtr parser(new Parser());
    return parser->parseString(deckData, ParseContext()) ;
}


BOOST_AUTO_TEST_CASE(NoGridOptsDefaultRegion) {
    DeckPtr deck = createDeckNoGridOpts();
    EclipseState state(deck, ParseContext());
    const auto& props   = state.get3DProperties();
    const auto& multnum = props.getIntGridProperty("MULTNUM");
    const auto& fluxnum = props.getIntGridProperty("FLUXNUM");
    const auto  default_kw = props.getDefaultRegionKeyword();
    const auto& def_pro = props.getIntGridProperty(default_kw);

    BOOST_CHECK_EQUAL( &fluxnum  , &def_pro );
    BOOST_CHECK_NE( &fluxnum  , &multnum );
}


BOOST_AUTO_TEST_CASE(WithGridOptsDefaultRegion) {
    DeckPtr deck = createDeckWithGridOpts();
    EclipseState state(deck, ParseContext());
    const auto& props   = state.get3DProperties();
    const auto& multnum = props.getIntGridProperty("MULTNUM");
    const auto& fluxnum = props.getIntGridProperty("FLUXNUM");
    const auto  default_kw = props.getDefaultRegionKeyword();
    const auto& def_pro = props.getIntGridProperty(default_kw);

    BOOST_CHECK_EQUAL( &multnum , &def_pro );
    BOOST_CHECK_NE( &fluxnum  , &multnum );
}

BOOST_AUTO_TEST_CASE(TestIOConfigBaseName) {
    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);
    EclipseState state(*deck, parseContext);
    BOOST_CHECK_EQUAL(state.getIOConfig()->getBaseName(), "SPE1CASE2");
    BOOST_CHECK_EQUAL(state.getIOConfig()->getOutputDir(), "testdata/integration_tests/IOConfig");

    ParserPtr parser2(new Parser());
    DeckConstPtr deck2 = createDeckWithGridOpts();
    EclipseState state2(*deck2, parseContext);
    BOOST_CHECK_EQUAL(state2.getIOConfig()->getBaseName(), "");
    BOOST_CHECK_EQUAL(state2.getIOConfig()->getOutputDir(), ".");
}

BOOST_AUTO_TEST_CASE(TestIOConfigCreation) {
    const char * deckData  =
                          "RUNSPEC\n"
                          "GRIDOPTS\n"
                          "  'YES'   10 /"
                          "\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SCHEDULE\n"
                          "DATES             -- 1\n"
                          " 10  OKT 2008 / \n"
                          "/\n"
                          "RPTRST\n"
                          "BASIC=3 FREQ=2 /\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  JAN 2011 / \n"
                          "/\n";


    ParserPtr parser(new Parser());
    DeckPtr deck = parser->parseString(deckData, ParseContext()) ;
    EclipseState state(deck , ParseContext());

    IOConfigConstPtr ioConfig = state.getIOConfigConst();

    BOOST_CHECK_EQUAL(false, ioConfig->getWriteRestartFile(0));
    BOOST_CHECK_EQUAL(false, ioConfig->getWriteRestartFile(1));
    BOOST_CHECK_EQUAL(true, ioConfig->getWriteRestartFile(2));
    BOOST_CHECK_EQUAL(false, ioConfig->getWriteRestartFile(3));
}


BOOST_AUTO_TEST_CASE(TestIOConfigCreationWithSolutionRPTRST) {
    const char * deckData  =
                          "RUNSPEC\n"
                          "GRIDOPTS\n"
                          "  'YES'   10 /"
                          "\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "SOLUTION\n"
                          "RPTRST\n"
                          "BASIC=1/\n"
                          "RPTRST\n"
                          "BASIC=3 FREQ=5 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SCHEDULE\n"
                          "DATES             -- 1\n"
                          " 10  OKT 2008 / \n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "RPTRST\n"
                          "BASIC=3 FREQ=2 /\n"
                          "DATES             -- 3\n"
                          " 20  JAN 2011 / \n"
                          "/\n";

    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckPtr deck = parser->parseString(deckData, parseContext) ;
    EclipseState state(deck, parseContext);

    IOConfigConstPtr ioConfig = state.getIOConfigConst();

    BOOST_CHECK_EQUAL(true  , ioConfig->getWriteRestartFile(0));
    BOOST_CHECK_EQUAL(false , ioConfig->getWriteRestartFile(1));
    BOOST_CHECK_EQUAL(false , ioConfig->getWriteRestartFile(2));
    BOOST_CHECK_EQUAL(false  , ioConfig->getWriteRestartFile(3));
}



BOOST_AUTO_TEST_CASE(TestIOConfigCreationWithSolutionRPTSOL) {
  const char *deckData =
                        "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "SOLUTION\n"
                        "RPTSOL\n"
                        "RESTART=2\n"
                        "/\n"
                        "START             -- 0 \n"
                        "19 JUN 2007 / \n"
                         "SCHEDULE\n"
                        "DATES             -- 1\n"
                        " 10  OKT 2008 / \n"
                        "/\n"
                        "RPTRST\n"
                        "BASIC=3 FREQ=3\n"
                        "/\n"
                        "DATES             -- 2\n"
                        " 20  JAN 2010 / \n"
                        "/\n"
                        "DATES             -- 3\n"
                        " 20  FEB 2010 / \n"
                        "/\n"
                        "RPTSCHED\n"
                        "RESTART=1\n"
                        "/\n";

    const char *deckData2 =
                          "RUNSPEC\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "SOLUTION\n"
                          "RPTSOL\n"
                          "0 0 0 0 0 0 2\n"
                          "/\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                           "SCHEDULE\n"
                          "DATES             -- 1\n"
                          " 10  OKT 2008 / \n"
                          "/\n"
                          "RPTRST\n"
                          "BASIC=3 FREQ=3\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  FEB 2010 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "RESTART=1\n"
                          "/\n";


    ParseContext parseContext;
    ParserPtr parser(new Parser());

    {   //mnemnonics
        DeckPtr deck = parser->parseString(deckData, parseContext) ;
        EclipseState state(deck, parseContext);

        IOConfigConstPtr ioConfig = state.getIOConfigConst();

        BOOST_CHECK_EQUAL(true, ioConfig->getWriteRestartFile(0));
    }

    {   //old fashion integer mnemonics
        DeckPtr deck = parser->parseString(deckData2, parseContext) ;
        EclipseState state(deck, parseContext);

        IOConfigConstPtr ioConfig = state.getIOConfigConst();

        BOOST_CHECK_EQUAL(true, ioConfig->getWriteRestartFile(0));
    }
}

BOOST_AUTO_TEST_CASE(getRegions) {
    const char* input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "RUNSPEC\n"
            "\n"
            "DIMENS\n"
            " 2 2 1 /\n"
            "GRID\n"
            "DXV \n 2*400 /\n"
            "DYV \n 2*400 /\n"
            "DZV \n 1*400 /\n"
            "REGIONS\n"
            "FIPNUM\n"
            "1 1 2 3 /\n";

    const auto deck = Parser().parseString(input, ParseContext());
    EclipseState es( deck, ParseContext() );

    std::vector< int > ref = { 1, 2, 3 };
    const auto regions = es.getRegions( "FIPNUM" );

    BOOST_CHECK_EQUAL_COLLECTIONS( ref.begin(), ref.end(),
                                   regions.begin(), regions.end() );

    BOOST_CHECK( es.getRegions( "EQLNUM" ).empty() );
}
