/*
  Copyright 2016 Statoil ASA.

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



#define BOOST_TEST_MODULE RestartConfigTests

#include <boost/test/unit_test.hpp>


#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/RestartConfig.hpp>
#include <opm/parser/eclipse/Utility/Functional.hpp>

inline std::string fst( const std::pair< std::string, int >& p ) {
    return p.first;
}

using namespace Opm;


BOOST_AUTO_TEST_CASE(RPTSCHED_INTEGER) {

    const char* deckData1 =
                          "RUNSPEC\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SOLUTION\n"
                          "RPTRST  -- PRES,DEN,PCOW,PCOG,RK,VELOCITY,COMPRESS\n"
                          "  6*0 1 0 1 9*0 1 7*0 1 0 3*1 /\n"
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
                          "RPTRST  -- RK,VELOCITY,COMPRESS\n"
                          "  18*0 0 8*0 /\n"
                          "DATES             -- 3\n"
                          " 20  FEB 2010 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "RESTART=0\n"
                          "/\n";

    Parser parser;

    auto deck1 = parser.parseString( deckData1);
    RestartConfig rstConfig1( TimeMap(deck1), deck1);

    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig1.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  !rstConfig1.getWriteRestartFile( 3 ) );

    std::vector< std::string > kw_list1;
    for( const auto& pair : rstConfig1.getRestartKeywords( 0 ) )
        if( pair.second != 0 ) kw_list1.push_back( pair.first );

    const auto expected1 = {"BG","BO","BW","COMPRESS","DEN","KRG","KRO","KRW","PCOG","PCOW","PRES","RK","VELOCITY","VGAS","VOIL","VWAT"};
    BOOST_CHECK_EQUAL_COLLECTIONS( expected1.begin(), expected1.end(),
                                   kw_list1.begin(), kw_list1.end() );

    // ACIP is a valid mneonic - but not in this deck.
    BOOST_CHECK_EQUAL( rstConfig1.getKeyword( "ACIP" , 0) , 0 );
    BOOST_CHECK_EQUAL( rstConfig1.getKeyword( "COMPRESS" , 0) , 1 );
    BOOST_CHECK_EQUAL( rstConfig1.getKeyword( "PCOG", 0) , 1 );
    BOOST_CHECK_THROW( rstConfig1.getKeyword( "UNKNOWN_KW", 0) , std::invalid_argument);

    std::vector< std::string > kw_list2;
    for( const auto& pair : rstConfig1.getRestartKeywords( 3 ) )
        if( pair.second != 0 ) kw_list2.push_back( pair.first );

    const auto expected2 = { "COMPRESS", "RESTART", "RK", "VELOCITY" };
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );

    BOOST_CHECK_EQUAL( rstConfig1.getKeyword( "ALLPROPS" , 0 ) , 0);
    BOOST_CHECK_EQUAL( rstConfig1.getKeyword( "ALLPROPS" , 3 ) , 0);
}


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

    ParseContext parseContext;
    ErrorGuard errors;
    auto deck = Parser().parseString( data, parseContext, errors );
    parseContext.update(ParseContext::RPT_MIXED_STYLE, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( RestartConfig( TimeMap(deck), deck, parseContext, errors ), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(RPTRST) {

    const char* deckData1 =
                          "RUNSPEC\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SOLUTION\n"
                          "RPTRST\n"
                          " ACIP KRG KRO KRW NORST SFREQ=10 ALLPROPS/\n"
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

    const char* deckData2 =
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
                          "BASIC=3 FREQ=2 FLOWS RUBBISH=5\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  JAN 2011 / \n"
                          "/\n";

    const char* deckData3 =
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

    auto deck1 = parser.parseString( deckData1);
    RestartConfig rstConfig1( TimeMap(deck1), deck1 );

    // Observe that this is true due to some undocumented guessing that
    // the initial restart file should be written if a RPTRST keyword is
    // found in the SOLUTION section, irrespective of the content of that
    // keyword.
    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig1.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 2 ) );


    std::vector<std::string> expected = { "ACIP","BASIC", "BG","BO","BW","DEN","KRG", "KRO", "KRW", "NORST", "SFREQ", "VGAS", "VOIL", "VWAT"};
    const auto kw_list = fun::map( fst, rstConfig1.getRestartKeywords(2) );

    BOOST_CHECK_EQUAL_COLLECTIONS( expected.begin() ,expected.end(),
                                   kw_list.begin() , kw_list.end() );

    BOOST_CHECK_EQUAL( rstConfig1.getKeyword( "ALLPROPS" , 2 ) , 0);

    auto deck2 = parser.parseString( deckData2 );
    RestartConfig rstConfig2( TimeMap(deck2), deck2 );

    const auto expected2 = { "BASIC", "FLOWS", "FREQ" };
    const auto kw_list2 = fun::map( fst, rstConfig2.getRestartKeywords( 2 ) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );

    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig2.getWriteRestartFile( 2 ) );
    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 3 ) );

    auto deck3 = parser.parseString( deckData3 );
    RestartConfig rstConfig3( TimeMap(deck3), deck3 );

    BOOST_CHECK( !rstConfig3.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig3.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig3.getWriteRestartFile( 2 ) );
    BOOST_CHECK( !rstConfig3.getWriteRestartFile( 3 ) );
}



BOOST_AUTO_TEST_CASE(RPTRST_FORMAT_ERROR) {

  const char* deckData0 =
    "RUNSPEC\n"
    "DIMENS\n"
    " 10 10 10 /\n"
    "GRID\n"
    "START             -- 0 \n"
    "19 JUN 2007 / \n"
    "SOLUTION\n"
    "RPTRST\n"
    " ACIP KRG KRO KRW NORST SFREQ=10 ALLPROPS/\n"
    "SCHEDULE\n"
    "DATES             -- 1\n"
    " 10  OKT 2008 / \n"
    "/\n"
    "RPTRST\n"
    "BASIC 1\n"
    "/\n"
    "DATES             -- 2\n"
    " 20  JAN 2010 / \n"
    "/\n";

    const char* deckData1 =
                          "RUNSPEC\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SOLUTION\n"
                          "RPTRST\n"
                          " ACIP KRG KRO KRW NORST SFREQ = 10 ALLPROPS/\n"
                          "SCHEDULE\n"
                          "DATES             -- 1\n"
                          " 10  OKT 2008 / \n"
                          "/\n"
                          "RPTRST\n"
                          "BASIC = 1\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n";

    const char* deckData2 =
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
                          "BASIC = 3 FREQ = 2 FLOWS RUBBISH = 5\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  JAN 2011 / \n"
                          "/\n";

    const char* deckData3 =
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
    ErrorGuard errors;

    auto deck0 = parser.parseString( deckData0, ctx, errors );
    auto deck1 = parser.parseString( deckData1, ctx, errors );
    ctx.update(ParseContext::RPT_UNKNOWN_MNEMONIC, InputError::IGNORE);
    ctx.update(ParseContext::RPT_MIXED_STYLE, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(RestartConfig(TimeMap(deck1), deck1, ctx, errors), std::invalid_argument);

    ctx.update(ParseContext::RPT_MIXED_STYLE, InputError::IGNORE);
    RestartConfig rstConfig1( TimeMap(deck1), deck1, ctx, errors );


    // The case "BASIC 1" - i.e. without '=' can not be salvaged; this should
    // give an exception whatever is the value of ParseContext::RPT_MIXED_STYLE:
    BOOST_CHECK_THROW(RestartConfig(TimeMap(deck0), deck0, ctx, errors), std::invalid_argument);


    // Observe that this is true due to some undocumented guessing that
    // the initial restart file should be written if a RPTRST keyword is
    // found in the SOLUTION section, irrespective of the content of that
    // keyword.
    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig1.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 2 ) );


    std::vector<std::string> expected = { "ACIP","BASIC", "BG","BO","BW","DEN","KRG", "KRO", "KRW", "NORST", "SFREQ", "VGAS", "VOIL", "VWAT"};
    const auto kw_list = fun::map( fst, rstConfig1.getRestartKeywords(2) );

    BOOST_CHECK_EQUAL_COLLECTIONS( expected.begin() ,expected.end(),
                                   kw_list.begin() , kw_list.end() );

    BOOST_CHECK_EQUAL( rstConfig1.getKeyword( "ALLPROPS" , 2 ) , 0);

    auto deck2 = parser.parseString( deckData2, ctx, errors );

    ctx.update(ParseContext::RPT_UNKNOWN_MNEMONIC, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(RestartConfig(TimeMap(deck2), deck2, ctx, errors), std::invalid_argument);
    ctx.update(ParseContext::RPT_UNKNOWN_MNEMONIC, InputError::IGNORE);

    RestartConfig rstConfig2( TimeMap(deck2), deck2, ctx, errors );

    const auto expected2 = { "BASIC", "FLOWS", "FREQ" };
    const auto kw_list2 = fun::map( fst, rstConfig2.getRestartKeywords( 2 ) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );

    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig2.getWriteRestartFile( 2 ) );
    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 3 ) );

    auto deck3 = parser.parseString( deckData3, ctx, errors );
    RestartConfig rstConfig3( TimeMap(deck3), deck3, ctx, errors );

    BOOST_CHECK( !rstConfig3.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig3.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig3.getWriteRestartFile( 2 ) );
    BOOST_CHECK( !rstConfig3.getWriteRestartFile( 3 ) );
}



BOOST_AUTO_TEST_CASE(RPTSCHED) {

    const char* deckData1 =
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


    const char* deckData2 =
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
                          "RESTART=3 FIP\n"
                          "/\n"
                          "DATES             -- 2\n"
                          " 20  JAN 2010 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "RESTART=4\n"
                          "/\n"
                          "DATES             -- 3\n"
                          " 20  FEB 2010 / \n"
                          "/\n"
                          "RPTSCHED\n"
                          "NOTHING RUBBISH\n"
                          "/\n";

    const char* deckData3 =
                          "RUNSPEC\n"
                          "DIMENS\n"
                          " 10 10 10 /\n"
                          "GRID\n"
                          "START             -- 0 \n"
                          "19 JUN 2007 / \n"
                          "SOLUTION\n"
                          "RPTSOL\n"
                          "  RESTART=4 /\n"
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

    auto deck1 = parser.parseString( deckData1 );
    RestartConfig rstConfig1( TimeMap(deck1), deck1 );

    BOOST_CHECK( !rstConfig1.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig1.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  rstConfig1.getWriteRestartFile( 3 ) );


    auto deck2 = parser.parseString( deckData2 );
    RestartConfig rstConfig2( TimeMap(deck2), deck2 );

    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig2.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig2.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  rstConfig2.getWriteRestartFile( 3 ) );

    const auto expected2 = { "FIP", "RESTART" };
    const auto kw_list2 = fun::map( fst, rstConfig2.getRestartKeywords( 2 ) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );


    auto deck3 = parser.parseString( deckData3 );
    RestartConfig rstConfig3( TimeMap(deck3), deck3 );
    //Older ECLIPSE 100 data set may use integer controls instead of mnemonics
    BOOST_CHECK(  rstConfig3.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig3.getWriteRestartFile( 1 ) );
    BOOST_CHECK(  rstConfig3.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  rstConfig3.getWriteRestartFile( 3 ) );

    std::vector<std::string> expected3 = { "BASIC", "FREQ" };
    const auto kw_list3 = fun::map( fst, rstConfig3.getRestartKeywords(2) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected3.begin() , expected3.end() , kw_list3.begin() , kw_list3.end() );
}


BOOST_AUTO_TEST_CASE(RPTSCHED_and_RPTRST) {
  const char* deckData =
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
                        "BASIC=3 FREQ=3 BG BO\n"
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

    auto deck = parser.parseString( deckData );
    RestartConfig rstConfig( TimeMap(deck), deck );

    BOOST_CHECK( !rstConfig.getWriteRestartFile( 0 ) );
    BOOST_CHECK( !rstConfig.getWriteRestartFile( 1 ) );
    BOOST_CHECK( !rstConfig.getWriteRestartFile( 2 ) );
    BOOST_CHECK(  rstConfig.getWriteRestartFile( 3 ) );
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

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

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

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

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

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

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

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

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

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

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
                        "  1 JAN 1982 /\n" // write
                        "  2 JAN 1982 /\n"
                        "  1 FEB 1982 /\n"
                        "  1 MAR 1982 /\n" // write
                        "  1 APR 1983 /\n" // write
                        "  2 JUN 1983 /\n" // write
                        "/\n";

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

    /* BASIC=5, restart file is written at the first report step of each month.
     */
    for( size_t ts : { 1, 2, 3, 4, 7, 8 } )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );

    for( size_t ts : { 5, 6, 9, 10, 11 } )
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

    auto deck = Parser().parseString( data ) ;
    RestartConfig ioConfig( TimeMap(deck), deck );

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

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

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

    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck);

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

    auto deck = Parser().parseString( data );
    RestartConfig ioConfig( TimeMap(deck), deck );

    BOOST_CHECK( ioConfig.getWriteRestartFile( 1 ) );
    for( size_t ts = 2; ts < 11; ++ts )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );
}

BOOST_AUTO_TEST_CASE(RESTART_SAVE) {
    const char* data =  "RUNSPEC\n"
                        "DIMENS\n"
                        " 10 10 10 /\n"
                        "GRID\n"
                        "START\n"
                        " 21 MAY 1981 /\n"
                        "\n"
                        "SCHEDULE\n"
                        "DATES\n"
                        " 22 MAY 1981 /\n"
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
                        "/\n"
                        "SAVE \n"
                        "TSTEP \n"
                        " 1 /\n";
    auto deck = Parser().parseString( data);
    RestartConfig ioConfig( TimeMap(deck), deck );

    for( size_t ts = 1; ts < 11; ++ts )
        BOOST_CHECK( !ioConfig.getWriteRestartFile( ts ) );
    BOOST_CHECK( ioConfig.getWriteRestartFile( 12 ) );

}

