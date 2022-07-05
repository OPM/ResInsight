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



#define BOOST_TEST_MODULE IOConfigTests

#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>

#include "tests/WorkArea.hpp"
namespace fs = std::filesystem;

using namespace Opm;

const std::string& deckStr =  R"(
RUNSPEC

DIMENS
 10 10 10 /
GRID
GRIDFILE
 0 1 /

START
 21 MAY 1981 /

SCHEDULE
DATES
 22 MAY 1981 /              -- timestep 1
 23 MAY 1981 /              -- timestep 2
 24 MAY 1981 /              -- timestep 3
 25 MAY 1981 /              -- timestep 4
 26 MAY 1981 /              -- timestep 5
 1 JAN 1982 /               -- timestep 6
 1 JAN 1982 13:55:44 /      -- timestep 7
 3 JAN 1982 14:56:45.123 /  -- timestep 8
 4 JAN 1982 14:56:45.123 /  -- timestep 9
 5 JAN 1982 14:56:45.123 /  -- timestep 10
 6 JAN 1982 14:56:45.123 /  -- timestep 11
 7 JAN 1982 14:56:45.123 /  -- timestep 12
 8 JAN 1982 14:56:45.123 /  -- timestep 13
 9 JAN 1982 14:56:45.123 /  -- timestep 14
 10 JAN 1982 14:56:45.123 / -- timestep 15
 11 JAN 1982 14:56:45.123 / -- timestep 16
 1 JAN 1983 /               -- timestep 17
 2 JAN 1983 /               -- timestep 18
 3 JAN 1983 /               -- timestep 19
 1 JAN 1984 /               -- timestep 20
 2 JAN 1984 /               -- timestep 21
 1 JAN 1985 /               -- timestep 22
 3 JAN 1986 14:56:45.123 /  -- timestep 23
 4 JAN 1986 14:56:45.123 /  -- timestep 24
 5 JAN 1986 14:56:45.123 /  -- timestep 25
 1 JAN 1987 /               -- timestep 26
 1 JAN 1988 /               -- timestep 27
 2 JAN 1988 /               -- timestep 28
 3 JAN 1988 /               -- timestep 29
 1 JAN 1989 /               -- timestep 30
 2 JAN 1989 /               -- timestep 31
 2 JAN 1990 /               -- timestep 32
 2 JAN 1991 /               -- timestep 33
 3 JAN 1991 /               -- timestep 34
 4 JAN 1991 /               -- timestep 35
 1 JAN 1992 /               -- timestep 36
 1 FEB 1992 /               -- timestep 37
 1 MAR 1992 /               -- timestep 38
 2 MAR 1992 /               -- timestep 39
 3 MAR 1992 /               -- timestep 40
 4 MAR 1992 /               -- timestep 41
 1 APR 1992 /               -- timestep 42
 2 APR 1992 /               -- timestep 43
 1 MAY 1992 /               -- timestep 44
 2 MAY 1992 /               -- timestep 45
 3 MAY 1992 /               -- timestep 46
 3 JUN 1992 /               -- timestep 47
 3 JUL 1992 /               -- timestep 48
 3 AUG 1992 /               -- timestep 49
 4 AUG 1992 /               -- timestep 50
 5 AUG 1992 /               -- timestep 51
 6 AUG 1992 /               -- timestep 52
 7 AUG 1992 /               -- timestep 53
 8 AUG 1992 /               -- timestep 54
 9 AUG 1992 /               -- timestep 55
 10 AUG 1992 /              -- timestep 56
 11 AUG 1992 /              -- timestep 57
 12 AUG 1992 /              -- timestep 58
 13 AUG 1992 /              -- timestep 59
 14 AUG 1992 /              -- timestep 60
 15 AUG 1992 /              -- timestep 61
/)";

const std::string deckStr_RFT = R"(
RUNSPEC
OIL
GAS
WATER
DIMENS
 10 10 10 /
GRID
DXV
10*0.25 /
DYV
10*0.25 /
DZV
10*0.25 /
TOPS
100*0.25 /

"START             -- 0
1 NOV 1979 /
SCHEDULE
DATES             -- 1
 1 DES 1979/
/
WELSPECS
    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
    'OP_2'       'OP'   4   4 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPDAT
 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 /
 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
 'OP_2'  4  4   4  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/
DATES             -- 2
 10  OKT 2008 /
/
WRFT
/
WELOPEN
 'OP_1' OPEN /
 'OP_2' OPEN /
/
DATES             -- 3
 10  NOV 2008 /
/
)";






