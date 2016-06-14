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

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>

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



static DeckPtr createDeck(const std::string& input) {
    Opm::Parser parser;
    return parser.parseString(input, Opm::ParseContext());
}


BOOST_AUTO_TEST_CASE( RFT_TIME) {
    DeckPtr deck = createDeck(deckStr_RFT);
    EclipseState state( deck , Opm::ParseContext() );
    std::shared_ptr<const IOConfig> ioConfig = state.getIOConfigConst();


    BOOST_CHECK_EQUAL( ioConfig->getFirstRFTStep() , 2 );
}

BOOST_AUTO_TEST_CASE(RPTRST_mixed_mnemonics_int_list) {
    const char* data = "RUNSPEC\n"
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
                       "BASIC=3 0 1 2\n"
                       "/\n"
                       "DATES             -- 2\n"
                       " 20  JAN 2010 / \n"
                       "/\n"
                       "DATES             -- 3\n"
                       " 20  FEB 2010 / \n"
                       "/\n"
                       "RPTSCHED\n"
                       "BASIC=1\n"
                       "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    BOOST_CHECK_THROW( IOConfig c( *deck ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE(RPTRST) {

    const char *deckData1 =
                          "RUNSPEC\n"
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
                          "BASIC=1\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n";

    const char *deckData2 =
                          "RUNSPEC\n"
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
                          "BASIC=3 FREQ=2 RUBBISH=5\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  JAN 2011 / \n"
                          "/\n";

    const char *deckData3 =
                          "RUNSPEC\n"
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
                          "3 0 0 0 0 2\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  JAN 2011 / \n"
                          "/\n";

    Opm::Parser parser;
    ParseContext ctx;

    auto deck1 = parser.parseString( deckData1, ctx );
    IOConfig ioConfig1( *deck1 );

    BOOST_CHECK( !ioConfig1.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !ioConfig1.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  ioConfig1.getWriteRestartFile( 2 ) );


    auto deck2 = parser.parseString( deckData2, ctx );
    IOConfig ioConfig2( *deck2 );

    BOOST_CHECK( !ioConfig2.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !ioConfig2.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  ioConfig2.getWriteRestartFile( 2 ) );
    BOOST_CHECK( !ioConfig2.getWriteRestartFile( 3 ) );

    auto deck3 = parser.parseString( deckData3, ctx );
    IOConfig ioConfig3( *deck3 );

    BOOST_CHECK( !ioConfig3.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !ioConfig3.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  ioConfig3.getWriteRestartFile( 2 ) );
    BOOST_CHECK( !ioConfig3.getWriteRestartFile( 3 ) );
}

BOOST_AUTO_TEST_CASE(RPTSCHED) {

    const char *deckData1 =
                          "RUNSPEC\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SCHEDULE\n"
                          "DATES             -- 1\n"
                          " 10  OKT 2008 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "RESTART=1\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  FEB 2010 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "RESTART=0\n"
                          "/\n";


    const char *deckData2 =
                          "RUNSPEC\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SCHEDULE\n"
                          "DATES             -- 1\n"
                          " 10  OKT 2008 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "RESTART=1\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  FEB 2010 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "NOTHING RUBBISH\n"
                          "/\n";

    const char *deckData3 =
                          "RUNSPEC\n"
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
                          "BASIC=3 FREQ=1 RUBBISH=5\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  FEB 2010 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "0 0 0 0 0 0 0 0\n"
                          "/\n";

    Parser parser;
    ParseContext ctx;

    auto deck1 = parser.parseString( deckData1, ctx );
    IOConfig ioConfig1( *deck1 );

    BOOST_CHECK( !ioConfig1.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !ioConfig1.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  ioConfig1.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  ioConfig1.getWriteRestartFile( 3 ) );


    auto deck2 = parser.parseString( deckData2, ctx );
    IOConfig ioConfig2( *deck2 );

    BOOST_CHECK( !ioConfig2.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !ioConfig2.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  ioConfig2.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  ioConfig2.getWriteRestartFile( 3 ) );


    auto deck3 = parser.parseString( deckData3, ctx );
    IOConfig ioConfig3( *deck3 );
    //Older ECLIPSE 100 data set may use integer controls instead of mnemonics
    BOOST_CHECK( !ioConfig3.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !ioConfig3.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  ioConfig3.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  ioConfig3.getWriteRestartFile( 3 ) );
}

