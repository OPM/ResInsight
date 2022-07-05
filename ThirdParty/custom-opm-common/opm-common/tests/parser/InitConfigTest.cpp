/*
  Copyright 2015 Statoil ASA.

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

#define BOOST_TEST_MODULE InitConfigTests

#include <boost/test/unit_test.hpp>

#include <fstream>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/InitConfig/InitConfig.hpp>
#include <opm/input/eclipse/Units/Units.hpp>

#include <tests/WorkArea.hpp>

#include <fstream>

using namespace Opm;


const std::string full_deck1 = R"(
START
7 OCT 2020 /

RUNSPEC

DIMENS
  10 10 3 /

UNIFIN

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  3*10.0 /

DEPTHZ
  121*2000.0 /

PORO
  300*0.3 /

SOLUTION

RESTART
  NOBASE 6 /

SCHEDULE
)";

const std::string full_deck2 = R"(
START
7 OCT 2020 /

RUNSPEC

DIMENS
  10 10 3 /

UNIFIN

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  3*10.0 /

DEPTHZ
  121*2000.0 /

PORO
  300*0.3 /

SOLUTION

RESTART
  BASE 6 /

SCHEDULE
)";




const std::string& deckStr =
    "RUNSPEC\n"
    "DIMENS\n"
    " 10 10 10 /\n"
    "SOLUTION\n"
    "RESTART\n"
    "BASE 5\n"
    "/\n"
    "GRID\n"
    "START             -- 0 \n"
    "19 JUN 2007 / \n"
    "SCHEDULE\n"
    "SKIPREST \n";


const std::string& deckStr2 =
    "RUNSPEC\n"
    "DIMENS\n"
    " 10 10 10 /\n"
    "SOLUTION\n"
    "GRID\n"
    "START             -- 0 \n"
    "19 JUN 2007 / \n"
    "SCHEDULE\n";


const std::string& deckStr3 =
    "RUNSPEC\n"
    "DIMENS\n"
    " 10 10 10 /\n"
    "SOLUTION\n"
    "RESTART\n"
    "BASE 5 SAVE UNFORMATTED /\n"
    "GRID\n"
    "START             -- 0 \n"
    "19 JUN 2007 / \n"
    "SCHEDULE\n"
    "SKIPREST \n";

const std::string& deckStr4 =
    "RUNSPEC\n"
    "DIMENS\n"
    " 10 10 10 /\n"
    "SOLUTION\n"
    "RESTART\n"
    "BASE 5 /\n"
    "GRID\n"
    "START             -- 0 \n"
    "19 JUN 2007 / \n"
    "SCHEDULE\n";

const std::string& deckStr5 =
  "RUNSPEC\n"
  "DIMENS\n"
  " 10 10 10 /\n"
  "SOLUTION\n"
  "RESTART\n"
  "'/abs/path/BASE' 5 /\n"
  "GRID\n"
  "START             -- 0 \n"
  "19 JUN 2007 / \n"
  "SCHEDULE\n";

const std::string& deckWithEquil =
    "RUNSPEC\n"
    "DIMENS\n"
    " 10 10 10 /\n"
    "EQLDIMS\n"
    "1  100  20  1  1  /\n"
    "SOLUTION\n"
    "RESTART\n"
    "BASE 5\n"
    "/\n"
    "EQUIL\n"
    "2469   382.4   1705.0  0.0    500    0.0     1     1      20 /\n"
    "GRID\n"
    "START             -- 0 \n"
    "19 JUN 2007 / \n"
    "SCHEDULE\n"
    "SKIPREST \n";

static Deck createDeck(const std::string& input) {
    Opm::Parser parser;
    auto deck = parser.parseString(input);
    // The call to setDataFile is completely bogus, it is just to ensure that a
    // meaningfull path for the input file has been specified - so that we can
    // locate restart files.
    deck.setDataFile("SPE1CASE1.DATA");
    return deck;
}

BOOST_AUTO_TEST_CASE(EclipseStateTest) {
    Deck deck1 = createDeck(full_deck1);
    // This throws because the restart file does not exist
    BOOST_CHECK_THROW(EclipseState{deck1}, std::exception);

    Deck deck2 = createDeck(full_deck2);
    // This throws because the restart file does not contain the requested report step
    BOOST_CHECK_THROW(EclipseState{deck2}, std::exception);
}

BOOST_AUTO_TEST_CASE(InitConfigTest) {

    Deck deck = createDeck(deckStr);
    InitConfig cfg( deck );
    BOOST_CHECK_EQUAL(cfg.restartRequested(), true);
    BOOST_CHECK_EQUAL(cfg.getRestartStep(), 5);
    BOOST_CHECK_EQUAL(cfg.getRestartRootName(), "BASE");

    Deck deck2 = createDeck(deckStr2);
    InitConfig cfg2( deck2 );
    BOOST_CHECK_EQUAL(cfg2.restartRequested(), false);
    BOOST_CHECK_EQUAL(cfg2.getRestartStep(), 0);
    BOOST_CHECK_EQUAL(cfg2.getRestartRootName(), "");

    cfg2.setRestart( "CASE" , 100);
    BOOST_CHECK_EQUAL(cfg2.restartRequested(), true);
    BOOST_CHECK_EQUAL(cfg2.getRestartStep(), 100);
    BOOST_CHECK_EQUAL(cfg2.getRestartRootName(), "CASE");

    Deck deck3 = createDeck(deckStr3);
    BOOST_CHECK_THROW( InitConfig{ deck3 }, std::runtime_error );

    Deck deck4 = createDeck(deckStr4);
    BOOST_CHECK_NO_THROW( InitConfig{ deck4 } );
}

BOOST_AUTO_TEST_CASE( InitConfigWithoutEquil ) {
    auto deck = createDeck( deckStr );
    InitConfig config( deck );

    BOOST_CHECK( !config.hasEquil() );
    BOOST_CHECK_THROW( config.getEquil(), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( InitConfigWithEquil )  {
    auto deck = createDeck( deckWithEquil );
    InitConfig config( deck );

    BOOST_CHECK( config.hasEquil() );
    BOOST_CHECK_NO_THROW( config.getEquil() );
}

BOOST_AUTO_TEST_CASE( EquilOperations ) {
    auto deck = createDeck( deckWithEquil );
    InitConfig config( deck );

    const auto& equil = config.getEquil();

    BOOST_CHECK( !equil.empty() );
    BOOST_CHECK_EQUAL( 1U, equil.size() );

    BOOST_CHECK_NO_THROW( equil.getRecord( 0 ) );
    BOOST_CHECK_THROW( equil.getRecord( 1 ), std::out_of_range );

    const auto& record = equil.getRecord( 0 );
    BOOST_CHECK_CLOSE( 2469, record.datumDepth(), 1e-12 );
    BOOST_CHECK_CLOSE( 382.4 * unit::barsa, record.datumDepthPressure(), 1e-12 );
    BOOST_CHECK_CLOSE( 1705.0, record.waterOilContactDepth(), 1e-12 );
    BOOST_CHECK_CLOSE( 0.0, record.waterOilContactCapillaryPressure(), 1e-12 );
    BOOST_CHECK_CLOSE( 500, record.gasOilContactDepth(), 1e-12 );
    BOOST_CHECK_CLOSE( 0.0, record.gasOilContactCapillaryPressure(), 1e-12 );
    BOOST_CHECK( !record.liveOilInitConstantRs() );
    BOOST_CHECK( !record.wetGasInitConstantRv() );
    BOOST_CHECK_EQUAL( 20, record.initializationTargetAccuracy() );
}

BOOST_AUTO_TEST_CASE(RestartCWD) {
    WorkArea output_area;

    output_area.makeSubDir("simulation");

    {
        std::fstream fs;
        fs.open ("simulation/CASE.DATA", std::fstream::out);
        fs << deckStr4;
        fs.close();

        fs.open("simulation/CASE5.DATA", std::fstream::out);
        fs << deckStr5;
        fs.close();

        fs.open("CASE5.DATA", std::fstream::out);
        fs << deckStr5;
        fs.close();

        fs.open("CWD_CASE.DATA", std::fstream::out);
        fs << deckStr4;
        fs.close();
    }
    Opm::Parser parser;
    {
        Opm::Deck deck = parser.parseFile("simulation/CASE.DATA");
        Opm::InitConfig init_config(deck);
        BOOST_CHECK_EQUAL(init_config.getRestartRootName(), "simulation/BASE");
    }
    {
      Opm::Deck deck = parser.parseFile("simulation/CASE5.DATA");
      Opm::InitConfig init_config(deck);
      BOOST_CHECK_EQUAL(init_config.getRestartRootName(), "/abs/path/BASE");
    }
    {
        Opm::Deck deck = parser.parseFile("CWD_CASE.DATA");
        Opm::InitConfig init_config(deck);
        BOOST_CHECK_EQUAL(init_config.getRestartRootName(), "BASE");
    }
    {
      Opm::Deck deck = parser.parseFile("CASE5.DATA");
      Opm::InitConfig init_config(deck);
      BOOST_CHECK_EQUAL(init_config.getRestartRootName(), "/abs/path/BASE");
    }
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (FILLEPS)

BOOST_AUTO_TEST_CASE(WrongSection)
{
    // FILLEPS in GRID section (should ideally be caught at load time)
    auto input = std::string { R"(
RUNSPEC

DIMENS
  5 5 3 /

TITLE
Break FILLEPS Keyword

START
  24 'JUN' 2019 /

GAS
OIL
WATER
DISGAS
METRIC

TABDIMS
/

GRID
INIT

DXV
  5*100 /

DYV
  5*100 /

DZV
  3*10 /

TOPS
  25*2000 /

EQUALS
  PERMX 100 /
/

COPY
  PERMX PERMY /
  PERMX PERMZ /
/

MULTIPLY
  PERMZ 0.1 /
/

PORO
  75*0.3 /

-- Wrong section (should be in PROPS)
FILLEPS

PROPS

SWOF
  0 0 1 0
  1 1 0 0 /

SGOF
  0 0 1 0
  1 1 0 0 /

DENSITY
  900 1000 1 /

PVTW
  400 1 1.0E-06 1 0 /

PVDG
   30  0.04234     0.01344
  530  0.003868    0.02935
/

PVTO
    0.000       1.0    1.07033 0.645
              500.0    1.02339 1.029  /
   17.345      25.0    1.14075 0.484
              500.0    1.07726 0.834  /
   31.462      50.0    1.18430 0.439
              500.0    1.11592 0.757  /
   45.089      75.0    1.22415 0.402
              500.0    1.15223 0.689  /
/

END
)" };

    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(input)
    };

    // Keyword present but placed in wrong section => treat as absent
    BOOST_CHECK(! es.cfg().init().filleps());
}

BOOST_AUTO_TEST_CASE(Present)
{
    auto input = std::string { R"(
RUNSPEC

DIMENS
  5 5 3 /

TITLE
Break FILLEPS Keyword

START
  24 'JUN' 2019 /

GAS
OIL
WATER
DISGAS
METRIC

TABDIMS
/

GRID
INIT

DXV
  5*100 /

DYV
  5*100 /

DZV
  3*10 /

TOPS
  25*2000 /

EQUALS
  PERMX 100 /
/

COPY
  PERMX PERMY /
  PERMX PERMZ /
/

MULTIPLY
  PERMZ 0.1 /
/

PORO
  75*0.3 /

PROPS

SWOF
  0 0 1 0
  1 1 0 0 /

SGOF
  0 0 1 0
  1 1 0 0 /

DENSITY
  900 1000 1 /

PVTW
  400 1 1.0E-06 1 0 /

PVDG
   30  0.04234     0.01344
  530  0.003868    0.02935
/

PVTO
    0.000       1.0    1.07033 0.645
              500.0    1.02339 1.029  /
   17.345      25.0    1.14075 0.484
              500.0    1.07726 0.834  /
   31.462      50.0    1.18430 0.439
              500.0    1.11592 0.757  /
   45.089      75.0    1.22415 0.402
              500.0    1.15223 0.689  /
/

FILLEPS

END
)" };

    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(input)
    };

    BOOST_CHECK(es.cfg().init().filleps());
}

BOOST_AUTO_TEST_CASE(Absent)
{
    auto input = std::string { R"(
RUNSPEC

DIMENS
  5 5 3 /

TITLE
Break FILLEPS Keyword

START
  24 'JUN' 2019 /

GAS
OIL
WATER
DISGAS
METRIC

TABDIMS
/

GRID
INIT

DXV
  5*100 /

DYV
  5*100 /

DZV
  3*10 /

TOPS
  25*2000 /

EQUALS
  PERMX 100 /
/

COPY
  PERMX PERMY /
  PERMX PERMZ /
/

MULTIPLY
  PERMZ 0.1 /
/

PORO
  75*0.3 /

PROPS

SWOF
  0 0 1 0
  1 1 0 0 /

SGOF
  0 0 1 0
  1 1 0 0 /

DENSITY
  900 1000 1 /

PVTW
  400 1 1.0E-06 1 0 /

PVDG
   30  0.04234     0.01344
  530  0.003868    0.02935
/

PVTO
    0.000       1.0    1.07033 0.645
              500.0    1.02339 1.029  /
   17.345      25.0    1.14075 0.484
              500.0    1.07726 0.834  /
   31.462      50.0    1.18430 0.439
              500.0    1.11592 0.757  /
   45.089      75.0    1.22415 0.402
              500.0    1.15223 0.689  /
/

-- No FILLEPS here
-- FILLEPS

END
)" };

    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(input)
    };

    BOOST_CHECK(! es.cfg().init().filleps());
}

BOOST_AUTO_TEST_SUITE_END()
