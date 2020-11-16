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
#include <stdlib.h>
#include <iostream>
#include <boost/filesystem.hpp>
#define BOOST_TEST_MODULE ParseContextTests
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/D.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/G.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/O.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/parser/eclipse/Parser/InputErrorAction.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>


#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

using namespace Opm;





BOOST_AUTO_TEST_CASE(TestUnkownKeyword) {
    const char * deck1 =
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 /n"
        "\n";

   const char * deck2 =
       "1rdomTX\n"
       "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 /n"
        "\n";

    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);


    parser.addKeyword<ParserKeywords::DIMENS>();
    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( parser.parseString( deck1 , parseContext , errors) , std::invalid_argument);

    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( parser.parseString( deck1 , parseContext , errors) );

    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::THROW_EXCEPTION );
    parseContext.update(ParseContext::PARSE_RANDOM_TEXT , InputError::IGNORE );
    BOOST_CHECK_THROW( parser.parseString( deck2 , parseContext , errors) , std::invalid_argument);

    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::IGNORE );
    parseContext.update(ParseContext::PARSE_RANDOM_TEXT , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( parser.parseString( deck2 , parseContext , errors) );

    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::IGNORE );
    parseContext.update(ParseContext::PARSE_RANDOM_TEXT , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( parser.parseString( deck2 , parseContext , errors) , std::invalid_argument);

    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::IGNORE );
    parseContext.update(ParseContext::PARSE_RANDOM_TEXT , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( parser.parseString( deck2 , parseContext , errors) );
}


BOOST_AUTO_TEST_CASE(TestUnkownKeywordII) {
    const char * deck1 =
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 /n"
        "\n";


    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);


    parser.addKeyword<ParserKeywords::DIMENS>();
    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( parser.parseString( deck1 , parseContext, errors ) , std::invalid_argument);
    parseContext.ignoreKeyword("RUNSPEC");
    BOOST_CHECK_NO_THROW( parser.parseString( deck1 , parseContext, errors ) );
}