BOOST_AUTO_TEST_CASE(RPTSCHED_and_RPTRST) {
  const char *deckData =
                        "RUNSPEC\n"
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


    Opm::Parser parser;
    ParseContext ctx;

    auto deck = parser.parseString( deckData, ctx );
    IOConfig ioConfig( *deck );

    BOOST_CHECK( !ioConfig.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !ioConfig.getWriteRestartFile( 1 ) );
    BOOST_CHECK( !ioConfig.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  ioConfig.getWriteRestartFile( 3 ) );
}

BOOST_AUTO_TEST_CASE(NO_BASIC) {
    const char* data = "RUNSPEC\n"
                       "DIMENS\n"
                       " 10 10 10 /\n"
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
                       "DATES             -- 3\n"
                       " 20  FEB 2010 / \n"
                       "/\n"
                       "RPTSCHED\n"
                       "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    for( size_t ts = 0; ts < 4; ++ts )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_1) {
    const char* data = "RUNSPEC\n"
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
                       "BASIC=3 FREQ=3\n"
                       "/\n"
                       "DATES             -- 2\n"
                       " 20  JAN 2010 / \n"
                       "/\n"
                       "DATES             -- 3\n"
                       " 20  FEB 2010 / \n"
                       "/\n"
                       "RPTSCHED\n"
                       "BASIC=1\n"
                       "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    for( size_t ts = 0; ts < 3; ++ts )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );

    BOOST_CHECK( ioConfig.getWriteRestartFile( 3 ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_3) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "BASIC=3 FREQ=3\n"
                        "/\n"
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
                        "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    const size_t freq = 3;

    /* BASIC=3, restart files are created every nth report time, n=3 */
    for( size_t ts = 1; ts < 12; ++ts )
        BOOST_CHECK_EQUAL( ts % freq == 0, ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_4) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "BASIC=4\n"
                        "/\n"
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
                        " 6 JAN 1983 14:56:45.123 /\n"  // timestep 12
                        "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    /* BASIC=4, restart file is written at the first report step of each year.
     */
    for( size_t ts : { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11 } )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );

    for( size_t ts : { 6, 12 } )
        BOOST_CHECK( ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_4_FREQ_2) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "BASIC=4 FREQ=2\n"
                        "/\n"
                        "DATES\n"
                        " 22 MAY 1981 /\n"
                        " 23 MAY 1981 /\n"
                        " 24 MAY 1981 /\n"
                        " 23 MAY 1982 /\n"
                        " 24 MAY 1982 /\n"
                        " 24 MAY 1983 /\n" // write
                        " 25 MAY 1984 /\n"
                        " 26 MAY 1984 /\n"
                        " 26 MAY 1985 /\n" // write
                        " 27 MAY 1985 /\n"
                        " 1 JAN 1986 /\n" 
                        "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    /* BASIC=4, restart file is written at the first report step of each year.
     * Optionally, if the mnemonic FREQ is set >1 the restart is written only
     * every n'th year.
     *
     * FREQ=2
     */
    for( size_t ts : { 1, 2, 3, 4, 5, 7, 8, 10, 11  } )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );

    for( size_t ts : { 6, 9 } )
        BOOST_CHECK( ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_5) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "BASIC=5 FREQ=2\n"
                        "/\n"
                        "DATES\n"
                        " 22 MAY 1981 /\n"
                        " 23 MAY 1981 /\n"
                        " 24 MAY 1981 /\n"
                        "  1 JUN 1981 /\n"
                        "  1 JUL 1981 /\n" // write
                        "  1 JAN 1982 /\n"
                        "  2 JAN 1982 /\n"
                        "  1 FEB 1982 /\n" // write
                        "  1 MAR 1982 /\n" 
                        "  1 APR 1983 /\n" //write
                        "  2 JUN 1983 /\n"
                        "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    /* BASIC=5, restart file is written at the first report step of each month.
     */
    for( size_t ts : { 1, 2, 3, 4, 6, 7, 9, 11  } )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );

    for( size_t ts : { 5, 8, 10 } )
        BOOST_CHECK( ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_0) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "BASIC=0 FREQ=2\n"
                        "/\n"
                        "DATES\n"
                        " 22 MAY 1981 /\n"
                        " 23 MAY 1981 /\n"
                        " 24 MAY 1981 /\n"
                        "  1 JUN 1981 /\n"
                        "  1 JUL 1981 /\n"
                        "  1 JAN 1982 /\n"
                        "  2 JAN 1982 /\n"
                        "  1 FEB 1982 /\n"
                        "  1 MAR 1982 /\n"
                        "  1 APR 1983 /\n"
                        "  2 JUN 1983 /\n"
                        "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    /* RESTART=0, no restart file is written
     */
    for( size_t ts = 0; ts < 11; ++ts )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );
}