BOOST_AUTO_TEST_CASE(DefaultProperties) {
    const char* data =  R"(
RUNSPEC
DIMENS
 10 10 10 /
GRID
START
 21 MAY 1981 /

SCHEDULE
RPTRST
'BASIC = 1'
/
DATES
 22 MAY 1981 /
/
RPTSCHED
RESTART=0
/
DATES
 23 MAY 1981 /
 24 MAY 1981 /
 23 MAY 1982 /
 24 MAY 1982 /
 24 MAY 1983 /
 25 MAY 1984 /
 26 MAY 1984 /
 26 MAY 1985 /
 27 MAY 1985 /
 1 JAN 1986 /
/
)";

    auto deck = Parser().parseString( data);
    IOConfig ioConfig( deck );

    /*If no GRIDFILE nor NOGGF keywords are specified, default output an EGRID file*/
    BOOST_CHECK( ioConfig.getWriteEGRIDFile() );
    /*If no INIT keyword is specified, verify no write of INIT file*/
    BOOST_CHECK( !ioConfig.getWriteINITFile() );
    /*If no UNIFIN keyword is specified, verify UNIFIN false (default is multiple) */
    BOOST_CHECK( !ioConfig.getUNIFIN() );
    /*If no UNIFOUT keyword is specified, verify UNIFOUT false (default is multiple) */
    BOOST_CHECK( !ioConfig.getUNIFOUT() );
    /*If no FMTIN keyword is specified, verify FMTIN false (default is unformatted) */
    BOOST_CHECK( !ioConfig.getFMTIN() );
    /*If no FMTOUT keyword is specified, verify FMTOUT false (default is unformatted) */
    BOOST_CHECK( !ioConfig.getFMTOUT() );
}

BOOST_AUTO_TEST_CASE(OutputProperties) {
    const std::string data = R"(
RUNSPEC
UNIFIN
UNIFOUT
FMTIN
FMTOUT
DIMENS
 10 10 10 /
GRID
NOGGF
INIT
START
 21 MAY 1981 /

SCHEDULE
)";


    auto deck = Parser().parseString( data );
    IOConfig ioConfig( deck );

    BOOST_CHECK( !ioConfig.getWriteEGRIDFile() );
    /*If INIT keyword is specified, verify write of INIT file*/
    BOOST_CHECK( ioConfig.getWriteINITFile() );
    /*If UNIFOUT keyword is specified, verify unified write*/
    BOOST_CHECK( ioConfig.getUNIFOUT() );
    /*If FMTOUT keyword is specified, verify formatted write*/
    BOOST_CHECK( ioConfig.getFMTOUT() );
}

BOOST_AUTO_TEST_CASE(NoGRIDFILE) {
    const char* data =  R"(
RUNSPEC

DIMENS
10 10 10 /
GRID
GRIDFILE
 0 0 /
)";

    auto deck = Parser().parseString( data );
    IOConfig ioConfig( deck );

    /*If GRIDFILE 0 0 is specified, no EGRID file is written */
    BOOST_CHECK( !ioConfig.getWriteEGRIDFile() );
}

void touch_file(const fs::path& file) {
    if (!fs::exists(file)) {
        if (file.has_parent_path()) {
            const auto& parent_path = file.parent_path();
            if (!fs::is_directory(parent_path))
                fs::create_directories(parent_path);
        }
        std::ofstream os{file};
    }
}

BOOST_AUTO_TEST_CASE(OutputPaths) {
    WorkArea wa;

    IOConfig config1( "" );
    BOOST_CHECK_EQUAL("", config1.getBaseName() );

    Deck deck2;
    touch_file("testString.DATA");
    deck2.setDataFile( "testString.DATA" );
    IOConfig config2( deck2 );
    std::string output_dir2 =  ".";
    BOOST_CHECK_EQUAL( output_dir2,  config2.getOutputDir() );
    BOOST_CHECK_EQUAL( "TESTSTRING", config2.getBaseName() );

    namespace fs = std::filesystem;

    Deck deck3;
    touch_file("path/to/testString.DATA");
    deck3.setDataFile( "path/to/testString.DATA" );
    IOConfig config3( deck3 );
    std::string output_dir3 =  "/path/to";
    config3.setOutputDir( output_dir3 );
    auto testpath = fs::path( "/path/to/TESTSTRING" ).make_preferred().string();
    BOOST_CHECK_EQUAL( output_dir3,  config3.getOutputDir() );
    BOOST_CHECK_EQUAL( "TESTSTRING", config3.getBaseName() );
    BOOST_CHECK_EQUAL( testpath, config3.fullBasePath() );
}