BOOST_AUTO_TEST_CASE(Handle_extra_records) {
    const char * deck_string =
         "EQLDIMS\n"
         "  2  100  20  1  1  /\n"
         "\n"
         "EQUIL\n"
         "  2469   382.4   1705.0  0.0    500    0.0     1     1      20 /\n"
         "  2469   382.4   1705.0  0.0    500    0.0     1     1      20 /\n"
         "  2470   382.4   1705.0  0.0    500    0.0     1     1      20 /\n"
         "GRID\n";

    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);

    parser.addKeyword<ParserKeywords::EQLDIMS>();
    parser.addKeyword<ParserKeywords::EQUIL>();
    parser.addKeyword<ParserKeywords::GRID>();
    BOOST_CHECK_THROW( parser.parseString( deck_string , parseContext, errors ) , std::invalid_argument );

    parseContext.update(ParseContext::PARSE_EXTRA_RECORDS , InputError::IGNORE );
    parser.parseString( deck_string , parseContext, errors );
    BOOST_CHECK( parser.hasKeyword( "GRID" ) );

    parseContext.update(ParseContext::PARSE_EXTRA_RECORDS , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( parser.parseString( deck_string , parseContext, errors ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(Handle_extra_records_2) {
    const char * deck_string =
         "EQLDIMS\n"
         "  2  100  20  1  1  /\n"
         "\n"
         "EQUIL\n"
         "  2469   382.4   1705.0  0.0    500    0.0     1     1      20 /\n"
         "  2469   382.4   1705.0  0.0    500    0.0     1     1      20 /\n"
         "GRID\n"
         "DIMENS\n"
          " 10 10 3 /\n"
          " 5 3 2 /\n";

    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);

    parser.addKeyword<ParserKeywords::EQLDIMS>();
    parser.addKeyword<ParserKeywords::EQUIL>();
    parser.addKeyword<ParserKeywords::GRID>();
    parser.addKeyword<ParserKeywords::DIMENS>();

    parseContext.update(ParseContext::PARSE_EXTRA_RECORDS , InputError::IGNORE );
    BOOST_CHECK_THROW( parser.parseString( deck_string , parseContext, errors ), std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(TestUnkownKeyword_DATA) {
    const char * deck_string1 =
        "RUNSPEC\n"
        "\n"
        "UNKNOWN1\n"
        "\n"
        "UNKNOWN2\n"
        "  10 10 10 /n"
        "\n"
        "UNKNOWN3\n"
        "  11 11 11 /n"
        "/\n"
        "\n"
        "DIMENS\n"
        "  12 12 12 /n"
        "\n";


    const char * deck_string2 =
        "RUNSPEC\n"
        "\n"
        "UNKNOWN1\n"
        "\n"
        "UNKNOWN2\n"
        "  10 10 10 /n"
        "\n"
        "UNKNOWN3\n"
        "  11 11 11 /n"
        "/\n"
        "\n"
        "DIMENS\n"
        "  12 12 12 /\n"
        "Ran/dom Noise; \n"
        "with 0 0 0 Data /\n"
        "/\n"
        "\n";


    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);


    parser.addKeyword<ParserKeywords::RUNSPEC>();
    parser.addKeyword<ParserKeywords::DIMENS>();
    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::IGNORE );
    parseContext.update(ParseContext::PARSE_RANDOM_TEXT , InputError::THROW_EXCEPTION );
    {
        Deck deck = parser.parseString( deck_string1 , parseContext, errors );
        BOOST_CHECK( deck.hasKeyword( "RUNSPEC") );
        BOOST_CHECK( deck.hasKeyword( "DIMENS") );
    }
    BOOST_CHECK_THROW( parser.parseString( deck_string2 , parseContext, errors ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(TEST_UNKNOWN_OPERATE) {
    const char * deck =
        "OPERATE\n"
        "SWL    6* MULTX  PERMX 1.E10       / Temp:  SWL=1.E10*PERMX\n"
        "SWL    6* MINLIM SWL   1.0         /\n"
        "SWL    6* LOG10  SWL               / Temp:  SWL=log(1.E10*PERMX)\n"
        "SWL    6* MULTA  SWL   -0.06  0.91 / Final: SWL=0.31-0.06*log(PERMX)\n"
        "--SWCR 6* COPY   SWL               / SWCR=SWL\n"
        "SWCR   6* MULTA  SWL   1.0    0.0  / SWCR=SWL+0.0 (+0.3)\n"
        "SWCR   6* MAXLIM SWCR  0.7         / max(SWCR)=0.7\n"
        "SGU    6* MULTA  SWL   -1.0   1.0  / SGU=1-SWL\n"
        "/\n";

    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);

    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( parser.parseString( deck , parseContext, errors ) , std::invalid_argument);

    parseContext.update(ParseContext::PARSE_RANDOM_SLASH , InputError::IGNORE );
    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::IGNORE );
    parser.parseString( deck , parseContext, errors );
    BOOST_CHECK_NO_THROW( parser.parseString( deck , parseContext, errors ) );

    parser.addKeyword<ParserKeywords::OPERATE>();
    parser.parseString( deck , parseContext, errors );
    parseContext.update(ParseContext::PARSE_RANDOM_SLASH , InputError::THROW_EXCEPTION );
    parseContext.update(ParseContext::PARSE_UNKNOWN_KEYWORD , InputError::THROW_EXCEPTION );
    BOOST_CHECK_NO_THROW( parser.parseString( deck , parseContext, errors ) );
}




BOOST_AUTO_TEST_CASE( CheckMissingSizeKeyword) {
    const char * deck =
        "SOLUTION\n"
        "EQUIL\n"
        "  10 10 10 10 / \n"
        "\n";

    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);

    parser.addKeyword<ParserKeywords::EQUIL>();
    parser.addKeyword<ParserKeywords::EQLDIMS>();
    parser.addKeyword<ParserKeywords::SOLUTION>();

    parseContext.update( ParseContext::PARSE_MISSING_DIMS_KEYWORD , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( parser.parseString( deck , parseContext, errors ) , std::invalid_argument);

    parseContext.update( ParseContext::PARSE_MISSING_DIMS_KEYWORD , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( parser.parseString( deck , parseContext, errors ) );
}


BOOST_AUTO_TEST_CASE( CheckUnsupportedInSCHEDULE ) {
    const char * deckStringUnSupported =
        "START\n"
        " 10 'JAN' 2000 /\n"
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 / \n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "SCHEDULE\n"
        "MULTZ\n"
        "   1000*0.10 /\n"
        "\n";

    const char * deckStringSupported =
        "START\n"
        " 10 'JAN' 2000 /\n"
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 / \n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "SCHEDULE\n"
        "MULTFLT\n"
        "   'F1' 0.10 /\n"
        "/\n"
        "\n";

    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(true);

    auto deckSupported = parser.parseString( deckStringSupported , parseContext, errors );
    auto deckUnSupported = parser.parseString( deckStringUnSupported , parseContext, errors );
    EclipseGrid grid( deckSupported );
    TableManager table ( deckSupported );
    FieldPropsManager fp(deckSupported, Phases{true, true, true}, grid, table);
    Runspec runspec(deckSupported);
    auto python = std::make_shared<Python>();

    parseContext.update( ParseContext::UNSUPPORTED_SCHEDULE_GEO_MODIFIER , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( Schedule( deckSupported  , grid , fp, runspec, parseContext, errors, python  ));
    BOOST_CHECK_NO_THROW( Schedule( deckUnSupported, grid , fp, runspec, parseContext, errors, python  ));

    parseContext.update( ParseContext::UNSUPPORTED_SCHEDULE_GEO_MODIFIER , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( Schedule( deckUnSupported , grid , fp, runspec , parseContext , errors, python), std::invalid_argument );
    BOOST_CHECK_NO_THROW( Schedule( deckSupported , grid , fp, runspec , parseContext, errors, python));
}



BOOST_AUTO_TEST_CASE(TestRandomSlash) {
    const char * deck1 =
        "SCHEDULE\n"
        "TSTEP\n"
        "  10 10 10 /\n"
        "/\n";

    const char * deck2 =
        "SCHEDULE\n"
        "TSTEP\n"
        "  10 10 10 /\n"
        "   /\n";


    ErrorGuard errors;
    ParseContext parseContext;
    Parser parser(false);


    parser.addKeyword<ParserKeywords::TSTEP>();
    parser.addKeyword<ParserKeywords::SCHEDULE>();

    parseContext.update(ParseContext::PARSE_RANDOM_SLASH , InputError::THROW_EXCEPTION);
    parseContext.update(ParseContext::PARSE_RANDOM_TEXT , InputError::IGNORE);
    BOOST_CHECK_THROW( parser.parseString( deck1 , parseContext, errors ) , std::invalid_argument);
    BOOST_CHECK_THROW( parser.parseString( deck2 , parseContext, errors ) , std::invalid_argument);


    parseContext.update(ParseContext::PARSE_RANDOM_SLASH , InputError::IGNORE);
    parseContext.update(ParseContext::PARSE_RANDOM_TEXT , InputError::THROW_EXCEPTION);
    BOOST_CHECK_NO_THROW( parser.parseString( deck1 , parseContext, errors ) );
    BOOST_CHECK_NO_THROW( parser.parseString( deck2 , parseContext, errors ) );
}



BOOST_AUTO_TEST_CASE(TestCOMPORD) {
    const char * deckString =
        "START\n"
        " 10 'JAN' 2000 /\n"
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 / \n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "SCHEDULE\n"
        "COMPORD\n"
        "  '*'  'DEPTH' /\n"
        "/\n";

    ParseContext parseContext;
    ErrorGuard errors;
    Parser parser(true);
    auto deck = parser.parseString( deckString , parseContext, errors );

    EclipseGrid grid( deck );
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec(deck);
    auto python = std::make_shared<Python>();

    parseContext.update( ParseContext::UNSUPPORTED_COMPORD_TYPE , InputError::IGNORE);
    BOOST_CHECK_NO_THROW( Schedule( deck , grid , fp, runspec, parseContext, errors, python ));

    parseContext.update( ParseContext::UNSUPPORTED_COMPORD_TYPE , InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( Schedule( deck,  grid , fp, runspec , parseContext, errors, python), std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(TestInvalidKey) {
    ParseContext parseContext;
    BOOST_CHECK_THROW( parseContext.addKey("KEY*", InputError::THROW_EXCEPTION) , std::invalid_argument );
    BOOST_CHECK_THROW( parseContext.addKey("KEY:", InputError::THROW_EXCEPTION) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(TestNew) {
    ParseContext parseContext;

    BOOST_CHECK_EQUAL( false , parseContext.hasKey("NO"));
    parseContext.addKey("NEW_KEY", InputError::THROW_EXCEPTION);
    BOOST_CHECK_EQUAL( true , parseContext.hasKey("NEW_KEY"));
    BOOST_CHECK_THROW( parseContext.get("NO") , std::invalid_argument );
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY") , InputError::THROW_EXCEPTION );
    parseContext.addKey("KEY2", InputError::THROW_EXCEPTION);
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY") , InputError::THROW_EXCEPTION );

    BOOST_CHECK_THROW( parseContext.updateKey("NO" , InputError::IGNORE) , std::invalid_argument);

    parseContext.updateKey("NEW_KEY" , InputError::WARN);
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY") , InputError::WARN );

    BOOST_CHECK_NO_THROW( parseContext.update("KEY2:NEW_KEY" , InputError::IGNORE));
    BOOST_CHECK_NO_THROW( parseContext.update("UnknownKey" , InputError::IGNORE));
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY") , InputError::IGNORE );
    BOOST_CHECK_EQUAL( parseContext.get("KEY2") , InputError::IGNORE );

    parseContext.addKey("SECRET_KEY", InputError::THROW_EXCEPTION);
    parseContext.addKey("NEW_KEY2", InputError::THROW_EXCEPTION);
    parseContext.addKey("NEW_KEY3", InputError::THROW_EXCEPTION);
    parseContext.update("NEW_KEY*" , InputError::WARN);
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY") , InputError::WARN );
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY2") , InputError::WARN );
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY3") , InputError::WARN );

    parseContext.update( InputError::IGNORE );
    BOOST_CHECK_EQUAL( parseContext.get("NEW_KEY3")   , InputError::IGNORE );
    BOOST_CHECK_EQUAL( parseContext.get("SECRET_KEY") , InputError::IGNORE );


}


BOOST_AUTO_TEST_CASE( test_constructor_with_values) {
    ParseContext parseContext( {{ParseContext::PARSE_RANDOM_SLASH , InputError::IGNORE},
                {"UNSUPPORTED_*" , InputError::WARN},
                    {"UNKNWON-IGNORED" , InputError::WARN}});

    BOOST_CHECK_EQUAL( parseContext.get(ParseContext::PARSE_RANDOM_SLASH) , InputError::IGNORE );
    BOOST_CHECK_EQUAL( parseContext.get(ParseContext::PARSE_RANDOM_TEXT) , InputError::THROW_EXCEPTION );
    BOOST_CHECK_EQUAL( parseContext.get(ParseContext::UNSUPPORTED_INITIAL_THPRES) , InputError::WARN );
    BOOST_CHECK_EQUAL( parseContext.get(ParseContext::UNSUPPORTED_COMPORD_TYPE) , InputError::WARN );
}



BOOST_AUTO_TEST_CASE( test_too_much_data ) {
    const char * deckString =
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 10 /n"
        "\n";

    ParseContext parseContext;
    Parser parser;
    ErrorGuard errors;


    parseContext.update(ParseContext::PARSE_EXTRA_DATA , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( parser.parseString( deckString , parseContext, errors ) , std::invalid_argument);

    parseContext.update(ParseContext::PARSE_EXTRA_DATA , InputError::IGNORE );
    auto deck = parser.parseString( deckString , parseContext, errors );
}


BOOST_AUTO_TEST_CASE(test_1arg_constructor) {
    setenv("OPM_ERRORS_IGNORE", "PARSE_RANDOM_SLASH", 1);
    {
        ParseContext ctx(InputError::WARN);
        BOOST_CHECK_EQUAL(ctx.get(ParseContext::UNSUPPORTED_COMPORD_TYPE), InputError::WARN);
        BOOST_CHECK_EQUAL(ctx.get(ParseContext::PARSE_RANDOM_SLASH), InputError::IGNORE);
    }
}

BOOST_AUTO_TEST_CASE( test_invalid_wtemplate_config ) {
    const std::string defDeckString = R"(
    START  -- 0
     10 'JAN' 2000 /
    RUNSPEC
    DIMENS
      10 10 10 /
    GRID
    DX
    1000*0.25 /
    DY
    1000*0.25 /
    DZ
    1000*0.25 /
    TOPS
    100*0.25 /
    SCHEDULE
    DATES             -- 1
     10  OKT 2008 /
    /
    WELSPECS
        'PROD' 'G1'  10 10 100 'OIL' /
        'INJ'  'G1'   3  3 100 'WATER' /
    /
    )";

    ParseContext parseContext;
    Parser parser;
    ErrorGuard errors;

    parseContext.update(ParseContext::SCHEDULE_INVALID_NAME , InputError::THROW_EXCEPTION );

    std::vector < std::string > testSamples;
    std::string testSample;

    // Invalid well name in COMPDAT
    testSample = R"(
    COMPDAT
    'SOMETHINGELSE' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WCONPROD
    testSample = R"(
    COMPDAT
    'PROD' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONPROD
    'SOMETHINGELSE' 'OPEN' 'ORAT' 20000 4* 1000 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WCONINJE
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJE
    'SOMETHINGELSE' 'WATER' 'OPEN' 'RATE' 20000 4* 1000 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WCONINJH
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJH
    'SOMETHINGELSE' 'WAT' 'OPEN' 20000 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WTEMP
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJE
    'INJ' 'WATER' 'OPEN' 'RATE' 20000 4*  /
    /
    DATES
    15  OKT 2008 /
    /
    WTEMP
    'SOMETHINGELSE' 40.0 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WINJTEMP
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJE
    'INJ' 'WATER' 'OPEN' 'RATE' 20000 4*  /
    /
    DATES
    15  OKT 2008 /
    /
    WINJTEMP
    'SOMETHINGELSE' 1* 40.0 1*  /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WSOLVENT
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJE
    'INJ' 'WATER' 'OPEN' 'RATE' 20000 4*  /
    /
    DATES
    15  OKT 2008 /
    /
    WSOLVENT
    'SOMETHINGELSE' 1.0  /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WPOLYMER
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJE
    'INJ' 'WATER' 'OPEN' 'RATE' 20000 4*  /
    /
    DATES
    15  OKT 2008 /
    /
    WPOLYMER
    'SOMETHINGELSE' 1.0  0.0 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WFOAM
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJE
    'INJ' 'WATER' 'OPEN' 'RATE' 20000 4*  /
    /
    DATES
    15  OKT 2008 /
    /
    WFOAM
    'SOMETHINGELSE' 0.02 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WELOPEN
    testSample = R"(
    COMPDAT
    'INJ' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONINJE
    'INJ' 'WATER' 'OPEN' 'RATE' 20000 4*  /
    /
    DATES
    15  OKT 2008 /
    /
    WELOPEN
    'SOMETHINGELSE' /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WELTARG
    testSample = R"(
    COMPDAT
    'PROD' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONHIST
    'PROD' 'OPEN' 'ORAT' 20000 4* 1000 /
    /
    WELTARG
    'SOMETHINGELSE' 'ORAT' 15000 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WECON
    testSample = R"(
    COMPDAT
    'PROD' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONPROD
    'PROD' 'OPEN' 'ORAT' 20000 4* 1000 /
    /
    WECON
    'SOMETHINGELSE' 15000 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid well name in WEFAC
    testSample = R"(
    COMPDAT
    'PROD' 10 10 3 3 'OPEN' 1* 1* 0.5 /
    /
    WCONPROD
    'PROD' 'OPEN' 'ORAT' 20000 4* 1000 /
    /
    WEFAC
    'SOMETHINGELSE' 0.5 /
    /
    )";

    testSamples.push_back(testSample);
    // Invalid group name in GCONPROD
    testSample = R"(
    GCONPROD
    'SOMETHINGELSE' 'ORAT' 20000 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid group name in GEFAC
    testSample = R"(
    GEFAC
    'SOMETHINGELSE' 0.5 /
    /
    )";
    testSamples.push_back(testSample);

    // Invalid group name in GCONINJE
    testSample = R"(
    GCONINJE
    'SOMETHINGELSE' 'WAT' 'RATE' 20000 /
    /
    )";
    testSamples.push_back(testSample);

    std::string deckinput;
    for (std::string sample : testSamples) {

        deckinput = defDeckString + sample;
        auto deckUnSupported = parser.parseString( deckinput , parseContext, errors );

        auto python = std::make_shared<Python>();
        EclipseGrid grid( deckUnSupported );
        TableManager table ( deckUnSupported );
        FieldPropsManager fp( deckUnSupported, Phases{true, true, true}, grid, table);
        Runspec runspec( deckUnSupported);

        BOOST_CHECK_THROW( Schedule( deckUnSupported , grid , fp, runspec , parseContext, errors, python), std::invalid_argument );
    }
}


/*
  If there are errors the ErrorGuard destructor will print error messages and
  call std::terminate(); if you do not accept the std::terminate in the
  destructor you should call the clear() method first.
*/


BOOST_AUTO_TEST_CASE(Test_ERRORGUARD) {
    ErrorGuard eg;

    BOOST_CHECK(!eg);

    eg.addWarning(ParseContext::SUMMARY_UNKNOWN_WELL, "Unknown well");
    BOOST_CHECK(!eg);

    eg.addError(ParseContext::SUMMARY_UNKNOWN_GROUP, "Unknwon Group");
    BOOST_CHECK(eg);
    eg.clear();
    BOOST_CHECK(!eg);
}





BOOST_AUTO_TEST_CASE(LONG_KEYWORDS) {
    const std::string deck_string = R"(
RPTRUNSPEC
)";
    Parser parser;
    ParseContext context;
    ErrorGuard error;

    context.update(ParseContext::PARSE_LONG_KEYWORD, InputError::IGNORE);
    auto deck = parser.parseString(deck_string, context, error);
    BOOST_CHECK( deck.hasKeyword("RPTRUNSP") );

    context.update(ParseContext::PARSE_LONG_KEYWORD, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( parser.parseString(deck_string, context, error), std::invalid_argument);
}