BOOST_AUTO_TEST_CASE(RESTART_EQ_0) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTSCHED\n"
                        "RESTART=0\n"
                        "/\n"
                        "DATES\n"
                        " 22 MAY 1981 /\n"
                        " 23 MAY 1981 /\n"
                        " 24 MAY 1981 /\n"
                        "  1 JUN 1981 /\n"
                        "  1 JUL 1981 /\n"
                        "  1 JAN 1982 /\n"
                        "  2 JAN 1982 /\n"
                        "  1 FEB 1982 /\n"
                        "  1 MAR 1982 /\n"
                        "  1 APR 1983 /\n"
                        "  2 JUN 1983 /\n"
                        "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    /* RESTART=0, no restart file is written
     */
    for( size_t ts = 0; ts < 11; ++ts )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(RESTART_BASIC_GT_2) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "BASIC=4 FREQ=2\n"
                        "/\n"
                        "DATES\n"
                        " 22 MAY 1981 /\n"
                        "/\n"
                        "RPTSCHED\n" // BASIC >2, ignore RPTSCHED RESTART
                        "RESTART=3, FREQ=1\n"
                        "/\n"
                        "DATES\n"
                        " 23 MAY 1981 /\n"
                        " 24 MAY 1981 /\n"
                        " 23 MAY 1982 /\n"
                        " 24 MAY 1982 /\n"
                        " 24 MAY 1983 /\n" // write
                        " 25 MAY 1984 /\n"
                        " 26 MAY 1984 /\n"
                        " 26 MAY 1985 /\n" // write
                        " 27 MAY 1985 /\n"
                        " 1 JAN 1986 /\n" 
                       "/\n";

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    for( size_t ts : { 1, 2, 3, 4, 5, 7, 8, 10, 11  } )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );

    for( size_t ts : { 6, 9 } )
        BOOST_CHECK( ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(RESTART_BASIC_LEQ_2) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "RPTRST\n"
                        "BASIC=1"
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

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

    BOOST_CHECK( ioConfig.getWriteRestartFile( 1 ) );
    for( size_t ts = 2; ts < 11; ++ts )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );
}

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
                        "BASIC=1"
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

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

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


    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

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

    auto deck = Parser().parseString( data, ParseContext() );
    IOConfig ioConfig( *deck );

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
    BOOST_CHECK_EQUAL( "testString", config2.getBaseName() );

    Deck deck3;
    deck3.setDataFile( "/path/to/testString.DATA" );
    IOConfig config3( deck3 );
    std::string output_dir3 =  "/path/to";
    config3.setOutputDir( output_dir3 );
    BOOST_CHECK_EQUAL( output_dir3,  config3.getOutputDir() );
    BOOST_CHECK_EQUAL( "testString", config3.getBaseName() );
}
