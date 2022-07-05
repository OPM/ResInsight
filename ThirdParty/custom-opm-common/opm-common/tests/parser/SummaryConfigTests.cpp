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
#include <fmt/format.h>
#include <opm/common/utility/OpmInputError.hpp>
#include <opm/io/eclipse/SummaryNode.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <algorithm>

using namespace Opm;

static Deck createDeck_no_wells( const std::string& summary ) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "RUNSPEC\n"
            "\n"
            "DIMENS\n"
            " 10 10 10 /\n"
            "REGDIMS\n"
            "  3/\n"
            "GRID\n"
            "DXV \n 10*400 /\n"
            "DYV \n 10*400 /\n"
            "DZV \n 10*400 /\n"
            "TOPS \n 100*2202 / \n"
            "PERMX\n"
            "  1000*0.25 /\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "PORO \n"
            "   1000*0.15 /\n"
            "REGIONS\n"
            "FIPNUM\n"
            "200*1 300*2 500*3 /\n"
            "SCHEDULE\n"
            "SUMMARY\n"
            + summary;

    return parser.parseString(input);
}


static Deck createDeck( const std::string& summary ) {
    Opm::Parser parser;
    std::string input = R"(
START             -- 0
10 MAI 2007 /
RUNSPEC

DIMENS
 10 10 10 /
REGDIMS
  3 3 /
AQUDIMS
4 4 1* 1* 3 200 1* 1* /
GRID
DXV
   10*400 /
DYV
   10*400 /
DZV
   10*400 /
TOPS
   100*2202 /
PERMX
  1000*0.25 /
COPY
  PERMX PERMY /
  PERMX PERMZ /
/
PORO
   1000*0.15 /
AQUNUM
  4       1 1 1      15000  5000  0.3  30  2700  / aq cell
  5       2 1 1     150000  9000  0.3  30  2700  / aq cell
  6       3 1 1     150000  9000  0.3  30  2700  / aq cell
  7       4 1 1     150000  9000  0.3  30  2700  / aq cell
/
AQUCON
-- #    I1 I2  J1 J2   K1  K2    Face
   4    1  1   16 18   19  20   'I-'    / connecting cells
   5    2  2   16 18   19  20   'I-'    / connecting cells
   6    3  3   16 18   19  20   'I-'    / connecting cells
   7    4  4   16 18   19  20   'I-'    / connecting cells
/
REGIONS
FIPNUM
200*1 300*2 500*3 /
FIPREG
200*10 300*20 500*30 /
FIPXYZ
200*2 300*3 500*1 /
SOLUTION
AQUCT
1    2040     1*    1000   .3    3.0e-5     1330     10     360.0   1   1* /
2    2040     1*    1000   .3    3.0e-5     1330     10     360.0   1   1* /
3    2040     1*    1000   .3    3.0e-5     1330     10     360.0   1   1* /
/
AQUANCON
1     1   10     10    2    10  10   'I-'      0.88      1  /
2     9   10     10    10    10  10   'I+'      0.88      1  /
3     9   9      8    10    9   8   'I+'      0.88      1  /
/
SCHEDULE
WELSPECS
     'W_1'        'OP'   1   1  3.33       'OIL'  7* /
     'WX2'        'OP'   2   2  3.33       'OIL'  7* /
     'W_3'        'OP'   2   5  3.92       'OIL'  7* /
     'PRODUCER' 'G'   5  5 2000 'GAS'     /
/
COMPDAT
'PRODUCER'   5  5  1  1 'OPEN' 1* -1  0.5  /
'W_1'   3    7    2    2      'OPEN'  1*          *      0.311   4332.346  2*         'X'     22.123 /
'W_1'   2    2    1    1      /
'W_1'   2    2    2    2      /
'WX2'   2    2    1    1      /
/

COMPLUMP
W_1 3 7 2 2 2 /
W_1 2 2 2 2 2 /
W_1 2 2 1 1 4 /
/

SUMMARY
)" + summary;

    return parser.parseString(input);
}

static std::vector< std::string > sorted_names( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary ) {
        auto wgname = x.namedEntity();
        if(wgname.size())
            ret.push_back( wgname );
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
        ret.push_back( x.uniqueNodeKey() );
    }

    std::sort( ret.begin(), ret.end() );
    return ret;
}

