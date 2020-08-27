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

#include <opm/common/utility/FileSystem.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/RestartConfig.hpp>

using namespace Opm;

const std::string& deckStr =  "RUNSPEC\n"
                              "\n"
                              "DIMENS\n"
                              " 10 10 10 /\n"
                              "GRID\n"
                              "GRIDFILE\n"
                              " 0 1 /\n"
                              "\n"
                              "START\n"
                              " 21 MAY 1981 /\n"
                              "\n"
                              "SCHEDULE\n"
                              "DATES\n"
                              " 22 MAY 1981 /\n"              // timestep 1
                              " 23 MAY 1981 /\n"              // timestep 2
                              " 24 MAY 1981 /\n"              // timestep 3
                              " 25 MAY 1981 /\n"              // timestep 4
                              " 26 MAY 1981 /\n"              // timestep 5
                              " 1 JAN 1982 /\n"               // timestep 6
                              " 1 JAN 1982 13:55:44 /\n"      // timestep 7
                              " 3 JAN 1982 14:56:45.123 /\n"  // timestep 8
                              " 4 JAN 1982 14:56:45.123 /\n"  // timestep 9
                              " 5 JAN 1982 14:56:45.123 /\n"  // timestep 10
                              " 6 JAN 1982 14:56:45.123 /\n"  // timestep 11
                              " 7 JAN 1982 14:56:45.123 /\n"  // timestep 12
                              " 8 JAN 1982 14:56:45.123 /\n"  // timestep 13
                              " 9 JAN 1982 14:56:45.123 /\n"  // timestep 14
                              " 10 JAN 1982 14:56:45.123 /\n" // timestep 15
                              " 11 JAN 1982 14:56:45.123 /\n" // timestep 16
                              " 1 JAN 1983 /\n"               // timestep 17
                              " 2 JAN 1983 /\n"               // timestep 18
                              " 3 JAN 1983 /\n"               // timestep 19
                              " 1 JAN 1984 /\n"               // timestep 20
                              " 2 JAN 1984 /\n"               // timestep 21
                              " 1 JAN 1985 /\n"               // timestep 22
                              " 3 JAN 1986 14:56:45.123 /\n"  // timestep 23
                              " 4 JAN 1986 14:56:45.123 /\n"  // timestep 24
                              " 5 JAN 1986 14:56:45.123 /\n"  // timestep 25
                              " 1 JAN 1987 /\n"               // timestep 26
                              " 1 JAN 1988 /\n"               // timestep 27
                              " 2 JAN 1988 /\n"               // timestep 28
                              " 3 JAN 1988 /\n"               // timestep 29
                              " 1 JAN 1989 /\n"               // timestep 30
                              " 2 JAN 1989 /\n"               // timestep 31
                              " 2 JAN 1990 /\n"               // timestep 32
                              " 2 JAN 1991 /\n"               // timestep 33
                              " 3 JAN 1991 /\n"               // timestep 34
                              " 4 JAN 1991 /\n"               // timestep 35
                              " 1 JAN 1992 /\n"               // timestep 36
                              " 1 FEB 1992 /\n"               // timestep 37
                              " 1 MAR 1992 /\n"               // timestep 38
                              " 2 MAR 1992 /\n"               // timestep 39
                              " 3 MAR 1992 /\n"               // timestep 40
                              " 4 MAR 1992 /\n"               // timestep 41
                              " 1 APR 1992 /\n"               // timestep 42
                              " 2 APR 1992 /\n"               // timestep 43
                              " 1 MAY 1992 /\n"               // timestep 44
                              " 2 MAY 1992 /\n"               // timestep 45
                              " 3 MAY 1992 /\n"               // timestep 46
                              " 3 JUN 1992 /\n"               // timestep 47
                              " 3 JUL 1992 /\n"               // timestep 48
                              " 3 AUG 1992 /\n"               // timestep 49
                              " 4 AUG 1992 /\n"               // timestep 50
                              " 5 AUG 1992 /\n"               // timestep 51
                              " 6 AUG 1992 /\n"               // timestep 52
                              " 7 AUG 1992 /\n"               // timestep 53
                              " 8 AUG 1992 /\n"               // timestep 54
                              " 9 AUG 1992 /\n"               // timestep 55
                              " 10 AUG 1992 /\n"              // timestep 56
                              " 11 AUG 1992 /\n"              // timestep 57
                              " 12 AUG 1992 /\n"              // timestep 58
                              " 13 AUG 1992 /\n"              // timestep 59
                              " 14 AUG 1992 /\n"              // timestep 60
                              " 15 AUG 1992 /\n"              // timestep 61
                                                        "/\n"
                                                        "\n";

