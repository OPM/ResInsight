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
            "'W_1'   3    7    2    2      'OPEN'  1*          *      0.311   4332.346  2*         'X'     22.123 / \n"
            "'W_1'   2    2    1    1      /\n"
            "'WX2'   2    2    1    1      /\n"
            "/\n"
            "SUMMARY\n"
            + summary;

    return parser.parseString(input, ParseContext());
}

static std::vector< std::string > sorted_names( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary ) {
        auto wgname = x.wgname();
        if(wgname)
            ret.push_back( x.wgname() );
    }

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

static std::vector< std::string > sorted_key_names( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary ) {
        ret.push_back( x.key1() );
    }

    std::sort( ret.begin(), ret.end() );
    return ret;
}

static SummaryConfig createSummary( std::string input , const ParseContext& parseContext = ParseContext()) {
    auto deck = createDeck( input );
    EclipseState state( *deck, parseContext );
    return state.getEclipseConfig().getSummaryConfig();
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

BOOST_AUTO_TEST_CASE(wells_pattern) {
    const auto input = "WWCT\n'W*' /\n";
    const auto summary = createSummary( input );
    const auto wells = { "WX2", "W_1", "W_3" };
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
    const auto input = "CWIR\n" // all specified
                       "'PRODUCER'  /\n"
                       "'WX2' 1 1 1 /\n"
                       "'WX2' 2 2 1 /\n"
                       "/\n"
                       "CWIT\n" // block defaulted
                       "'W_1' /\n"
                       "/\n"
                       "CGIT\n" // well defaulted
                       "* 2 2 1 /\n"
                       "/\n"
                       "CGIR\n" // all defaulted
                       " '*' /\n"
                       "/\n";

    const auto summary = createSummary( input );
    const auto keywords = { "CGIR", "CGIR", "CGIR", "CGIR",
                            "CGIT", "CGIT",
                            "CWIR", "CWIR",
                            "CWIT", "CWIT" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );

}

BOOST_AUTO_TEST_CASE( merge ) {
    const auto input1 = "WWCT\n/\n";
    auto summary1 = createSummary( input1 );

    const auto keywords = { "FOPT", "WWCT", "WWCT", "WWCT", "WWCT" };
    const auto wells = { "PRODUCER", "WX2", "W_1", "W_3" };

    const auto input2 = "FOPT\n";
    const auto summary2 = createSummary( input2 );

    summary1.merge( summary2 );
    const auto kw_names = sorted_keywords( summary1 );
    const auto well_names = sorted_names( summary1 );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            kw_names.begin(), kw_names.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            well_names.begin(), well_names.end() );
}

BOOST_AUTO_TEST_CASE( merge_move ) {
    const auto input = "WWCT\n/\n";
    auto summary = createSummary( input );

    const auto keywords = { "FOPT", "WWCT", "WWCT", "WWCT", "WWCT" };
    const auto wells = { "PRODUCER", "WX2", "W_1", "W_3" };

    summary.merge( createSummary( "FOPT\n" ) );

    const auto kw_names = sorted_keywords( summary );
    const auto well_names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            kw_names.begin(), kw_names.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            well_names.begin(), well_names.end() );
}

static const auto ALL_keywords = {
    "FAQR",  "FAQRG", "FAQT", "FAQTG", "FGIP", "FGIPG", "FGIPL",
    "FGIR",  "FGIT",  "FGOR", "FGPR",  "FGPT", "FOIP",  "FOIPG",
    "FOIPL", "FOIR",  "FOIT", "FOPR",  "FOPT", "FPR",   "FVIR",
    "FVIT",  "FVPR",  "FVPT", "FWCT",  "FWGR", "FWIP",  "FWIR",
    "FWIT",  "FWPR",  "FWPT",
    "GGIR",  "GGIT",  "GGOR", "GGPR",  "GGPT", "GOIR",  "GOIT",
    "GOPR",  "GOPT",  "GVIR", "GVIT",  "GVPR", "GVPT",  "GWCT",
    "GWGR",  "GWIR",  "GWIT", "GWPR",  "GWPT",
    "WBHP",  "WGIR",  "WGIT", "WGOR",  "WGPR", "WGPT",  "WOIR",
    "WOIT",  "WOPR",  "WOPT", "WPI",   "WTHP", "WVIR",  "WVIT",
    "WVPR",  "WVPT",  "WWCT", "WWGR",  "WWIR", "WWIT",  "WWPR",
    "WWPT",
    // ALL will not expand to these keywords yet
    "AAQR",  "AAQRG", "AAQT", "AAQTG"
};


BOOST_AUTO_TEST_CASE(summary_ALL) {

    const auto input = "ALL\n";

    const auto summary = createSummary( input );
    const auto key_names = sorted_key_names( summary );

    std::vector<std::string> all;

    for( std::string keyword: ALL_keywords ) {
        if(keyword[0]=='F') {
            all.push_back(keyword);
        }
        else if (keyword[0]=='G') {
            auto kn = keyword + ":";
            all.push_back(kn + "G");
            all.push_back(kn + "OP");
            all.push_back(kn + "FIELD");
        }
        else if (keyword[0]=='W') {
            auto kn = keyword + ":";
            all.push_back(kn + "W_1");
            all.push_back(kn + "WX2");
            all.push_back(kn + "W_3");
            all.push_back(kn + "PRODUCER");
        }
    }

    std::sort(all.begin(), all.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        all.begin(), all.end(),
        key_names.begin(), key_names.end());

    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "FOPT"));
    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "GGIT"));
    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "WWCT"));

    BOOST_CHECK_EQUAL( false , summary.hasKeyword("NO-NOT-THIS"));
}



BOOST_AUTO_TEST_CASE(INVALID_WELL1) {
    ParseContext parseContext;
    const auto input = "CWIR\n"
                       "NEW-WELL /\n"
        "/\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , std::invalid_argument);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}


BOOST_AUTO_TEST_CASE(INVALID_WELL2) {
    ParseContext parseContext;
    const auto input = "WWCT\n"
        " NEW-WELL /\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , std::invalid_argument);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}


BOOST_AUTO_TEST_CASE(INVALID_GROUP) {
    ParseContext parseContext;
    const auto input = "GWCT\n"
        " NEW-GR /\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_GROUP , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , std::invalid_argument);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_GROUP , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}

BOOST_AUTO_TEST_CASE( REMOVE_DUPLICATED_ENTRIES ) {
    ParseContext parseContext;
    const auto input = "WGPR \n/\n"
                       "WGPR \n/\n"
                       "ALL\n";

    const auto summary = createSummary( input );
    const auto keys = sorted_key_names( summary );
    auto uniq_keys = keys;
    uniq_keys.erase( std::unique( uniq_keys.begin(),
                                  uniq_keys.end(),
                                  std::equal_to< std::string >() ),
                     uniq_keys.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keys.begin(), keys.end(),
            uniq_keys.begin(), uniq_keys.end() );
}