static SummaryConfig createSummary(const std::string& input , const ParseContext& parseContext = ParseContext()) {
    ErrorGuard errors;
    auto deck = createDeck( input );
    auto python = std::make_shared<Python>();
    EclipseState state( deck );
    Schedule schedule(deck, state, parseContext, errors, python);
    return SummaryConfig(deck, schedule, state.fieldProps(), state.aquifer(), parseContext, errors);
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

BOOST_AUTO_TEST_CASE(WSTATE) {
    const auto input = "WSTAT\n/\n";
    const auto summary = createSummary( input );

    const auto wells = { "PRODUCER", "WX2", "W_1", "W_3" };
    for (const auto& well : wells)
        BOOST_CHECK(summary.hasSummaryKey(fmt::format("WSTAT:{}", well)));
}


BOOST_AUTO_TEST_CASE(EMPTY) {
    auto deck = createDeck_no_wells( "" );
    auto python = std::make_shared<Python>();
    EclipseState state( deck );
    Schedule schedule(deck, state, python);
    SummaryConfig conf(deck, schedule, state.fieldProps(), state.aquifer());
    BOOST_CHECK_EQUAL( conf.size(), 0U );
}

BOOST_AUTO_TEST_CASE(wells_missingI) {
    auto python = std::make_shared<Python>();
    ParseContext parseContext;
    ErrorGuard errors;
    const auto input = "WWCT\n/\n";
    auto deck = createDeck_no_wells( input );
    parseContext.update(ParseContext::SUMMARY_UNKNOWN_WELL, InputError::THROW_EXCEPTION);
    EclipseState state( deck );
    Schedule schedule(deck, state, parseContext, errors, python );
    BOOST_CHECK_NO_THROW(SummaryConfig(deck, schedule, state.fieldProps(), state.aquifer(), parseContext, errors));
}


BOOST_AUTO_TEST_CASE(wells_select) {
    const auto input = "WWCT\n'W_1' 'WX2' /\n";
    const auto summary = createSummary( input );
    const auto wells = { "WX2", "W_1" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            names.begin(), names.end() );

    BOOST_CHECK_EQUAL( summary.size(), 2U );
}

BOOST_AUTO_TEST_CASE(groups_all) {
    const auto summary = createSummary( "GWPR \n /\n" );
    const auto groups = { "G", "OP" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS( groups.begin(), groups.end(),
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

BOOST_AUTO_TEST_CASE(tracer) {
    const auto input = "FTIRSEA\n WTICSEA\n'W_1'/\n WTPRSEA\n'W_3' 'WX2'/\n";
    const auto summary = createSummary( input );
    const auto keywords = { "FTIRSEA", "WTICSEA", "WTPRSEA", "WTPRSEA" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(field_oil_efficiency) {
    const auto input = "FOE\n";
    const auto summary = createSummary( input );

    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "FOE"));
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

BOOST_AUTO_TEST_CASE(aquifer) {
    const auto input = R"(
ALQR      -- This is completely ignored
   'ALQ1' 'ALQ2' /
AAQR
 1 2 /
AAQT
 1 /
AAQP
 1  2 3/
)";
    const auto summary = createSummary( input );
    const auto keywords = { "AAQP", "AAQP", "AAQP", "AAQR", "AAQR", "AAQT" };
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

BOOST_AUTO_TEST_CASE(region2region) {
    const auto input = std::string { R"(ROFT
1 2/
3 4/
/
ROFT+
1 2/
3 4/
/
ROFT-
1 2/
3 4/
/
ROFR
1 2/
3 4/
/
ROFR+
1 2/
3 4/
/
ROFR-
1 2/
3 4/
/
ROFTL
1 2/
3 4/
/
ROFTG
1 2/
3 4/
1 3/
/
RGFT
5 6/
7 8/
/
RGFT+
5 6/
7 8/
/
RGFT-
5 6/
7 8/
/
RGFR
5 6/
7 8/
/
RGFR+
5 6/
7 8/
/
RGFR-
5 6/
7 8/
/
RGFTL
1 2 /
1 3 /
/
RGFTG
1 2 /
1 3 /
/
RWFT
2 3 /
/
RWFT+
2 3 /
/
RWFT-
2 3 /
/
RWFR
2 3 /
/
RWFR+
2 3 /
/
RWFR-
2 3 /
/
RWIP
/
)" };

    ParseContext parseContext;

    const auto summary = createSummary(input, parseContext);

    {
        const auto expect_kw = std::vector<std::string> {
            "ROFT", "ROFT+", "ROFT-", "ROFR", "ROFR+", "ROFR-", "ROFTL", "ROFTG",
            "RGFT", "RGFT+", "RGFT-", "RGFR", "RGFR+", "RGFR-", "RGFTL", "RGFTG",
            "RGFT", "RGFT+", "RGFT-", "RGFR", "RGFR+", "RGFR-",
        };

        for (const auto& kw : expect_kw) {
            BOOST_CHECK_MESSAGE(summary.hasKeyword(kw),
                                "SummaryConfig MUST have keyword '" << kw << "'");
        }
    }

    {
        const auto kw = summary.keywords("ROFT");
        BOOST_CHECK_EQUAL(kw.size(), 2);

        BOOST_CHECK_MESSAGE(kw[1].namedEntity().empty(),
                            "ROFT vector must NOT have an associated named entity");

        BOOST_CHECK_MESSAGE(kw[0].type() == SummaryConfigNode::Type::Total,
                            "ROFT must be a Cumulative Total");

        BOOST_CHECK_MESSAGE(kw[1].category() == SummaryConfigNode::Category::Region,
                            "ROFT must be a Region vector");

        const auto expect_number = std::vector<int> {
            393'217, // 1 2
            458'755, // 3 4
        };

        const auto n1 = kw[0].number();
        const auto n2 = kw[1].number();

        BOOST_CHECK_MESSAGE(((n1 == expect_number[0]) && (n2 == expect_number[1])) ||
                            ((n2 == expect_number[0]) && (n1 == expect_number[1])),
                            R"(ROFT 'NUMS' must match expected set)");
    }

    {
        const auto kw = summary.keywords("RGFR-");
        BOOST_CHECK_EQUAL(kw.size(), 2);

        BOOST_CHECK_MESSAGE(kw[1].namedEntity().empty(),
                            "RGFR- vector must NOT have an associated named entity");

        BOOST_CHECK_MESSAGE(kw[0].type() == SummaryConfigNode::Type::Rate,
                            "RGFR- must be a Rate");

        BOOST_CHECK_MESSAGE(kw[1].category() == SummaryConfigNode::Category::Region,
                            "RGFR- must be a Region vector");

        const auto expect_number = std::vector<int> {
            524'293, // 5 6
            589'831, // 7 8
        };

        const auto n1 = kw[0].number();
        const auto n2 = kw[1].number();

        BOOST_CHECK_MESSAGE(((n1 == expect_number[0]) && (n2 == expect_number[1])) ||
                            ((n2 == expect_number[0]) && (n1 == expect_number[1])),
                            R"(RGFR- 'NUMS' must match expected set)");
    }

    {
        const auto kw = summary.keywords("ROFTG");
        BOOST_CHECK_EQUAL(kw.size(), 3);

        BOOST_CHECK_MESSAGE(kw[1].namedEntity().empty(),
                            "ROFTG vector must NOT have an associated named entity");

        BOOST_CHECK_MESSAGE(kw[0].type() == SummaryConfigNode::Type::Total,
                            "ROFTG must be a Cumulative Total");

        BOOST_CHECK_MESSAGE(kw[1].category() == SummaryConfigNode::Category::Region,
                            "ROFTG must be a Region vector");

        const auto expect_number = std::vector<int> {
            393'217, // 1 2
            458'755, // 3 4
            425'985, // 1 3
        };

        const auto actual = std::vector<int> {
            kw[0].number(),
            kw[1].number(),
            kw[2].number(),
        };

        BOOST_CHECK_MESSAGE(std::is_permutation(actual       .begin(), actual       .end(),
                                                expect_number.begin(), expect_number.end()),
                            R"(ROFTG 'NUMS' must match expected set)");
    }
}

BOOST_AUTO_TEST_CASE(region2region_unsupported) {
    const auto input = std::string { R"(REFR-
2 3 /
/
RKFT
2 3 /
/
)" };

  ParseContext parseContext;

  parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::THROW_EXCEPTION);
  BOOST_CHECK_THROW( createSummary(input, parseContext), OpmInputError);
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
                       "/\n"
                       "CPRL\n" // all defaulted
                       " '*' /\n"
                       "/\n";

    const auto summary = createSummary( input );
    const auto keywords = { "CGIR", "CGIR", "CGIR", "CGIR", "CGIR",
                            "CGIT", "CGIT",
                            "CPRL", "CPRL", "CPRL", "CPRL", "CPRL",
                            "CWIR", "CWIR",
                            "CWIT", "CWIT", "CWIT" };
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
        "WWPT",  "WGLIR",
        // ALL will not expand to these keywords yet
        "AAQR",  "AAQRG", "AAQT", "AAQTG"
};


BOOST_AUTO_TEST_CASE(summary_ALL) {

    const auto input = "ALL\n";

    const auto summary = createSummary( input );
    const auto key_names = sorted_key_names( summary );

    std::vector<std::string> all;

    for( std::string keyword: ALL_keywords ) {
        if(keyword[0]=='A' && keyword !="ALL") {
           all.push_back(keyword + ":1");
           all.push_back(keyword + ":2");
           all.push_back(keyword + ":3");
        }
        if(keyword[0]=='F') {
            all.push_back(keyword);
        }
        else if (keyword[0]=='G') {
            auto kn = keyword + ":";
            all.push_back(kn + "G");
            all.push_back(kn + "OP");
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

    BOOST_CHECK_EQUAL( false, summary.hasKeyword( "WOPP"));
    BOOST_CHECK_EQUAL( false, summary.hasKeyword( "FOPP"));

    BOOST_CHECK_EQUAL( false , summary.hasKeyword("NO-NOT-THIS"));
}



BOOST_AUTO_TEST_CASE(INVALID_WELL1) {
    ParseContext parseContext;
    const auto input = "CWIR\n"
                       "NEW-WELL /\n"
        "/\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , OpmInputError);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}


BOOST_AUTO_TEST_CASE(INVALID_WELL2) {
    ParseContext parseContext;
    const auto input = "WWCT\n"
        " NEW-WELL /\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , OpmInputError);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}

BOOST_AUTO_TEST_CASE(UNDEFINED_UDQ_WELL) {
    ParseContext parseContext;
    const auto input = "WUWCT\n"
        "/\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNDEFINED_UDQ, InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , OpmInputError);

    parseContext.updateKey( ParseContext::SUMMARY_UNDEFINED_UDQ, InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}




BOOST_AUTO_TEST_CASE(INVALID_GROUP) {
    ParseContext parseContext;
    const auto input = "GWCT\n"
        " NEW-GR /\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_GROUP , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , OpmInputError);

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

BOOST_AUTO_TEST_CASE( ANALYTICAL_AQUIFERS ) {
    {
        const auto faulty_input = std::string {R"(
AAQT
-- Neither of these are analytic aquifers => input error
    4 5 6 7 /
)" };

        BOOST_CHECK_THROW(const auto summary = createSummary(faulty_input),
                          OpmInputError);
    }

    {
        const auto faulty_input = std::string {R"(
AAQP
-- Aquifer ID out of range => input error
    1729 /
)" };

        BOOST_CHECK_THROW(const auto summary = createSummary(faulty_input),
                          OpmInputError);
    }

    const std::string input = R"(
AAQR
    1 2 /
AAQP
    2 1 /
AAQT
    /
AAQRG
    /
AAQTG
    /
AAQTD
    /
AAQPD
    /
)";
    const auto summary = createSummary( input );

    const auto keywords = { "AAQP", "AAQP", "AAQPD", "AAQPD", "AAQPD",
                            "AAQR", "AAQR", "AAQRG", "AAQRG", "AAQRG",
                            "AAQT", "AAQT", "AAQT", "AAQTD", "AAQTD", "AAQTD",
                            "AAQTG", "AAQTG", "AAQTG"
                            };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE( NUMERICAL_AQUIFERS )
{
    {
        const auto faulty_input = std::string {R"(
ANQR
-- Neither of these are numeric aquifers => input error
    1 2 3 /
)" };

        BOOST_CHECK_THROW(const auto summary = createSummary(faulty_input),
                          OpmInputError);
    }

    {
        const auto faulty_input = std::string {R"(
ANQP
-- Aquifer ID out of range => input error
    42 /
)" };

        BOOST_CHECK_THROW(const auto summary = createSummary(faulty_input),
                          OpmInputError);
    }

    const std::string input = R"(
ANQR
    5 /
ANQP
    4 7 /
ANQT
    /
)";
    const auto summary = createSummary( input );

    const auto keywords = { "ANQP", "ANQP", "ANQR", "ANQT", "ANQT", "ANQT", "ANQT" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

static const auto GMWSET_keywords = {
    "GMWPT", "GMWPR", "GMWPA", "GMWPU", "GMWPG", "GMWPO", "GMWPS",
    "GMWPV", "GMWPP", "GMWPL", "GMWIT", "GMWIN", "GMWIA", "GMWIU", "GMWIG",
    "GMWIS", "GMWIV", "GMWIP", "GMWDR", "GMWDT", "GMWWO", "GMWWT"
};

BOOST_AUTO_TEST_CASE( summary_GMWSET ) {

    const auto input = "GMWSET\n";
    const auto summary = createSummary( input );
    const auto key_names = sorted_key_names( summary );

    std::vector< std::string > all;

    for( std::string kw : GMWSET_keywords ) {
        all.emplace_back(kw + ":G");
        all.emplace_back(kw + ":OP");
    }

    std::sort( all.begin(), all.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS( all.begin(), all.end(),
                                   key_names.begin(), key_names.end() );

    BOOST_CHECK( summary.hasKeyword( "GMWPS" ) );
    BOOST_CHECK( summary.hasKeyword( "GMWPT" ) );
    BOOST_CHECK( summary.hasKeyword( "GMWPR" ) );

    BOOST_CHECK( !summary.hasKeyword("NO-NOT-THIS") );
}

static const auto FMWSET_keywords = {
    "FMCTF", "FMWPT", "FMWPR", "FMWPA", "FMWPU", "FMWPF", "FMWPO", "FMWPS",
    "FMWPV", "FMWPP", "FMWPL", "FMWIT", "FMWIN", "FMWIA", "FMWIU", "FMWIF",
    "FMWIS", "FMWIV", "FMWIP", "FMWDR", "FMWDT", "FMWWO", "FMWWT"
};

BOOST_AUTO_TEST_CASE( summary_FMWSET ) {

    const auto input = "FMWSET\n";
    const auto summary = createSummary( input );
    const auto key_names = sorted_key_names( summary );

    std::vector< std::string > all( FMWSET_keywords.begin(),
                                    FMWSET_keywords.end() );
    std::sort( all.begin(), all.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS( all.begin(), all.end(),
                                   key_names.begin(), key_names.end() );

    BOOST_CHECK( summary.hasKeyword( "FMWPS" ) );
    BOOST_CHECK( summary.hasKeyword( "FMWPT" ) );
    BOOST_CHECK( summary.hasKeyword( "FMWPR" ) );

    BOOST_CHECK( !summary.hasKeyword("NO-NOT-THIS") );
}

BOOST_AUTO_TEST_CASE(FMWPA) {
    const auto input = "FMWPA\n";
    const auto summary = createSummary( input );
    BOOST_CHECK_EQUAL(1U , summary.size() );
}





BOOST_AUTO_TEST_CASE( summary_require3DField ) {
    {
        const auto input = "WWCT\n/\n";
        const auto summary = createSummary( input );

        BOOST_CHECK( !summary.require3DField( "NO-NOT-THIS"));

        BOOST_CHECK( !summary.require3DField( "PRESSURE"));
        BOOST_CHECK( !summary.require3DField( "OIP"));
        BOOST_CHECK( !summary.require3DField( "GIP"));
        BOOST_CHECK( !summary.require3DField( "WIP"));
        BOOST_CHECK( !summary.require3DField( "OIPL"));
        BOOST_CHECK( !summary.require3DField( "OIPG"));
        BOOST_CHECK( !summary.require3DField( "GIPL"));
        BOOST_CHECK( !summary.require3DField( "GIPG"));
        BOOST_CHECK( !summary.require3DField( "SWAT"));
        BOOST_CHECK( !summary.require3DField( "SGAS"));
    }

    {
        const auto input = "BPR\n"
            "3 3 6 /\n"
            "4 3 6 /\n"
            "/";

        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "PRESSURE"));
    }


    {
        const auto input = "FPR\n";
        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "PRESSURE"));
    }


    {
        const auto input = "BSWAT\n"
            "3 3 6 /\n"
            "4 3 6 /\n"
            "/";

        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "SWAT"));
    }

    {
        const auto input = "BSGAS\n"
            "3 3 6 /\n"  // 523
            "4 3 6 /\n"  // 524
            "/";

        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "SGAS"));
        BOOST_CHECK( summary.hasSummaryKey( "BSGAS:523" ) );
    }


    {
        const auto input = "RPR\n/\n";
        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "PRESSURE"));
        BOOST_CHECK( summary.hasKeyword( "RPR" ) );
        BOOST_CHECK( summary.hasSummaryKey( "RPR:1" ) );
        BOOST_CHECK( summary.hasSummaryKey( "RPR:3" ) );
        BOOST_CHECK( !summary.hasSummaryKey( "RPR:4" ) );
    }


    {
        const auto input = "RPR\n 10 /\n";
        BOOST_CHECK_NO_THROW( createSummary( input ) );
    }



    {
        const auto input = "RGIPL\n/\n";
        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "GIPL"));
    }
}


BOOST_AUTO_TEST_CASE( SUMMARY_MISC) {
    {
        const auto summary = createSummary( "TCPU\n" );
        BOOST_CHECK( summary.hasKeyword( "TCPU" ) );
    }

    {
        const auto summary = createSummary( "PERFORMA\n" );
        BOOST_CHECK( summary.hasKeyword( "ELAPSED" ) );
        BOOST_CHECK( !summary.hasKeyword("PERFORMA"));
    }
}

BOOST_AUTO_TEST_CASE(Summary_Segment)
{
    auto python = std::make_shared<Python>();
    const auto input = std::string { "SOFR_TEST.DATA" };
    const auto deck  = Parser{}.parseFile(input);
    const auto state = EclipseState { deck };

    const auto schedule = Schedule { deck, state, python};
    const auto summary  = SummaryConfig {
        deck, schedule, state.fieldProps(), state.aquifer()
    };

    // SOFR PROD01 segments 1, 10, 21.
    BOOST_CHECK(deck.hasKeyword("SOFR"));
    BOOST_CHECK(summary.hasKeyword("SOFR"));
    BOOST_CHECK(summary.hasSummaryKey("SOFR:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SOFR:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:15"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SOFR:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:26"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:27"));

    BOOST_CHECK(!summary.hasSummaryKey("SOFR:INJE01:1"));

    {
        auto sofr = std::find_if(summary.begin(), summary.end(),
            [](const SummaryConfigNode& node)
        {
            return node.keyword() == "SOFR";
        });

        BOOST_REQUIRE(sofr != summary.end());

        BOOST_CHECK_MESSAGE(sofr->category() == SummaryConfigNode::Category::Segment,
            R"("SOFR" keyword category must be "Segment")"
        );
        BOOST_CHECK_EQUAL(sofr->namedEntity(), "PROD01");
    }

    BOOST_CHECK(deck.hasKeyword("SGFR"));
    BOOST_CHECK(summary.hasKeyword("SGFR"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:26"));
    BOOST_CHECK(!summary.hasSummaryKey("SGFR:PROD01:27"));  // No such segment.

    // SPR PROD01 segment 10 only.
    BOOST_CHECK(deck.hasKeyword("SPR"));
    BOOST_CHECK(summary.hasKeyword("SPR"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPR:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:15"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:20"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPR:INJE01:10"));

    // SWFR for all segments in all MS wells.
    BOOST_CHECK(deck.hasKeyword("SWFR"));
    BOOST_CHECK(summary.hasKeyword("SWFR"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SWFR:INJE01:1"));

    // SPRD for all segments in all MS wells.
    BOOST_CHECK(deck.hasKeyword("SPRD"));
    BOOST_CHECK(summary.hasKeyword("SPRD"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRD:INJE01:1"));

    // SPRDH for all segments of MS well PROD01.
    BOOST_CHECK(deck.hasKeyword("SPRDH"));
    BOOST_CHECK(summary.hasKeyword("SPRDH"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRDH:INJE01:1"));

    // SPRDF for segments 10 and 16 of MS well PROD01.
    BOOST_CHECK(deck.hasKeyword("SPRDF"));
    BOOST_CHECK(summary.hasKeyword("SPRDF"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDF:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDF:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:20"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:INJE01:1"));

    // SPRDA for segments 10 and 16 of all MS wells
    BOOST_CHECK(deck.hasKeyword("SPRDA"));
    BOOST_CHECK(summary.hasKeyword("SPRDA"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDA:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDA:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:20"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:INJE01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:INJE01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:INJE01:16"));
}

BOOST_AUTO_TEST_CASE(Summary_Network) {
    // Example data adapted from opm-tests/model5/1_NETWORK_MODEL5.DATA.
    const auto deck = ::Opm::Parser{}.parseString(R"(RUNSPEC
START
  21 SEP 2020 12:34:56 /

DIMENS
  10 10 3 /

GRID

DXV
  10*100.0
/

DYV
  10*100.0
/

DZV
  5 3 2
/

DEPTHZ
  121*2000.0
/

SUMMARY

GPR
/

SCHEDULE

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
/

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    B1        PLAT-A   5       1* /
    C1        PLAT-A   4       1* /
/

NODEPROP
--  Node_name  Press  autoChoke?  addGasLift?  Group_name
     PLAT-A    21.0   NO          NO           1*  /
     B1        1*     NO          NO           1*  /
     C1        1*     NO          NO           1*  /
/

TSTEP
  10*10 /
END
)");

    ErrorGuard errors;
    const auto parseContext = ParseContext{};
    const auto state = EclipseState (deck);
    const auto schedule = Schedule (deck, state, parseContext, errors, std::make_shared<const Python>());
    const auto smry = SummaryConfig(deck, schedule, state.fieldProps(), state.aquifer(), parseContext, errors);

    BOOST_CHECK_MESSAGE(deck.hasKeyword("GPR"), R"(Deck must have "GPR" keyword)");
    BOOST_CHECK_MESSAGE(smry.hasKeyword("GPR"), R"(SummaryConfig must have "GPR" keyword)");
    BOOST_CHECK_MESSAGE(smry.hasSummaryKey("GPR:PLAT-A"), R"(SummaryConfig must have "GPR:PLAT-A" key)");
    BOOST_CHECK_MESSAGE(smry.hasSummaryKey("GPR:B1"), R"(SummaryConfig must have "GPR:B1" key)");
    BOOST_CHECK_MESSAGE(smry.hasSummaryKey("GPR:C1"), R"(SummaryConfig must have "GPR:C1" key)");

    BOOST_CHECK_MESSAGE(!smry.hasSummaryKey("GPR:PROD"), R"(SummaryConfig must NOT have "GPR:PROD" key)");
    BOOST_CHECK_MESSAGE(!smry.hasSummaryKey("GPR:FIELD"), R"(SummaryConfig must NOT have "GPR:FIELD" key)");
    BOOST_CHECK_MESSAGE(!smry.hasSummaryKey("GPR:M5N"), R"(SummaryConfig must NOT have "GPR:M5N" key)");
    BOOST_CHECK_MESSAGE(!smry.hasSummaryKey("GPR:M5S"), R"(SummaryConfig must NOT have "GPR:M5S" key)");
    BOOST_CHECK_MESSAGE(!smry.hasSummaryKey("GPR:F1"), R"(SummaryConfig must NOT have "GPR:F1" key)");
    BOOST_CHECK_MESSAGE(!smry.hasSummaryKey("GPR:G1"), R"(SummaryConfig must NOT have "GPR:G1" key)");
}

BOOST_AUTO_TEST_CASE(ProcessingInstructions) {
    const std::string deck_string = R"(
RPTONLY
RUNSUM
NARROW
SEPARATE
)";

    const auto& summary_config = createSummary(deck_string);

    BOOST_CHECK(!summary_config.hasKeyword("NARROW"));
    BOOST_CHECK(!summary_config.hasKeyword("RPTONLY"));
    BOOST_CHECK(!summary_config.hasKeyword("RUNSUM"));
    BOOST_CHECK(!summary_config.hasKeyword("SEPARATE"));
    BOOST_CHECK(!summary_config.hasKeyword("SUMMARY"));
}


BOOST_AUTO_TEST_CASE(EnableRSM) {
    std::string deck_string1 = "";
    std::string deck_string2 = R"(
RUNSUM
)";
    const auto& summary_config1 = createSummary(deck_string1);
    const auto& summary_config2 = createSummary(deck_string2);

    BOOST_CHECK(!summary_config1.createRunSummary());
    BOOST_CHECK(!summary_config1.hasKeyword("RUNSUM"));

    BOOST_CHECK( summary_config2.createRunSummary());
    BOOST_CHECK(!summary_config2.hasKeyword("RUNSUM"));
}


BOOST_AUTO_TEST_CASE(FIPREG) {
    std::string deck_string = R"(
-- The FIPREG region has three distinct values, i.e.
-- there will be three different RPR__REG keywords.
RPR__REG
/

ROPT_REG
/

RRPV_REG
/

ROEW_REG
/

RHPV_REG
/

)";
    const auto& summary_config = createSummary(deck_string);
    // The +5 corresponds to five additional COPT summary config keywords which
    // have been automatically added for the ROEW calculation.
    BOOST_CHECK_EQUAL(summary_config.size(), 15 + 5);
    BOOST_CHECK(summary_config.hasKeyword("RPR__REG"));
    BOOST_CHECK(summary_config.hasKeyword("ROPT_REG"));
    BOOST_CHECK(summary_config.hasKeyword("RRPV_REG"));
    BOOST_CHECK(summary_config.hasKeyword("ROEW_REG"));
    BOOST_CHECK(summary_config.hasKeyword("RHPV_REG"));
    BOOST_CHECK(!summary_config.hasKeyword("RPR"));
    BOOST_CHECK(!summary_config.match("BPR*"));
    BOOST_CHECK(summary_config.match("RPR*"));
    for (const auto& node : summary_config) {
        if (node.category() == EclIO::SummaryNode::Category::Region)
            BOOST_CHECK_EQUAL( node.fip_region(), "FIPREG" );
    }

    const auto& fip_regions = summary_config.fip_regions();
    BOOST_CHECK_EQUAL(fip_regions.size(), 1U);

    auto reg_iter = fip_regions.find("FIPREG");
    BOOST_CHECK( reg_iter != fip_regions.end() );


    auto rpr = summary_config.keywords("RP*");
    BOOST_CHECK_EQUAL(rpr.size(), 3U);

    // See comment on the roew() function in Summary.cpp for this uglyness.
    BOOST_CHECK(summary_config.hasKeyword("COPT"));
}

BOOST_AUTO_TEST_CASE(InterReg_Flows) {
    const auto deck_string = std::string { R"(
ROFT
1 2 /
/

ROFTGXYZ
1 2 /
/

RGFT_XYZ
1 2 /
/

RWFR-XYZ
1 3 /
2 3 /
/

RGFR+XYZ
1 3 /
2 3 /
/

RGFTL
2 3 /
/
)" };

    const auto summary_config = createSummary(deck_string);

    BOOST_CHECK_EQUAL(summary_config.size(), 8);
    BOOST_CHECK(summary_config.hasKeyword("ROFT"));
    BOOST_CHECK(summary_config.hasKeyword("ROFTGXYZ"));
    BOOST_CHECK(summary_config.hasKeyword("RGFT_XYZ"));
    BOOST_CHECK(summary_config.hasKeyword("RWFR-XYZ"));
    BOOST_CHECK(summary_config.hasKeyword("RGFR+XYZ"));
    BOOST_CHECK(summary_config.hasKeyword("RGFTL"));

    const auto fip_regions_ireg = summary_config.fip_regions_interreg_flow();
    const auto expect = std::vector<std::string> {
        "FIPNUM", "FIPXYZ",
    };

    BOOST_CHECK_MESSAGE(std::is_permutation(fip_regions_ireg.begin(), fip_regions_ireg.end(),
                                            expect.begin(), expect.end()),
                        "Inter-regional arrays must match expected set");
}

BOOST_AUTO_TEST_CASE( WOPRL ) {
    const std::string input1 = R"(
WOPRL
   'W_1'  2 /
   'xxx'  2 /
/
)";


const std::string input2 = R"(
WOPRL
   'W_1'  2   /
   'W_1'  999 /
/
)";

    ParseContext parseContext;
    // Invalid well
    parseContext.update(ParseContext::SUMMARY_UNKNOWN_WELL, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(createSummary( input1, parseContext ), OpmInputError);

    // Invalid completion
    parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(createSummary( input2, parseContext ), OpmInputError);


    parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::IGNORE);
    parseContext.update(ParseContext::SUMMARY_UNKNOWN_WELL, InputError::IGNORE);
    const auto& summary_config1 = createSummary(input1, parseContext);
    BOOST_CHECK(summary_config1.hasKeyword("WOPRL__2"));
    BOOST_CHECK_EQUAL(summary_config1.size(), 1);

    const auto& summary_config2 = createSummary(input2, parseContext );
    BOOST_CHECK(summary_config2.hasKeyword("WOPRL__2"));
    BOOST_CHECK(!summary_config2.hasKeyword("WOPRL999"));
    const auto& node = summary_config2[0];
    BOOST_CHECK_EQUAL( node.number(), 2 );
    BOOST_CHECK(node.type() == EclIO::SummaryNode::Type::Rate);
}

BOOST_AUTO_TEST_CASE( COPRL ) {
    const std::string input1 = R"(
COPRL
   'W_1'  3 7 2 /
   'xxx'  3 7 2 /
/
)";


const std::string input2 = R"(
COPRL
   'W_1'  3 7 2   /
   'W_1'  2 6 1   /
/
)";

const std::string input3 = R"(
COPRL
   'W_1'  /
/
)";

    ParseContext parseContext;
    // Invalid well
    parseContext.update(ParseContext::SUMMARY_UNKNOWN_WELL, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(createSummary( input1, parseContext ), OpmInputError);

    // Invalid connection
    parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(createSummary( input2, parseContext ), OpmInputError);


    parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::IGNORE);
    parseContext.update(ParseContext::SUMMARY_UNKNOWN_WELL, InputError::IGNORE);
    const auto& summary_config1 = createSummary(input1, parseContext);
    BOOST_CHECK(summary_config1.hasKeyword("COPRL"));
    BOOST_CHECK_EQUAL(summary_config1.size(), 1);


    EclipseGrid grid(10,10,10);
    const auto g1 = grid.getGlobalIndex(1,1,0) + 1;
    const auto g2 = grid.getGlobalIndex(1,1,1) + 1;
    const auto g3 = grid.getGlobalIndex(2,6,1) + 1;

    const auto& summary_config2 = createSummary(input2, parseContext );
    BOOST_CHECK(summary_config2.hasKeyword("COPRL"));
    BOOST_CHECK_EQUAL( summary_config2.size(), 1);
    {
        const auto& node = summary_config2[0];
        BOOST_CHECK_EQUAL( node.number(),  g3);
        BOOST_CHECK(node.type() == EclIO::SummaryNode::Type::Rate);
    }


    const auto& summary_config3 = createSummary(input3, parseContext );
    BOOST_CHECK(summary_config3.hasKeyword("COPRL"));
    BOOST_CHECK_EQUAL( summary_config3.size(), 3);
    {
        const auto& node = summary_config3[0];
        BOOST_CHECK_EQUAL( node.number(),  g1);
    }
    {
        const auto& node = summary_config3[1];
        BOOST_CHECK_EQUAL( node.number(),  g2);
    }
    {
        const auto& node = summary_config3[2];
        BOOST_CHECK_EQUAL( node.number(),  g3);
    }
}




BOOST_AUTO_TEST_CASE( WBP ) {
    const std::string input = R"(
MSUMERR

MSUMLINP

WBP
/

WBP4
/

WBP5
/

WBP9
/
)";

    const auto& summary_config = createSummary(input);

    BOOST_CHECK(summary_config.hasKeyword("WBP"));
    BOOST_CHECK(summary_config.hasKeyword("WBP4"));
    BOOST_CHECK(summary_config.hasKeyword("WBP5"));
    BOOST_CHECK(summary_config.hasKeyword("WBP9"));
}

BOOST_AUTO_TEST_CASE( SUMMARY_INVALID_FIPNUM ) {
    const std::string input = R"(
RPR__ABC
1 2 3 /

RWIP_REG
1 2 3 /
)";

    const std::string input_too_large = R"(
RPR
1 2 3  99 /
)";

    const std::string input_empty= R"(
RPR__REG
15 /
)";
    ParseContext parse_context;
    {
        parse_context.update(ParseContext::SUMMARY_INVALID_FIPNUM, InputError::IGNORE);
        const auto& summary_config = createSummary(input, parse_context);
        BOOST_CHECK(summary_config.hasKeyword("RWIP_REG"));
        BOOST_CHECK(!summary_config.hasKeyword("RPR__ABC"));
    }
    {
        parse_context.update(ParseContext::SUMMARY_INVALID_FIPNUM, InputError::THROW_EXCEPTION);
        BOOST_CHECK_THROW(createSummary(input, parse_context), std::exception);
    }

    {
        parse_context.update(ParseContext::SUMMARY_REGION_TOO_LARGE, InputError::THROW_EXCEPTION);
        BOOST_CHECK_THROW(createSummary(input_too_large, parse_context), std::exception);
    }

    {
        parse_context.update(ParseContext::SUMMARY_REGION_TOO_LARGE, InputError::IGNORE);
        const auto& summary_config = createSummary(input_too_large, parse_context);
        BOOST_CHECK_EQUAL( summary_config.size(), 3);
    }

    {
        parse_context.update(ParseContext::SUMMARY_EMPTY_REGION, InputError::THROW_EXCEPTION);
        BOOST_CHECK_THROW(createSummary(input_empty, parse_context), std::exception);
    }

    {
        parse_context.update(ParseContext::SUMMARY_EMPTY_REGION, InputError::IGNORE);
        const auto& summary_config = createSummary(input_empty, parse_context);
        BOOST_CHECK_EQUAL( summary_config.size(), 1);
    }
}