const std::string deckStr_RFT = "RUNSPEC\n"
                                "OIL\n"
                                "GAS\n"
                                "WATER\n"
                                "DIMENS\n"
                                " 10 10 10 /\n"
                                "GRID\n"
                                "DXV\n"
                                "10*0.25 /\n"
                                "DYV\n"
                                "10*0.25 /\n"
                                "DZV\n"
                                "10*0.25 /\n"
                                "TOPS\n"
                                "100*0.25 /\n"
                                "\n"
                                 "START             -- 0 \n"
                                "1 NOV 1979 / \n"
                                "SCHEDULE\n"
                                "DATES             -- 1\n"
                                " 1 DES 1979/ \n"
                                "/\n"
                                "WELSPECS\n"
                                "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                                "    'OP_2'       'OP'   4   4 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                                "/\n"
                                "COMPDAT\n"
                                " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                                " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                                " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                                " 'OP_2'  4  4   4  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                                "/\n"
                                "DATES             -- 2\n"
                                " 10  OKT 2008 / \n"
                                "/\n"
                                "WRFT \n"
                                "/ \n"
                                "WELOPEN\n"
                                " 'OP_1' OPEN / \n"
                                " 'OP_2' OPEN / \n"
                                "/\n"
                                "DATES             -- 3\n"
                                " 10  NOV 2008 / \n"
                                "/\n";






BOOST_AUTO_TEST_CASE(DefaultProperties) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "'BASIC = 1'"
                        "/\n"
                        "DATES\n"
                        " 22 MAY 1981 /\n"
                        "/\n"
                        "RPTSCHED\n"
                        "RESTART=0\n"
                        "/\n"
                        "DATES\n"
                        " 23 MAY 1981 /\n"
                        " 24 MAY 1981 /\n"
                        " 23 MAY 1982 /\n"
                        " 24 MAY 1982 /\n"
                        " 24 MAY 1983 /\n"
                        " 25 MAY 1984 /\n"
                        " 26 MAY 1984 /\n"
                        " 26 MAY 1985 /\n"
                        " 27 MAY 1985 /\n"
                        " 1 JAN 1986 /\n"
                       "/\n";

    auto deck = Parser().parseString( data);
    IOConfig ioConfig( deck );
    RestartConfig rstConfig( TimeMap(deck), deck);

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
    const char* data =  "RUNSPEC\n"
                        "UNIFIN\n"
                        "UNIFOUT\n"
                        "FMTIN\n"
                        "FMTOUT\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "NOGGF\n"
                        "INIT\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n";


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
    const char* data =  "RUNSPEC\n"
                        "\n"
                        "DIMENS\n"
                        "10 10 10 /\n"
                        "GRID\n"
                        "GRIDFILE\n"
                        " 0 0 /\n"
                        "\n";

    auto deck = Parser().parseString( data );
    IOConfig ioConfig( deck );

    /*If GRIDFILE 0 0 is specified, no EGRID file is written */
    BOOST_CHECK( !ioConfig.getWriteEGRIDFile() );
}

BOOST_AUTO_TEST_CASE(OutputPaths) {

    IOConfig config1( "" );
    BOOST_CHECK_EQUAL("", config1.getBaseName() );

    Deck deck2;
    deck2.setDataFile( "testString.DATA" );
    IOConfig config2( deck2 );
    std::string output_dir2 =  ".";
    BOOST_CHECK_EQUAL( output_dir2,  config2.getOutputDir() );
    BOOST_CHECK_EQUAL( "TESTSTRING", config2.getBaseName() );

    namespace fs = Opm::filesystem;

    Deck deck3;
    deck3.setDataFile( "/path/to/testString.DATA" );
    IOConfig config3( deck3 );
    std::string output_dir3 =  "/path/to";
    config3.setOutputDir( output_dir3 );
    auto testpath = fs::path( "/path/to/TESTSTRING" ).make_preferred().string();
    BOOST_CHECK_EQUAL( output_dir3,  config3.getOutputDir() );
    BOOST_CHECK_EQUAL( "TESTSTRING", config3.getBaseName() );
    BOOST_CHECK_EQUAL( testpath, config3.fullBasePath() );
}
