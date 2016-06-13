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

#define BOOST_TEST_MODULE SummaryConfigTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

using namespace Opm;

static DeckPtr createDeck( const std::string& summary ) {
    Opm::Parser parser;
    std::string input = 
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "RUNSPEC\n"
            "\n"
            "DIMENS\n"
            " 10 10 10 /\n"
            "GRID\n"
            "DXV \n 10*400 /\n"
            "DYV \n 10*400 /\n"
            "DZV \n 10*400 /\n"
            "TOPS \n 100*2202 / \n"
            "REGIONS\n"
            "FIPNUM\n"
            "200*1 300*2 500*3 /\n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     \'W_1\'        \'OP\'   1   1  3.33       \'OIL\'  7* /   \n"
            "     \'WX2\'        \'OP\'   2   2  3.33       \'OIL\'  7* /   \n"
            "     \'W_3\'        \'OP\'   2   5  3.92       \'OIL\'  7* /   \n"
            "     'PRODUCER' 'G'   5  5 2000 'GAS'     /\n"
            "/\n"
            "COMPDAT\n"
            "'PRODUCER'   5  5  1  1 'OPEN' 1* -1  0.5  / \n"
            "'W_1'   3    7    1    3      'OPEN'  1*     32.948      0.311   3047.839  2*         'X'     22.100 / \n"
            "'W_1'   3    7    2    2      'OPEN'  1*          *      0.311   4332.346  2*         'X'     22.123 / \n"
            "/\n"
            "SUMMARY\n"
            + summary;

    return parser.parseString(input, ParseContext());
}

static std::vector< std::string > sorted_names( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary )
        ret.push_back( x.wgname() );

    std::sort( ret.begin(), ret.end() );
    return ret;
}

static std::vector< std::string > sorted_keywords( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary )
        ret.push_back( x.keyword() );

    std::sort( ret.begin(), ret.end() );
    return ret;
}

static SummaryConfig createSummary( std::string input ) {
    auto deck = createDeck( input );
    EclipseState state( deck, ParseContext() );
    return SummaryConfig( *deck, state );
}

BOOST_AUTO_TEST_CASE(wells_all) {
    const auto input = "WWCT\n/\n";
    const auto summary = createSummary( input );

    const auto wells = { "PRODUCER", "WX2", "W_1", "W_3" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(wells_select) {
    const auto input = "WWCT\n'W_1' 'WX2' /\n";
    const auto summary = createSummary( input );
    const auto wells = { "WX2", "W_1" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(fields) {
    const auto input = "FOPT\n";
    const auto summary = createSummary( input );
    const auto keywords = { "FOPT" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(blocks) {
    const auto input = "BPR\n"
                       "3 3 6 /\n"
                       "4 3 6 /\n"
                       "/";
    const auto summary = createSummary( input );
    const auto keywords = { "BPR", "BPR" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(regions) {
    const auto input = "ROIP\n"
                       "1 2 3 /\n"
                       "RWIP\n"
                       "/\n"
                       "RGIP\n"
                       "1 2 /\n";

    const auto summary = createSummary( input );
    const auto keywords = { "RGIP", "RGIP",
                    "ROIP", "ROIP", "ROIP",
                    "RWIP", "RWIP", "RWIP" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(completions) {
    const auto input = "CWIR\n"
                       "'PRODUCER'  /\n"
                       "'WX2' 1 1 1 /\n"
                       "'WX2' 2 2 2 /\n"
                       "/\n"
                       "CWIT\n"
                       "'W_1' /\n"
                       "/\n";

    const auto summary = createSummary( input );
    const auto keywords = { "CWIR", "CWIR", "CWIR",
                            "CWIT", "CWIT", "CWIT" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}
